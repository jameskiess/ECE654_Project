#include "model.h"

#include "weights.h"
#include "dma.h"
#include "timer.h"

#include "settings.h"

#include "HL_sys_mpu.h"

/*
 * Define model structure:
 *  - Layer number
 *  - Layer size (total # params, including bias)
 *  - Matrix dimensions (M, N)
 *  - Bias length
 *  - Bias offset
 *
 * Note that weights are stored in contiguous memory, e.g. a 1D array.
 * Thus, we need to track the dimensions of the matrix as well as an
 * offset for the bias
 */

/* Initialize model by loading first layer */
void model_init(void)
{
   /* Load first layer (blocking, non-DMA) */
}


/*
 * Initiates a DMA transfer to load the requested layer weights from Flash.
 *
 * Returns 1 if DMA request is successful, and 0 otherwise (e.g., DMA is busy)
 */
uint8_t model_request_layer(uint8_t layer_num)
{
   uint8_t req_rtn = 0U;

   if(!DMA_check_busy())
   {
      //DMA_schedule_transfer(src, dst, 4U);

      req_rtn = 1U;
   }

   return req_rtn;
}


/*
 * Performs inference on the given token sequence, and outputs the final hidden states.
 * Returns the length of the final hidden sequence.
 */
uint16_t model_inference(uint8_t* tok_in, uint16_t tok_in_len, uint8_t* hidden_out)
{
   /* Local buffers for activations */
   const size_t ACT_BUFF_SIZE = 8192U; // Placeholder
   uint8_t* act_buffer = (uint8_t*)malloc(ACT_BUFF_SIZE);

   /* Execute computational graph */
}


#ifdef TEST_MODEL_MODULE

/*
 * Debugging / test function for model computational graph and weight loading.
 * Implemented for debug mode, e.g., no printouts
 */
void model_testing(void)
{
   /* Local variables */
   uint8_t dummy_var = 0U;

   /* Test 1 - XXX */

   dummy_var = 0U;
}
#endif /* TEST_MODEL_MODULE */
