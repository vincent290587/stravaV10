/*
 * serial_handling.c
 *
 *  Created on: 9 août 2017
 *      Author: Vincent
 */


#include "nordic_common.h"
#include "uart_tdd.h"
#include "Locator.h"
#include "segger_wrapper.h"
#include "ring_buffer.h"


/*******************************************************************************
 * Variables
 ******************************************************************************/
#define UART0_RB_SIZE         256
RING_BUFFER_DEF(uart0_rb1, UART0_RB_SIZE);

volatile uint32_t m_last_rx; /* Index of the memory to save new arrived data. */

static volatile uint32_t m_error_mask;

static uint8_t m_uart_rx_buffer[4];


/**
 * @brief Handler for timer events.
 */
void timer_event_handler(void* p_context)
{
}

/**
 *
 * @param p_event
 * @param p_context
 */
void uart_rx_handler(char c)
{
	W_SYSVIEW_RecordEnterISR();

	if (RING_BUFF_IS_NOT_FULL(uart0_rb1)) {
		RING_BUFFER_ADD_ATOMIC(uart0_rb1, c);
	} else {
		NRF_LOG_ERROR("Ring buffer full");

		// empty ring buffer
		RING_BUFF_EMPTY(uart0_rb1);
	}

	W_SYSVIEW_RecordExitISR();
}

/**
 *
 */
void uart_timer_init(void) {

}

/**
 * @brief Function for initializing the UART.
 */
void uart_init_tx_only(nrf_uarte_baudrate_t baud)
{
	//	 ret_code_t err_code;
	//
	//	 m_baud = baud;
	//
	//	 m_error_mask = 0;
	//
	//	 nrfx_uarte_config_t nrf_uarte_config = NRFX_UARTE_DEFAULT_CONFIG;
	//
	//	 nrf_uarte_config.pseltxd    = TX_PIN_NUMBER;
	//	 nrf_uarte_config.pselrxd    = RX_PIN_NUMBER;
	//	 nrf_uarte_config.baudrate   = baud;
	//	 nrf_uarte_config.parity     = NRF_UARTE_PARITY_EXCLUDED;
	//	 nrf_uarte_config.hwfc       = NRF_UARTE_HWFC_DISABLED;
	//
	//	 err_code = nrfx_uarte_init(&uart, &nrf_uarte_config, uart_event_handler);
	//	 APP_ERROR_CHECK(err_code);

	LOG_DEBUG("UART configured TX only baud=%u", (uint32_t)baud);

	//uart_xfer_done = true;

}

/**
 * @brief Function for initializing the UART.
 */
void uart_init(nrf_uarte_baudrate_t baud)
{
	//	 uart_init_tx_only(baud);
	//
	//	 nrfx_uarte_rx(&uart, m_uart_rx_buffer, sizeof(m_uart_rx_buffer));

}

/**
 *
 */
void uart_uninit(void) {

	//	 nrfx_uarte_uninit(&uart);
	//
	//	 nrf_delay_us(500);
}

/**
 *
 */
void uart_send(uint8_t * p_data, size_t length) {

	LOG_DEBUG("UART TX of %u bytes", length);

	//	 ret_code_t err_code = nrfx_uarte_tx(&uart, p_data, length);
	//	 APP_ERROR_CHECK(err_code);
	//
	//	 if (!err_code) uart_xfer_done = false;
	//
	//	 while (!uart_xfer_done) {
	//		 nrf_delay_ms(1);
	//	 }
}

/**
 *
 */
void uart_tasks(void) {

	/* If ring buffer is not empty, parse data. */
	while (RING_BUFF_IS_NOT_EMPTY(uart0_rb1))
	{
		char c = RING_BUFF_GET_ELEM(uart0_rb1);

		locator_encode_char(c);

		//LOG_RAW_INFO(c);

		RING_BUFFER_POP(uart0_rb1);
	}

}
