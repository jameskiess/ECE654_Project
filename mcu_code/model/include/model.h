#ifndef _INCLUDE_MODEL_H_
#define _INCLUDE_MODEL_H_

#include "settings.h"

//#include "arm_nnfunctions.h"
//#include "arm_nnsupportfunctions.h"

/* Constants */

/* Function declarations */
void model_init(void);

//uint8_t model_request_layer(uint8_t layer_num);
uint8_t model_request_layer(uint8_t layer_num);

#ifdef TEST_MODEL_MODULE
/* Test / Debug */
void model_testing(void);
#endif /* TEST_MODEL_MODULE */

#endif /* _INCLUDE_MODEL_H_ */
