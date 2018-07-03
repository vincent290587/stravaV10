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
#include "app_timer.h"
#include "ring_buffer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define NRFX_UARTE_INDEX     0

#define UART_RELOAD_DELAY           APP_TIMER_TICKS(2000)

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define UART0_RB_SIZE         256
RING_BUFFER_DEF(uart0_rb1, UART0_RB_SIZE);

APP_TIMER_DEF(m_uarte_timer);

volatile uint32_t m_last_rx; /* Index of the memory to save new arrived data. */

static const nrfx_uarte_t uart = NRFX_UARTE_INSTANCE(NRFX_UARTE_INDEX);

static volatile bool uart_xfer_done = true;  /**< Flag used to indicate that SPI instance completed the transfer. */

static uint8_t ch = 0;


/**
 * @brief Handler for timer events.
 */
void timer_event_handler(void* p_context)
{
	nrfx_uarte_rx_abort(&uart);

	APP_ERROR_CHECK(nrfx_uarte_rx(&uart, &ch, 1));
}

/**
 *
 * @param p_event
 * @param p_context
 */
void uart_event_handler(nrfx_uarte_event_t const * p_event,
        				void * p_context)
{
    W_SYSVIEW_RecordEnterISR();

    switch (p_event->type) {
    case NRFX_UARTE_EVT_ERROR:
    {
    	APP_ERROR_CHECK(p_event->data.error.error_mask);
    }
    break;
    case NRFX_UARTE_EVT_RX_DONE:
    {
    	if (p_event->data.rxtx.bytes) {

    		ch = p_event->data.rxtx.p_data[0];

//    		NRF_LOG_RAW_INFO("%c", ch);

    		if (RING_BUFF_IS_NOT_FULL(uart0_rb1)) {
    			RING_BUFFER_ADD(uart0_rb1, ch);
    		} else {
    			NRF_LOG_ERROR("Ring buffer full");
    		}

    	}

    	APP_ERROR_CHECK(nrfx_uarte_rx(&uart, &ch, 1));
    }
    break;
    case NRFX_UARTE_EVT_TX_DONE:
    {
    	NRF_LOG_INFO("UART TX done");
    	uart_xfer_done = true;
    }
    break;
    }

    W_SYSVIEW_RecordExitISR();
}

/**
 *
 */
void uart_timer_init(void) {

	ret_code_t err_code = app_timer_create(&m_uarte_timer, APP_TIMER_MODE_REPEATED, timer_event_handler);
	APP_ERROR_CHECK(err_code);

}

/**
 * @brief Function for initializing the UART.
 */
 void uart_init(nrf_uarte_baudrate_t baud)
{
	 nrfx_uarte_config_t nrf_uarte_config = NRFX_UARTE_DEFAULT_CONFIG;

	 nrf_uarte_config.pseltxd    = TX_PIN_NUMBER;
	 nrf_uarte_config.pselrxd    = RX_PIN_NUMBER;
	 nrf_uarte_config.baudrate   = baud;
	 nrf_uarte_config.parity     = NRF_UARTE_PARITY_EXCLUDED;
	 nrf_uarte_config.hwfc       = NRF_UARTE_HWFC_DISABLED;

	 ret_code_t err_code = nrfx_uarte_init(&uart, &nrf_uarte_config, uart_event_handler);

	 NRF_LOG_INFO("UART configured baud=%u", (uint32_t)baud);

	 nrfx_uarte_rx(&uart, &ch, 1);

	 err_code = app_timer_start(m_uarte_timer, UART_RELOAD_DELAY, NULL);
	 APP_ERROR_CHECK(err_code);

	 uart_xfer_done = true;

}

 /**
  *
  */
 void uart_uninit(void) {

	 nrfx_uarte_uninit(&uart);

	 ret_code_t err_code = app_timer_stop(m_uarte_timer);
	 APP_ERROR_CHECK(err_code);

 }

/**
 *
 */
 void uart_send(uint8_t * p_data, size_t length) {

	 NRF_LOG_INFO("UART TX of %u bytes", length);

	 ret_code_t err_code = nrfx_uarte_tx(&uart, p_data, length);
	 APP_ERROR_CHECK(err_code);

	 uart_xfer_done = false;

	 while (!uart_xfer_done) {
		 nrf_delay_ms(1);
	 }
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
