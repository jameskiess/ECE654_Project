#ifndef _INCLUDE_SERIAL_H_
#define _INCLUDE_SERIAL_H_

#include "HL_sci.h"
#include "HL_reg_sci.h"

#include "settings.h"

/* Constants */
#define RX_SEQ_START_BYTE1      (0xAEU)
#define RX_SEQ_START_BYTE2      (0x7DU)
#define RX_SEQ_START_BYTE3      (0x49U)
#define RX_SEQ_START_BYTE4      (0xBCU)

#define EMB_TOK_SIZE            (EMB_TOK_BYTES * EMB_TOK_LEN)
#define RX_BUFFER_SIZE          (EMB_TOK_SIZE * MAX_PROMPT_LEN * 2)

/* Packet state machine */
typedef enum
{
   STATE_SEQ_BYTE1,
   STATE_SEQ_BYTE2,
   STATE_SEQ_BYTE3,
   STATE_SEQ_BYTE4,
   STATE_GET_LENGTH,
   STATE_GET_PAYLOAD,
} packet_state_t;

/* Useful alias */
#define SCI_1                   (sciREG1)

/* Function declarations */
void SCI_TX_packet(uint8_t* data_packet, uint16_t data_len);

/* RX functions (polling) */
uint8_t* SCI_RX_poll(uint16_t* payload_len);

void SCI_init(void);

#if 0
/* RX functions (interrupt) */
uint8_t* SCI_check_rx(void);
#endif

#endif /* _INCLUDE_SERIAL_H_ */
