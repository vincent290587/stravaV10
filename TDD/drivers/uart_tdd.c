/*
 * serial_handling.c
 *
 *  Created on: 9 ao�t 2017
 *      Author: Vincent
 */


#include "nordic_common.h"
#include "uart_tdd.h"
#include "Model.h"
#include "Locator.h"
#include "GPSMGMT.h"
#include "segger_wrapper.h"
#include "ring_buffer.h"


/*******************************************************************************
 * Variables
 ******************************************************************************/
#define UART0_RB_SIZE         2048
RING_BUFFER_DEF(uart0_rb1, UART0_RB_SIZE);


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

		// remove the delay on the task
		if (m_tasks_id.uart_id != TASK_ID_INVALID) {
			w_task_delay_cancel(m_tasks_id.uart_id);
		}

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

	LOG_DEBUG("UART configured TX only baud=%u", (uint32_t)baud);
}

/**
 * @brief Function for initializing the UART.
 */
void uart_init(nrf_uarte_baudrate_t baud)
{

	if (m_tasks_id.uart_id == TASK_ID_INVALID) {
		m_tasks_id.uart_id = task_create(uart_tasks, "uart_tasks", 65536, NULL);
	}
}

/**
 *
 */
void uart_uninit(void) {

}

/**
 *
 */
void uart_send(uint8_t * p_data, size_t length) {

	LOG_DEBUG("UART TX of %u bytes", length);
}

/**
 *
 */
void tdd_uart_process(void) {

	/* If ring buffer is not empty, parse data. */
	while (RING_BUFF_IS_NOT_EMPTY(uart0_rb1))
	{
		char c = RING_BUFF_GET_ELEM(uart0_rb1);

		gps_encode_char(c);

		LOG_RAW_INFO(c);

		RING_BUFFER_POP(uart0_rb1);
	}

}

/**
 *
 */
void uart_tasks(void *p_context) {

	for (;;) {

		tdd_uart_process();

		if (task_manager_is_started()) {
			w_task_delay(100);
		}
	}
}

