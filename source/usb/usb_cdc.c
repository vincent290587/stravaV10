/*
 * usb_cdc.c
 *
 *  Created on: 3 juil. 2018
 *      Author: Vincent
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "nrf.h"
#include "nrf_drv_usbd.h"
#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_power.h"
#include "usb_parser.h"
#include "segger_wrapper.h"

#include "millis.h"
#include "app_error.h"
#include "app_util.h"
#include "app_timer.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


/**
 * @brief Enable power USB detection
 *
 * Configure if example supports USB port connection
 */
#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION true
#endif


static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1

#define CDC_X_BUFFERS           (0b11)

/**
 * @brief CDC_ACM class instance
 * */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250
);

#define READ_SIZE 1

static char m_rx_buffer[READ_SIZE];

static char m_tx_buffer[CDC_X_BUFFERS+1][NRF_DRV_USBD_EPSIZE];
static uint16_t m_tx_buffer_bytes_nb[CDC_X_BUFFERS+1];
static uint8_t m_tx_buffer_index = 0;

static volatile bool m_is_xfer_done = true;

static volatile bool m_is_port_open = false;

static uint32_t m_last_buffered = 0;

/**
 * @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t (headphones)
 * */
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
        	m_is_port_open = true;
            /*Setup first transfer*/
            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   m_rx_buffer,
                                                   READ_SIZE);
            UNUSED_VARIABLE(ret);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
        	m_is_port_open = false;
        	m_is_xfer_done = true;
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
        {
        	m_is_xfer_done = true;
        } break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            ret_code_t ret;
            NRF_LOG_INFO("Bytes waiting: %d", app_usbd_cdc_acm_bytes_stored(p_cdc_acm));
            do
            {
                /*Get amount of data transfered*/
                size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
                NRF_LOG_INFO("RX: size: %lu char: %c", size, m_rx_buffer[0]);

                /* Fetch data until internal buffer is empty */
                ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                            m_rx_buffer,
                                            READ_SIZE);

                // parse chars
                //usb_cdc_decoder(m_rx_buffer[0]);

            } while (ret == NRF_SUCCESS);

            LOG_INFO("Bytes waiting: %d", app_usbd_cdc_acm_bytes_stored(p_cdc_acm));

            break;
        }
        default:
            break;
    }
}

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SUSPEND:

            break;
        case APP_USBD_EVT_DRV_RESUME:

            break;
        case APP_USBD_EVT_STARTED:
            break;
        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();

            break;
        case APP_USBD_EVT_POWER_DETECTED:
            NRF_LOG_INFO("USB power detected");

            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            NRF_LOG_INFO("USB power removed");
            app_usbd_stop();
            break;
        case APP_USBD_EVT_POWER_READY:
            NRF_LOG_INFO("USB ready");
            app_usbd_start();
            break;
        default:
            break;
    }
}


/**
 *
 */
static void usb_cdc_trigger_xfer(void) {

	NRF_LOG_INFO("VCOM Xfer triggered index %u with %u bytes",
			m_tx_buffer_index,
			m_tx_buffer_bytes_nb[m_tx_buffer_index]);

	if (m_is_port_open)
	{
		while (!m_is_xfer_done);

		ret_code_t ret = app_usbd_cdc_acm_write(&m_app_cdc_acm,
				m_tx_buffer[m_tx_buffer_index],
				m_tx_buffer_bytes_nb[m_tx_buffer_index]);
		APP_ERROR_CHECK(ret);

		if (!ret) m_is_xfer_done = false;

	} else {
		NRF_LOG_INFO("Waiting for VCOM connection")
	}

	// switch buffers
	m_tx_buffer_index++;
	m_tx_buffer_index = m_tx_buffer_index & CDC_X_BUFFERS;

	NRF_LOG_INFO("VCOM new index %u", m_tx_buffer_index);

	// reset new buffer
	m_tx_buffer_bytes_nb[m_tx_buffer_index] = 0;
}



/**
 *
 */
