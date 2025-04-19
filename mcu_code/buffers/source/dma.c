#include "dma.h"

/* Global variables */
g_dmaCTRL dma_packet;

void DMA_init(void)
{
   /* Enable DMA through HAL */
   _enable_IRQ();
   dmaEnable();
}


/*
 * NOTE - According to the TRM, frame count and element counts are 13-bits wide.
 *        Structuring the transfer maximizing both allows for 26 bits = 64MB with
 *        8-bit elements, and 512MB with 64-bit elements.
 *
 *        For reference, 13 bits can represent values up to 8K - 1 (8191)
 *
 * Returns 1 if the transfer is successful, 0 otherwise.
 */
uint8_t DMA_schedule_transfer(const uint8_t* source_addr, uint8_t* dest_addr, uint32_t num_bytes)
{
   uint8_t dma_tf_rtn = 0U;
   uint32_t curr_bytes = 0U;
   uint32_t curr_transfer = 0U;
   uint32_t num_transfers = 0U;
   uint32_t sadd = 0U;
   uint32_t dadd = 0U;
   dmaChannel_t chan = DMA_CH0;
   dmaChannel_t next_chan = DMA_CH0;

   /*
    * For simplicity, currently only supports transfers of
    * N frames of 1 8-bit element. This limits us to a maximum of
    * (8K - 1) * 32 bytes through chaining requests.
    *
    * TODO: More sophisticated system
    */
   if(num_bytes <= DMA_MAX_TOTAL_SIZE)
   {
      /*
       * We need to chain requests when num_bytes >= 8K. To do so, we create
       * successive DMA requests, where the final request sets the CHCTRL field
       * to '0' to end the chain.
       */
      num_transfers = (num_bytes / DMA_MAX_TRANSFER_SIZE) + 1U;
      for(curr_transfer = 0U; curr_transfer < num_transfers; curr_transfer++)
      {
         /* Address offset calculation for source and destination */
         sadd = (uint32_t)source_addr + (curr_transfer * DMA_MAX_TRANSFER_SIZE);
         dadd = (uint32_t)dest_addr + (curr_transfer * DMA_MAX_TRANSFER_SIZE);

         /* Channel calculation for chaining */
         chan = DMA_CH0 + curr_transfer;

         /* Transfer size + next channel (for chaining) */
         if(num_bytes > DMA_MAX_TRANSFER_SIZE)
         {
            curr_bytes = DMA_MAX_TRANSFER_SIZE;
            next_chan = chan + 2U;
         }
         else
         {
            curr_bytes = num_bytes;
            next_chan = 0U;
         }

         /* Fill DMA packet */
         dma_packet.SADD = sadd;
         dma_packet.DADD = dadd;
         dma_packet.CHCTRL = next_chan;
         dma_packet.FRCNT = curr_bytes;
         dma_packet.ELCNT = 1U;
         dma_packet.ELDOFFSET = 0U;
         dma_packet.ELSOFFSET = 0U;
         dma_packet.FRDOFFSET = 0U;
         dma_packet.FRSOFFSET = 0U;
         dma_packet.PORTASGN = PORTA_READ_PORTA_WRITE;
         dma_packet.RDSIZE = ACCESS_8_BIT;
         dma_packet.WRSIZE = ACCESS_8_BIT;
         dma_packet.TTYPE = BLOCK_TRANSFER;
         dma_packet.ADDMODERD = ADDR_INC1;
         dma_packet.ADDMODEWR = ADDR_INC1;
         dma_packet.AUTOINIT = AUTOINIT_OFF;

         dmaSetCtrlPacket(chan, dma_packet);
         dmaSetPriority(chan, HIGHPRIORITY);

         /* Decrement */
         num_bytes -= curr_bytes;
      }

      /* Enable all channels */
      for(curr_transfer = 0U; curr_transfer < num_transfers; curr_transfer++)
      {
         dmaSetChEnable(DMA_CH0 + curr_transfer, DMA_SW);
      }

      dma_tf_rtn = 1U;
   }

   return dma_tf_rtn;
}


uint8_t DMA_check_busy(void)
{
   return (uint8_t)dmaIsBusy();
}


#ifdef TEST_DMA_MODULE

#include "timer.h"
#include "serial.h"

#define DMA_TEST_SIZE           (131072U)

