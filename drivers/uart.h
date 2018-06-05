/*
 * serial.h
 *
 *  Created on: 7 déc. 2017
 *      Author: Vincent
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include "nrf_libuarte_async.h"

#ifdef __cplusplus
extern "C" {
#endif

void uart_init(nrf_uarte_baudrate_t baud);

void uart_tasks(void);

void uart_send(uint8_t * p_data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* SERIAL_H_ */
