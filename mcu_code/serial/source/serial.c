#include "serial.h"

/* Global input buffer */
uint8_t RX_buffer[RX_BUFFER_SIZE] = {0U};
uint16_t RX_buffer_idx = 0U;
uint16_t RX_expected_bytes = 0U;
packet_state_t rx_state;

/* Local function declarations */
uint16_t buffer_data(uint8_t byte);


/* Transmission function */
void SCI_TX_packet(uint8_t* data_packet, uint16_t data_len)
{
   sciSend(SCI_1, data_len, data_packet);
}


/*
 * RX function in polling mode. Receives and buffers a single byte.
 *
 * If a complete packet has been formed, sets the "payload_len" input
 * pointer to the packet length and returns a pointer to the data buffer.
 *
 * If a complete packet has not been formed, returns a NULL pointer and
 * does not set the "payload_len" input pointer.
 */
uint8_t* SCI_RX_poll(uint16_t* payload_len)
{
   uint8_t* data_ready = NULL;
   uint16_t data_len = 0U;
   uint8_t rx_byte;

   /* Check if byte is available */
   if(sciIsRxReady(SCI_1))
   {
      /* Read in byte */
      rx_byte = (uint8_t)sciReceiveByte(SCI_1);

      /* Move into buffer and form packet */
      data_len = buffer_data(rx_byte);
      if(data_len > 0U)
      {
         data_ready = RX_buffer;
         *payload_len = data_len;
      }
   }

   return data_ready;
}


/*
 * Buffers a received byte and performs checks for proper packet
 * formation.
 *
 * Returns the length of prompt received once the payload is complete, and 0 otherwise.
 */
uint16_t buffer_data(uint8_t byte)
{
   uint16_t len_rtn = 0U;

   switch(rx_state)
   {
      case STATE_SEQ_BYTE1:
         RX_buffer_idx = 0U;
         if(byte == RX_SEQ_START_BYTE1)
         {
            rx_state = STATE_SEQ_BYTE2;
         }

         break;
      case STATE_SEQ_BYTE2:
         if(byte == RX_SEQ_START_BYTE2)
         {
            rx_state = STATE_SEQ_BYTE3;
         }
         else
         {
            rx_state = STATE_SEQ_BYTE1;
         }

         break;
      case STATE_SEQ_BYTE3:
         if(byte == RX_SEQ_START_BYTE3)
         {
            rx_state = STATE_SEQ_BYTE4;
         }
         else
         {
            rx_state = STATE_SEQ_BYTE1;
         }

         break;
      case STATE_SEQ_BYTE4:
         if(byte == RX_SEQ_START_BYTE4)
         {
            rx_state = STATE_GET_LENGTH;
         }
         else
         {
            rx_state = STATE_SEQ_BYTE1;
         }

         break;
      case STATE_GET_LENGTH:
         /* Length byte refers to number of tokens */
         if(byte > MAX_PROMPT_LEN)
         {
            rx_state = STATE_SEQ_BYTE1;
         }
         else
         {
            rx_state = STATE_GET_PAYLOAD;

            /* Tokens transmitted as [word embedding, position embedding] */
            RX_expected_bytes = (byte * EMB_TOK_SIZE * 2);
         }

         break;
      case STATE_GET_PAYLOAD:
         RX_buffer[RX_buffer_idx] = byte;
         RX_buffer_idx++;

         /* Check for end of payload */
         if(RX_buffer_idx == RX_expected_bytes)
         {
            rx_state = STATE_SEQ_BYTE1;
            len_rtn = RX_buffer_idx;
         }

         break;
      default:
         rx_state = STATE_SEQ_BYTE1;
         break;
   }

   return len_rtn;
}


/* Initialization function */
void SCI_init(void)
{
   /* HAL Initialization */
    sciInit();

    RX_buffer_idx = 0U;
    rx_state = STATE_SEQ_BYTE1;

    /* Enable interrupts */
    //sciEnableNotification(SCI_1, SCI_RX_INT);

    /* Set mode to receive a single byte */
    //sciReceive(SCI_1, 1U, &RX_buffer[RX_buffer_idx]);
}


#if 0
/*
 * Checks if data is available (e.g., a prompt has arrived). For interrupt mode.
 * Returns a pointer to the data buffer if available, NULL pointer if not.
 */
uint8_t* SCI_check_rx(void)
{
   uint8_t* data_ready = NULL;

   /* If data is ready, reset flags and buffer index for future packets */
   if(payload_complete)
   {
      data_ready = RX_buffer;
      payload_complete = 0U;
      RX_buffer_idx = 0U;
   }

   return data_ready;
}


/*
 * Implementation of sciNotification() function
 */
void sciNotification(sciBASE_t* sci, uint32_t flags)
{

}
#endif
