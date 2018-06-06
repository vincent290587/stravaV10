/*
 * serial_handling.c
 *
 *  Created on: 9 août 2017
 *      Author: Vincent
 */

#include "ant.h"
#include "nordic_common.h"
#include "nrf.h"
#include "uart.h"
#include "boards.h"
#include "segger_wrapper.h"
#include "nrf_soc.h"
#include "GPSMGMT.h"
#include "nrf_delay.h"
#include "ring_buffer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"



/*******************************************************************************
 * Variables
 ******************************************************************************/
#define UART0_RB_SIZE         256
RING_BUFFER_DEF(uart0_rb1, UART0_RB_SIZE);

volatile uint32_t m_last_rx; /* Index of the memory to save new arrived data. */



void uart_event_handler(nrf_libuarte_async_evt_t * p_evt)
{
    W_SYSVIEW_RecordEnterISR();

	static uint8_t ch = 0;
	ret_code_t ret;
	switch (p_evt->type)
	{
	case NRF_LIBUARTE_ASYNC_EVT_ERROR:
		break;
	case NRF_LIBUARTE_ASYNC_EVT_RX_DATA:
	{
		ch = p_evt->data.rxtx.p_data[0];
		nrf_libuarte_async_rx_free(p_evt->data.rxtx.p_data, p_evt->data.rxtx.length);
		ret = nrf_libuarte_async_tx(&ch, 1);
		APP_ERROR_CHECK(ret);

		if (RING_BUFF_IS_NOT_FULL(uart0_rb1)) {
			RING_BUFFER_ADD(uart0_rb1, ch);
		} else {
			NRF_LOG_ERROR("Ring buffer full");
		}

	}
		break;
	case NRF_LIBUARTE_ASYNC_EVT_TX_DONE:
		break;
	default:
		break;
	}

    W_SYSVIEW_RecordExitISR();
}


/**
 * @brief Function for initializing the UART.
 */
 void uart_init(nrf_uarte_baudrate_t baud)
{
	 nrf_libuarte_async_config_t nrf_libuarte_async_config = {
			 .tx_pin     = TX_PIN_NUMBER,
			 .rx_pin     = RX_PIN_NUMBER,
			 .baudrate   = baud,
			 .parity     = NRF_UARTE_PARITY_EXCLUDED,
			 .hwfc       = NRF_UARTE_HWFC_DISABLED,
			 .timeout_us = 100,
	 };

	 ret_code_t err_code = nrf_libuarte_async_init(&nrf_libuarte_async_config, uart_event_handler);

	 APP_ERROR_CHECK(err_code);

	 nrf_libuarte_async_enable(1);

}

 /**
  *
  */
 void uart_uninit(void) {

	 // TODO
 }

/**
 *
 */
 void uart_send(uint8_t * p_data, size_t length)
 {
	 ret_code_t err_code = nrf_libuarte_async_tx(p_data, length);
	 APP_ERROR_CHECK(err_code);
 }

/**
 *
 */
 void uart_tasks(void) {

	 /* If ring buffer is not empty, parse data. */
	 while (RING_BUFF_IS_NOT_EMPTY(uart0_rb1))
	 {
		 gps_encode_char(RING_BUFF_GET_ELEM(uart0_rb1));

		 RING_BUFFER_POP(uart0_rb1);
	 }

 }
