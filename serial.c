/*
 * serial_handling.c
 *
 *  Created on: 9 août 2017
 *      Author: Vincent
 */

#include "ant.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "app_uart.h"
#include "app_scheduler.h"
#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif
#include "nrf_soc.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "nrf_delay.h"
#include "mk64f_parser.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "glasses.h"


#define TAILLE_BUFFER 30

#define UART_TX_BUF_SIZE 128                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 64                           /**< UART RX buffer size. */


#define RB_SIZE 20
static uint8_t read_byte[RB_SIZE];
static uint16_t status_byte = 0;
static uint16_t marque_byte = 0;

/**
 *
 * @param byte
 * @return
 */
uint8_t encode (uint8_t byte) {

	return 0;
}



/**
 *
 * @param p_event
 */
void uart_event_handler(app_uart_evt_t * p_event)
{
	uint8_t read_byte = 0;

	if (p_event->evt_type == APP_UART_DATA_READY)
	{
		// get data
		while(app_uart_get(&read_byte) != NRF_SUCCESS) {;}

		if (encode(read_byte)) {
			// TODO
//			set_glasses_buffer ();
		}

	}

	if (0) {
		if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
		{
			APP_ERROR_CHECK(p_event->data.error_communication);
		}
		else if (p_event->evt_type == APP_UART_FIFO_ERROR)
		{
			APP_ERROR_CHECK(p_event->data.error_code);
		}
	}
}


/**
 * @brief Function for initializing the UART.
 */
void uart_init(void)
{
	uint32_t                     err_code;
	const app_uart_comm_params_t comm_params =
	{
			RX_PIN_NUMBER,
			TX_PIN_NUMBER,
			RTS_PIN_NUMBER,
			CTS_PIN_NUMBER,
			APP_UART_FLOW_CONTROL_DISABLED,
			false,
			UART_BAUDRATE_BAUDRATE_Baud115200
	};

	APP_UART_FIFO_INIT(&comm_params,
			UART_RX_BUF_SIZE,
			UART_TX_BUF_SIZE,
			uart_event_handler,
			APP_IRQ_PRIORITY_LOW,
			err_code);
	APP_ERROR_CHECK(err_code);
}


