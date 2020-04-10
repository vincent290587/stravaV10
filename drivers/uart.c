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
#include "helper.h"
#include "boards.h"
#include "Model.h"
#include "segger_wrapper.h"
#include "nrf_soc.h"
#include "nrf_queue.h"
#include "nrf_pwr_mgmt.h"
#include "GPSMGMT.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "ring_buffer.h"


#define NRFX_UARTE_INDEX     0

#define UARTE_BUF_LENGTH     16

typedef struct {
	uint8_t rx_buffer[UARTE_BUF_LENGTH];
	size_t  nb_bytes_read;
} sUarteBuffer;

typedef struct {
	sUarteBuffer rx_buf1;
	sUarteBuffer rx_buf2;
	sUarteBuffer *p_readBuffer;
} sUarteDBuffer;

void uart_tasks(void *p_context);

NRF_QUEUE_DEF(sUarteBuffer, m_uart_rx_queue, 50, NRF_QUEUE_MODE_NO_OVERFLOW);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static const nrfx_uarte_t uart = NRFX_UARTE_INSTANCE(NRFX_UARTE_INDEX);

static volatile uint32_t m_error_mask;

static volatile bool uart_xfer_done = true;  /**< Flag used to indicate that SPI instance completed the transfer. */

nrf_uarte_baudrate_t m_baud;

static sUarteDBuffer m_uart_dbuffer;


/**
 *
 * @param p_event
 * @param p_context
 */
static void uart_event_handler(nrfx_uarte_event_t const * p_event,
		void * p_context)
{
	W_SYSVIEW_RecordEnterISR();

	switch (p_event->type) {
	case NRFX_UARTE_EVT_ERROR:
	{
		m_error_mask = p_event->data.error.error_mask;
	}
	break;
	case NRFX_UARTE_EVT_RX_DONE:
	{
		ret_code_t err_code;
		sUarteDBuffer *_buffer = (sUarteDBuffer *) p_context;

		// prepare next transfer
		if (p_event->data.rxtx.p_data == _buffer->rx_buf1.rx_buffer) {
			// buff1 was just filled
			// reset buff1 for RX
			err_code = nrfx_uarte_rx(&uart, _buffer->rx_buf1.rx_buffer, UARTE_BUF_LENGTH);
			_buffer->p_readBuffer = &_buffer->rx_buf1;
		} else {
			// buff2 was just filled
			// reset buff2 for RX
			err_code = nrfx_uarte_rx(&uart, _buffer->rx_buf2.rx_buffer, UARTE_BUF_LENGTH);
			_buffer->p_readBuffer = &_buffer->rx_buf2;
		}
		APP_ERROR_CHECK(err_code);

		_buffer->p_readBuffer->nb_bytes_read = p_event->data.rxtx.bytes;

		// store it in a queue
		if (_buffer->p_readBuffer->nb_bytes_read) {
			err_code = nrf_queue_push(&m_uart_rx_queue, _buffer->p_readBuffer);
			APP_ERROR_CHECK(err_code);
		}

		// remove the delay on the task
		if (m_tasks_id.uart_id != TASK_ID_INVALID) {
			w_task_delay_cancel(m_tasks_id.uart_id);
		}
	}
	break;
	case NRFX_UARTE_EVT_TX_DONE:
	{
		NRF_LOG_DEBUG("UART TX done");
		uart_xfer_done = true;
	}
	break;
	}

	W_SYSVIEW_RecordExitISR();
}

/**
 *
 */
static void uart_trigger_rx(void) {
	ret_code_t err_code;

	nrfx_uarte_rx_abort(&uart);

	err_code = nrfx_uarte_rx(&uart, m_uart_dbuffer.rx_buf1.rx_buffer, UARTE_BUF_LENGTH);
	APP_ERROR_CHECK(err_code);

	err_code = nrfx_uarte_rx(&uart, m_uart_dbuffer.rx_buf2.rx_buffer, UARTE_BUF_LENGTH);
	APP_ERROR_CHECK(err_code);

	LOG_WARNING("UART RX restarted");

	if (m_tasks_id.uart_id == TASK_ID_INVALID) {
		m_tasks_id.uart_id = task_create(uart_tasks, "uart_tasks", NULL);
	}
}