#define v1                      10U
#define v2                      v1, v1
#define v4                      v2, v2
#define v8                      v4, v4
#define v16                     v8, v8
#define v32                     v16, v16
#define v64                     v32, v32
#define v128                    v64, v64
#define v256                    v128, v128
#define v512                    v256, v256
#define v1024                   v512, v512
#define v2048                   v1024, v1024
#define v4096                   v2048, v2048
#define v8192                   v4096, v4096
#define v16384                  v8192, v8192
#define v32768                  v16384, v16384
#define v65536                  v32768, v32768
#define v131072                 v65536, v65536

const uint8_t TEST_WEIGHTS_LARGE[DMA_TEST_SIZE] = {v131072};
uint8_t test_buff[DMA_TEST_SIZE] = {0U};

uint8_t m1[DMA_TEST_SIZE];
uint8_t d[DMA_TEST_SIZE];

/*
 * Naive matrix multiplication:
 * m1 - M n N matrix
 * m2 - N x P matrix
 * m3 - M x P matrix
 */
void naive_MatMul(const uint8_t* m1, const uint8_t* m2, uint8_t* dest, uint16_t M, uint16_t N, uint16_t P)
{
   int i, j, k;
   for(i = 0; i < M; i++)
   {
       for(j = 0; j < P; j++)
       {
           for(k = 0; k < N; k++)
           {
               dest[i*P + j] += m1[i*N + k] * m2[k*P +j];
           }
       }
   }
}


/*
 * Debugging / test function for DMA subsystem, including timing analysis.
 * Implemented for debug mode, e.g., no printouts
 */
void DMA_testing(void)
{
   /* Local variables */
   uint8_t dummy_var = 0U;
   uint64_t ustart = 0U;
   uint64_t uend = 0U;
   uint64_t udiff = 0U;

   uint64_t uend2 = 0U;
   uint64_t udiff2 = 0U;

   uint8_t i;
   uint32_t mem_sizes[8U] = {1024U, 2048U, 4096U, 8192U, 16384U, 32768U, 65536U, 131072U};

   uint16_t M[8U] = {32U, 45U, 64U, 90U, 128U, 181U, 256U, 362U};
   uint16_t N[8U] = {32U, 45U, 64U, 90U, 128U, 181U, 256U, 362U};

   uint8_t msg_buffer[128U] = {0U};
   uint16_t msg_len = 0U;

   for(i = 0U; i < 8U; i++)
   {
       /*
        * DMA weight loading
        */
       ustart = micros();
       DMA_schedule_transfer(TEST_WEIGHTS_LARGE, test_buff, mem_sizes[i]);
       uend2 = micros();
       while(DMA_check_busy())
       {
          dummy_var &= dummy_var;
       }
       uend = micros();

       /* udiff - Timing for DMA transfer (CPU-free) */
       /* udiff2 - Timing for initiating DMA request (CPU-bound) */
       udiff = uend - uend2;
       udiff2 = uend2 - ustart;

       msg_len = (uint16_t)sprintf(msg_buffer, "DMA Request time (%u bytes): %llu us\r\n", mem_sizes[i], udiff2);
       SCI_TX_packet(msg_buffer, msg_len);

       msg_len = (uint16_t)sprintf(msg_buffer, "DMA Transfer time (%u bytes): %llu us\r\n", mem_sizes[i], udiff);
       SCI_TX_packet(msg_buffer, msg_len);

       /*
        * Direct transfer using memcpy() for comparison
        */
       ustart = micros();
       memcpy(test_buff, TEST_WEIGHTS_LARGE, mem_sizes[i]);
       uend = micros();
       udiff = uend - ustart;

       msg_len = (uint16_t)sprintf(msg_buffer, "Memcpy time (%u bytes): %llu us\r\n", mem_sizes[i], udiff);
       SCI_TX_packet(msg_buffer, msg_len);

       /*
        * (Naive) matrix multiplication to compare latency
        */
       ustart = micros();
       naive_MatMul(m1, m1, d, M[i], N[i], M[i]);
       uend = micros();
       udiff = uend - ustart;

       msg_len = (uint16_t)sprintf(msg_buffer, "MatMul time (%u bytes): %llu us\r\n", mem_sizes[i], udiff);
       SCI_TX_packet(msg_buffer, msg_len);
   }

   dummy_var = 0U;
}
#endif /* TEST_DMA_MODULE */
