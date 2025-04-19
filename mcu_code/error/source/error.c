#include <string.h>

#include "settings.h"
#include "error.h"
#include "serial.h"

/* Constants */
uint8_t err_flag = 0U;
uint8_t err_msg_buff[ERR_MSG_BUFF_SIZE] = {0U};
uint16_t err_msg_len = 0U;


/* Returns the status of error flag */
uint8_t check_error(void)
{
   return err_flag;
}


/* Transmits stored error message(s) */
void error_message_send(void)
{
   if(err_msg_len > 0U)
   {
      SCI_TX_packet(err_msg_buff, err_msg_len);
   }
}


/* Sets the error flag and appends to the error message buffer */
void flag_error(uint8_t* err_msg, uint16_t msg_len)
{
   err_flag = 1U;

   /* Append only partial message if it would cause an overflow */
   if(err_msg_len + msg_len >= ERR_MSG_BUFF_SIZE)
   {
      /* -1U to account for EOS char (0x00) */
      msg_len = ERR_MSG_BUFF_SIZE - err_msg_len - 1U;
   }

   memcpy(&err_msg_buff[err_msg_len], err_msg, msg_len);
   err_msg_len += msg_len;
}
