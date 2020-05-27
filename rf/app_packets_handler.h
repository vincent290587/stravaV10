/*
 * app_packets_handler.h
 *
 *  Created on: 27 mai 2020
 *      Author: vgol
 */

#ifndef APP_PACKETS_HANDLER_H_
#define APP_PACKETS_HANDLER_H_

#include <stdint.h>

void nrf_qwr_error_reset(void);
uint32_t nrf_qwr_error_get(void);
uint32_t nus_data_send(uint8_t *p_data, uint16_t length);


void app_handler__nus_data_handler(uint8_t const *p_data, uint16_t length);

void app_handler__signal(void);

void app_handler__on_connected(void);

void app_handler__task(void * p_context);


#endif /* APP_PACKETS_HANDLER_H_ */
