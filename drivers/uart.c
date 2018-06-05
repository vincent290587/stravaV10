/*
 * serial_handling.c
 *
 *  Created on: 9 août 2017
 *      Author: Vincent
 */

#include "ant.h"
#include "nordic_common.h"
#include "nrf.h"
#include "boards.h"
#include "nrf_libuarte_async.h"

#include "nrf_soc.h"
#include "nrf_delay.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


static uint8_t text[] = "UART example started.\r\n Loopback:\r\n";
static uint8_t text_size = sizeof(text);


void uart_event_handler(nrf_libuarte_async_evt_t * p_evt)
{
	static uint8_t ch = 0;
	ret_code_t ret;
	switch (p_evt->type)
	{
	case NRF_LIBUARTE_ASYNC_EVT_ERROR:
		break;
	case NRF_LIBUARTE_ASYNC_EVT_RX_DATA:
		ch = p_evt->data.rxtx.p_data[0];
		nrf_libuarte_async_rx_free(p_evt->data.rxtx.p_data, p_evt->data.rxtx.length);
		ret = nrf_libuarte_async_tx(&ch, 1);
		APP_ERROR_CHECK(ret);
		break;
	case NRF_LIBUARTE_ASYNC_EVT_TX_DONE:
		break;
	default:
		break;
	}
}


/**
 * @brief Function for initializing the UART.
 */
 void uart_init(void)
{
	 nrf_libuarte_async_config_t nrf_libuarte_async_config = {
			 .tx_pin     = TX_PIN_NUMBER,
			 .rx_pin     = RX_PIN_NUMBER,
			 .baudrate   = NRF_UARTE_BAUDRATE_115200,
			 .parity     = NRF_UARTE_PARITY_EXCLUDED,
			 .hwfc       = NRF_UARTE_HWFC_DISABLED,
			 .timeout_us = 100,
	 };

	 ret_code_t err_code = nrf_libuarte_async_init(&nrf_libuarte_async_config, uart_event_handler);

	 APP_ERROR_CHECK(err_code);

	 nrf_libuarte_async_enable(1);

	 err_code = nrf_libuarte_async_tx(text, text_size);
	 APP_ERROR_CHECK(err_code);
}

/**
 *
 */
 void uart_tx(void)
 {

	 ret_code_t err_code = nrf_libuarte_async_tx(text, text_size);
	 APP_ERROR_CHECK(err_code);
 }