/**
 * @brief Function for initializing the UART.
 */
void uart_init_tx_only(nrf_uarte_baudrate_t baud)
{
	ret_code_t err_code;

	m_baud = baud;

	m_error_mask = 0;

	nrfx_uarte_config_t nrf_uart_config = NRFX_UARTE_DEFAULT_CONFIG;

	nrf_uart_config.pseltxd    = TX_PIN_NUMBER;
	nrf_uart_config.pselrxd    = RX_PIN_NUMBER;
	nrf_uart_config.baudrate   = baud;
	nrf_uart_config.parity     = NRF_UARTE_PARITY_EXCLUDED;
	nrf_uart_config.hwfc       = NRF_UARTE_HWFC_DISABLED;
	nrf_uart_config.p_context  = (void*) &m_uart_dbuffer;

	err_code = nrfx_uarte_init(&uart, &nrf_uart_config, uart_event_handler);
	APP_ERROR_CHECK(err_code);

	LOG_DEBUG("UART configured baud=%u", (uint32_t)baud);

	uart_xfer_done = true;

	memset(&m_uart_dbuffer, 0, sizeof(m_uart_dbuffer));

}

/**
 * @brief Function for initializing the UART.
 */
void uart_init(nrf_uarte_baudrate_t baud)
{
	uart_init_tx_only(baud);

	memset(&m_uart_dbuffer, 0, sizeof(m_uart_dbuffer));

	uart_trigger_rx();

}

/**
 *
 */
void uart_uninit(void) {

	nrfx_uarte_rx_abort(&uart);

	nrfx_uarte_uninit(&uart);

	nrf_delay_us(500);
}

/**
 *
 */
void uart_send(uint8_t * p_data, size_t length) {

	LOG_DEBUG("UART TX of %u bytes", length);

	ret_code_t err_code = nrfx_uarte_tx(&uart, p_data, length);
	APP_ERROR_CHECK(err_code);

	if (!err_code) uart_xfer_done = false;

	while (!uart_xfer_done) {
		pwr_mgmt_run();
	}
}

/**
 *
 */
void uart_tasks(void *p_context) {

	for (;;) {

		if (NRF_UARTE_ERROR_FRAMING_MASK & m_error_mask) {

			LOG_ERROR("UART framing error");
			memset(&m_uart_dbuffer, 0, sizeof(m_uart_dbuffer));

		} else if(NRF_UARTE_ERROR_PARITY_MASK & m_error_mask) {

			LOG_ERROR("UART parity error");
			memset(&m_uart_dbuffer, 0, sizeof(m_uart_dbuffer));

		} else if (NRF_UARTE_ERROR_OVERRUN_MASK & m_error_mask) {

			LOG_ERROR("UART overrun: restarting RX");
			memset(&m_uart_dbuffer, 0, sizeof(m_uart_dbuffer));
			uart_trigger_rx();

		} else if (m_error_mask) {

			LOG_ERROR("UART error %u", m_error_mask);
			memset(&m_uart_dbuffer, 0, sizeof(m_uart_dbuffer));

		}
		m_error_mask = 0;

		/* If buffer is not empty, parse data. */
		while (!nrf_queue_is_empty(&m_uart_rx_queue)) {

			sUarteBuffer _u_buffer;

			// get the next victim
			ret_code_t err_code = nrf_queue_pop(&m_uart_rx_queue, &_u_buffer);
			APP_ERROR_CHECK(err_code);

			// send data for parsing
			gps_encode_array((char*)_u_buffer.rx_buffer, _u_buffer.nb_bytes_read);

		}

		if (task_manager_is_started()) {
			w_task_delay(100);
		}

	}
}
