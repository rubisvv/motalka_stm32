/*
 * bcd.h
 *
 *  Created on: 17 июн. 2026 г.
 *      Author: rubis
 */

#ifndef INC_BCD_DIHALT_H_
#define INC_BCD_DIHALT_H_

#include "stm32f1xx_hal.h"

typedef struct
{
	uint32_t quot;
	uint8_t rem;
}divmod10_t;

inline static divmod10_t divmodu10(uint32_t n);
char* utoa_fast_div(uint32_t value, char *buffer);
char* utoa_fast_div_len(uint32_t value, char *buffer, uint8_t NumLen);
uint16_t atou_1(const char *buffer);

#endif /* INC_BCD_DIHALT_H_ */
