#ifndef _INCLUDE_ERROR_H_
#define _INCLUDE_ERROR_H_

#include "settings.h"

/* Constants */
#define ERR_MSG_BUFF_SIZE       (128U)

/* Function declarations */
uint8_t check_error(void);
void error_message_send(void);
void flag_error(uint8_t* err_msg, uint16_t msg_len);

#endif /* _INCLUDE_ERROR_H_ */
