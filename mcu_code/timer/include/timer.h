#ifndef _INCLUDE_TIMER_H_
#define _INCLUDE_TIMER_H_

#include "HL_rti.h"
#include "HL_reg_rti.h"

#include "settings.h"

/* Constants */
#define RTI_CLK_FREQ        (75000000U)
#define RTI_CLK_FREQ_MHZ    (75U)

/* Useful alias */
#define RTI_PORT            (rtiREG1)
#define RTI_0               (rtiCOUNTER_BLOCK0)
#define RTI_1               (rtiCOUNTER_BLOCK1)

/* Function declarations */
void RTI_init(void);

uint64_t micros(void);
uint64_t millis(void);
void delay_us(uint64_t delay_period_us);
void delay_ms(uint64_t delay_period_ms);

#endif /* _INCLUDE_TIMER_H_ */
