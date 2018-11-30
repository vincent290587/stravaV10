/*
 * millis.h
 *
 *  Created on: 5 oct. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_UTILS_MILLIS_H_
#define LIBRARIES_UTILS_MILLIS_H_

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

void millis_init(void);

uint32_t millis(void);

void delay_ms(uint32_t delay_);

void delay_us(uint32_t delay_);

#ifdef	__cplusplus
}
#endif

#endif /* LIBRARIES_UTILS_MILLIS_H_ */