void usb_cdc_init(void)
{
    ret_code_t ret;
    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };

    ret = nrf_drv_clock_init();
    APP_ERROR_CHECK(ret);

    nrf_drv_clock_lfclk_request(NULL);

    NRF_LOG_INFO("Starting LF clock");
    while(!nrf_drv_clock_lfclk_is_running())
    {
        /* Just waiting */
    	NRF_LOG_RAW_INFO(".");
    	nrf_delay_ms(1);
    }

    ret = app_timer_init();
    APP_ERROR_CHECK(ret);

    app_usbd_serial_num_generate();

    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);
    NRF_LOG_INFO("USBD CDC ACM started.");

    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);

    if (USBD_POWER_DETECTION)
    {
        ret = app_usbd_power_events_enable();
        APP_ERROR_CHECK(ret);
    }
    else
    {
        NRF_LOG_INFO("No USB power detection enabled\r\nStarting USB now");

        app_usbd_enable();
        app_usbd_start();
    }

}

/**
 *
 */
void usb_cdc_tasks(void) {

	while (app_usbd_event_queue_process())
	{
		/* Nothing to do */
	}

	// send if inactive and data is waiting in buffer or if the buffer is almost full
	if (m_tx_buffer_bytes_nb[m_tx_buffer_index] > NRF_DRV_USBD_EPSIZE - 5 ||
			(m_tx_buffer_bytes_nb[m_tx_buffer_index] && millis() - m_last_buffered > 50)) {

		usb_cdc_trigger_xfer();

	}

}


/**
 * Prints a char* to VCOM printf style
 * @param format
 */
void usb_printf(const char *format, ...) {

	va_list args;
	va_start(args, format);

	static char m_usb_char_buffer[256];

	memset(m_usb_char_buffer, 0, sizeof(m_usb_char_buffer));

	int length = vsnprintf(m_usb_char_buffer,
			sizeof(m_usb_char_buffer),
			format, args);

	// add a newline
	if (length+2 < NRF_DRV_USBD_EPSIZE) {
		m_usb_char_buffer[length++] = '\r';
		m_usb_char_buffer[length++] = '\n';
	}

	NRF_LOG_INFO("Printing %d bytes to VCOM", length);

	for (int i=0; i < length; i++) {

		uint16_t ind = m_tx_buffer_bytes_nb[m_tx_buffer_index];

		if (m_tx_buffer_bytes_nb[m_tx_buffer_index] < NRF_DRV_USBD_EPSIZE) {
			m_tx_buffer[m_tx_buffer_index][ind] = m_usb_char_buffer[i];
		} else {
			// buffer full: trigger xfer to switch buffers
			usb_cdc_trigger_xfer();

			// process queue
			usb_cdc_tasks();

			ASSERT(m_tx_buffer_bytes_nb[m_tx_buffer_index] == 0);

			// refrsh index
			ind = m_tx_buffer_bytes_nb[m_tx_buffer_index];
			// store bytes
			m_tx_buffer[m_tx_buffer_index][ind] = m_usb_char_buffer[i];
		}

		// increase index
		m_tx_buffer_bytes_nb[m_tx_buffer_index] += 1;

	}

	m_last_buffered = millis();

}

/**
 * Prints a single char to VCOM
 * @param c
 */
void usb_print(char c) {

	uint16_t ind = m_tx_buffer_bytes_nb[m_tx_buffer_index];

	if (m_tx_buffer_bytes_nb[m_tx_buffer_index] < NRF_DRV_USBD_EPSIZE) {
		m_tx_buffer[m_tx_buffer_index][ind] = c;
	} else {
		// buffer full: trigger xfer to switch buffers
		usb_cdc_trigger_xfer();

		// process queue
		usb_cdc_tasks();

		ASSERT(m_tx_buffer_bytes_nb[m_tx_buffer_index] == 0);

		// refrsh index
		ind = m_tx_buffer_bytes_nb[m_tx_buffer_index];
		// store bytes
		m_tx_buffer[m_tx_buffer_index][ind] = c;
	}

	// increase index
	m_tx_buffer_bytes_nb[m_tx_buffer_index] += 1;

	m_last_buffered = millis();

}
