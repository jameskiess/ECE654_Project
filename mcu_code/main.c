#include "settings.h"

#include "serial.h"
#include "timer.h"
#include "error.h"
#include "dma.h"

/* Some constants (e.g., return messages) */
const uint8_t* INIT_MSG = (uint8_t*)"Initialization complete. Ready to receive prompt.\n";
const uint16_t INIT_MSG_LEN = 50U;

const uint8_t* PROMPT_RX_MSG = (uint8_t*)"Prompt received. Initiating computational graph.\n";
const uint16_t PROMPT_RX_MSG_LEN = 49U;

/*
 * Allocated buffer size equals static message character length + 9 + EOS
 * 9 = maximum expected microsecond value (999 seconds = 999,000,000us)
 */
const char* TIMING_MSG = "Token inference latency: %llu us.\n";
const uint16_t TIMING_MSG_BUFF_SIZE = 40U;

int inference_main(void)
{
   /* Function variables */
   uint8_t* prompt_ptr = NULL;
   uint16_t prompt_len = 0U;

   uint64_t start_us = 0U;
   uint64_t end_us = 0U;
   uint64_t process_time_us = 0U;

   uint8_t prompt_time_report_buffer[TIMING_MSG_BUFF_SIZE] = {0U};
   uint16_t prompt_time_report_msg_len = 0U;

   /* Initialize SCI */
   SCI_init();

   /* Initialize RTI (for microsecond timing) */
   RTI_init();

   /* Initialize DMA subsystem */
   DMA_init();

   /* Initialize and prime model by loading first layer (blocking) */
   model_init();

   /* Send a welcome message */
   SCI_TX_packet(INIT_MSG, INIT_MSG_LEN);

   /*
    * ======== TESTING =======
    */

#ifdef TEST_MODEL_MODULE
   /* ----- TESTING ----- */
   model_testing();
#endif /* TEST_MODEL_MODULE */

#ifdef TEST_DMA_MODULE
   DMA_testing();
#endif /* TEST_DMA_MODULE */

   /*
    * ======= END TESTING =====
    */

   /* Processing loop once data is available */
   while(!check_error())
   {
      prompt_ptr = SCI_RX_poll(&prompt_len);
      if(prompt_ptr != NULL)
      {
         /* Prompt received - initiate processing */
         SCI_TX_packet(PROMPT_RX_MSG, PROMPT_RX_MSG_LEN);

         start_us = micros();

         /* Process prompt */
         /* ... */

         end_us = micros();
         process_time_us = end_us - start_us;

         prompt_time_report_msg_len = (uint16_t)sprintf(prompt_time_report_buffer, TIMING_MSG, process_time_us);
         SCI_TX_packet(prompt_time_report_buffer, prompt_time_report_msg_len);
      }
   }

   /* Never reached (except for error) */
   error_message_send();
   return 0;
}
