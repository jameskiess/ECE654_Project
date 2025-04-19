#ifndef _INCLUDE_SETTINGS_H_
#define _INCLUDE_SETTINGS_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef NULL
#define NULL        ((void*)0)
#endif

/* Global settings / flags */
#define EMB_TOK_BYTES           (1U)
#define EMB_TOK_LEN             (128U)
#define MAX_PROMPT_LEN          (16)

//#define ENABLE_CMSIS_NN

/* Test flags */
//#define TEST_MODEL_MODULE
#define TEST_DMA_MODULE

#endif /* _INCLUDE_SETTINGS_H_ */
