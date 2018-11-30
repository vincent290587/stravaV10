/*
 * serial.h
 *
 *  Created on: 7 déc. 2017
 *      Author: Vincent
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#ifndef TDD

#include "nrfx_uarte.h"

#ifdef __cplusplus
extern "C" {
#endif

void uart_timer_init(void);

void uart_init(nrf_uarte_baudrate_t baud);

void uart_init_tx_only(nrf_uarte_baudrate_t baud);

void uart_uninit(void);

void uart_tasks(void);

void uart_send(uint8_t * p_data, size_t length);

#ifdef __cplusplus
}
#endif

#else /* TDD */
#include "uart_tdd.h"
#endif

#endif /* SERIAL_H_ */
