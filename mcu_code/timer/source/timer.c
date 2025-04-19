#include "timer.h"


uint64_t micros(void)
{
   /* RTIUC1 is driven by RTICLK (75MHz) */
   /* RTIFRC1 is incremented when RTIUC1 reaches 0xFFFF FFFFU */
   uint64_t frc = (uint64_t)RTI_PORT->CNT[RTI_1].FRCx;
   uint64_t uc = (uint64_t)RTI_PORT->CNT[RTI_1].UCx;
   uint64_t micros = ((frc << 32U) | uc) / RTI_CLK_FREQ_MHZ;

   return micros;
}


uint64_t millis(void)
{
   return micros() / 1000U;
}


/* Simple / naive delay - hold CPU until specified microseconds have passed */
void delay_us(uint64_t delay_period_us)
{
   uint64_t start_us = micros();
   uint64_t current_us = start_us;

   while((current_us - start_us) < delay_period_us)
   {
      current_us = micros();
   }
}


/* Simple / naive delay - hold CPU until specified milliseconds have passed */
void delay_ms(uint64_t delay_period_ms)
{
   uint64_t start_ms = millis();
   uint64_t current_ms = start_ms;

   while((current_ms - start_ms) < delay_period_ms)
   {
      current_ms = millis();
   }
}


void RTI_init(void)
{
   /* HAL initialization */
   rtiInit();

   /* Start RTI counter 1 */
   rtiStartCounter(RTI_PORT, RTI_1);
}
