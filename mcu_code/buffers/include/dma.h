#ifndef _INCLUDE_DMA_H_
#define _INCLUDE_DMA_H_

#include "HL_reg_dma.h"
#include "HL_sys_dma.h"

#include "settings.h"

/* Constants */
#define DMA_MAX_TRANSFER_SIZE       (8191U)
#define DMA_MAX_TOTAL_SIZE          (8191U * 32U)

void DMA_init(void);
uint8_t DMA_schedule_transfer(const uint8_t* source_addr, uint8_t* dest_addr, uint32_t num_bytes);
uint8_t DMA_check_busy(void);

#ifdef TEST_DMA_MODULE
void DMA_testing(void);
#endif /* TEST_DMA_MODULE */

#endif /* _INCLUDE_DMA_H_ */
