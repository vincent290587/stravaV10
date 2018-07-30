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
#include "boards.h"
#include "nrf_drv_usbd.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_power.h"
#include "usb_parser.h"
#include "usb_cdc.h"
#include "ring_buffer.h"
#include "Model.h"

#include "ff.h"
#include "diskio_blkdev.h"
#include "nrf_block_dev_sdc.h"
#include "segger_wrapper.h"

#include "millis.h"
#include "app_error.h"
#include "app_util.h"
#include "app_timer.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_msc.h"
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

static void msc_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_msc_user_event_t     event);

static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1

#define CDC_X_BUFFERS           (0b1)

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

/**
 * @brief Endpoint list passed to @ref APP_USBD_MSC_GLOBAL_DEF
 */
#define ENDPOINT_LIST() APP_USBD_MSC_ENDPOINT_LIST(1, 1)

/**
 * @brief Mass storage class work buffer size
 */
#define MSC_WORKBUFFER_SIZE (1024)

/**
 * @brief  SDC block device definition
 * */
NRF_BLOCK_DEV_SDC_DEFINE(
        m_block_dev_sdc,
        NRF_BLOCK_DEV_SDC_CONFIG(
                SDC_SECTOR_SIZE,
                APP_SDCARD_CONFIG(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN, SDC_CS_PIN)
         ),
         NFR_BLOCK_DEV_INFO_CONFIG("Nordic", "SDC", "1.00")
);

/**
 * @brief Block devices list passed to @ref APP_USBD_MSC_GLOBAL_DEF

 */
#define BLOCKDEV_LIST() (                                   \
    NRF_BLOCKDEV_BASE_ADDR(m_block_dev_sdc, block_dev),    \
)

/**
 * @brief Mass storage class instance
 */
APP_USBD_MSC_GLOBAL_DEF(m_app_msc,
                        0,
                        msc_user_ev_handler,
                        ENDPOINT_LIST(),
                        BLOCKDEV_LIST(),
                        MSC_WORKBUFFER_SIZE);


#define READ_SIZE 64

static char m_rx_buffer[READ_SIZE];


#define CDC_RB_SIZE         1024
RING_BUFFER_DEF(cdc_rb1, CDC_RB_SIZE);

static char m_tx_buffer[CDC_X_BUFFERS+1][NRF_DRV_USBD_EPSIZE];
static uint16_t m_tx_buffer_bytes_nb[CDC_X_BUFFERS+1];
static uint8_t m_tx_buffer_index = 0;

static volatile bool m_is_xfer_done = true;

static volatile bool m_is_port_open = false;

static uint32_t m_last_buffered = 0;

static FATFS fs;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 * @brief Class specific event handler.
 *
 * @param p_inst    Class instance.
 * @param event     Class specific event.
 */
static void msc_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_msc_user_event_t     event)
{
    UNUSED_PARAMETER(p_inst);
    UNUSED_PARAMETER(event);
}

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
            ret_code_t ret = app_usbd_cdc_acm_read_any(&m_app_cdc_acm,
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

			W_SYSVIEW_OnTaskStopExec(USB_VCOM_TASK);
        } break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {

        	// parse chars
        	size_t bytes_read = app_usbd_cdc_acm_rx_size(&m_app_cdc_acm);

        	LOG_INFO("Bytes RCV: %u", bytes_read);

        	size_t ind = 0;
        	while (bytes_read--) {
        		usb_cdc_decoder(m_rx_buffer[ind++]);
        	}

        	/* Fetch data until internal buffer is empty */
        	ret_code_t ret = app_usbd_cdc_acm_read_any(&m_app_cdc_acm,
        			m_rx_buffer,
					READ_SIZE);


        } break;
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

	NRF_LOG_DEBUG("VCOM Xfer triggered index %u with %u bytes",
			m_tx_buffer_index,
			m_tx_buffer_bytes_nb[m_tx_buffer_index]);

	if (m_is_port_open && m_is_xfer_done)
	{
		ret_code_t ret = app_usbd_cdc_acm_write(&m_app_cdc_acm,
				m_tx_buffer[m_tx_buffer_index],
				m_tx_buffer_bytes_nb[m_tx_buffer_index]);
		APP_ERROR_CHECK(ret);

		if (!ret) {
			m_is_xfer_done = false;

			W_SYSVIEW_OnTaskStartExec(USB_VCOM_TASK);
		}

	} else if (!m_is_xfer_done) {
		NRF_LOG_WARNING("VCOM bytes dropped");
	} else {
		NRF_LOG_DEBUG("Waiting for VCOM connection");
	}

	// reset old buffer
	m_tx_buffer_bytes_nb[m_tx_buffer_index] = 0;

	// switch buffers
	m_tx_buffer_index++;
	m_tx_buffer_index = m_tx_buffer_index & CDC_X_BUFFERS;

	// reset new buffer
	m_tx_buffer_bytes_nb[m_tx_buffer_index] = 0;

	NRF_LOG_DEBUG("VCOM new index %u", m_tx_buffer_index);
}

/**
 *
 * @return
 */
int sd_functions_init(void) {

	FRESULT ff_result;
	DSTATUS disk_state = STA_NOINIT;

	// Initialize FATFS disk I/O interface by providing the block device.
	static diskio_blkdev_t drives[] =
	{
			DISKIO_BLOCKDEV_CONFIG(NRF_BLOCKDEV_BASE_ADDR(m_block_dev_sdc, block_dev), NULL)
	};

	diskio_blockdev_register(drives, ARRAY_SIZE(drives));

	LOG_INFO("Initializing disk 0 (SDC)...");
	for (uint32_t retries = 3; retries && disk_state; --retries)
	{
		disk_state = disk_initialize(0);
	}
	if (disk_state)
	{
		LOG_INFO("Disk initialization failed.");
		return -1;
	}

	uint32_t blocks_per_mb = (1024uL * 1024uL) / m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_size;
	uint32_t capacity = m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_count / blocks_per_mb;
	LOG_INFO("Capacity: %d MB", capacity);

	LOG_INFO("Mounting volume...");
	ff_result = f_mount(&fs, "", 1);
	if (ff_result)
	{
		LOG_INFO("Mount failed.");
		return -2;
	}
	LOG_INFO("Volume mounted");

	return 0;
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

    app_usbd_serial_num_generate();

    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);
    NRF_LOG_INFO("USBD CDC / MSC started.");

//    app_usbd_class_inst_t const * class_inst_msc = app_usbd_msc_class_inst_get(&m_app_msc);
//    ret = app_usbd_class_append(class_inst_msc);
//    APP_ERROR_CHECK(ret);

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
void usb_cdc_close(void) {

    app_usbd_disable();
    app_usbd_stop();

}

/**
 *
 */
void usb_flush(void) {

	/* If ring buffer is not empty, parse data. */
	while (m_is_port_open &&
			RING_BUFF_IS_NOT_EMPTY(cdc_rb1))
	{
		perform_system_tasks();
	}

}

/**
 *
 */
void usb_cdc_tasks(void) {

	/* If ring buffer is not empty, parse data. */
	while (m_is_port_open &&
			m_is_xfer_done &&
			RING_BUFF_IS_NOT_EMPTY(cdc_rb1) &&
			m_tx_buffer_bytes_nb[m_tx_buffer_index] < NRF_DRV_USBD_EPSIZE)
	{
		char c = RING_BUFF_GET_ELEM(cdc_rb1);

		uint16_t ind = m_tx_buffer_bytes_nb[m_tx_buffer_index];
		m_tx_buffer[m_tx_buffer_index][ind] = c;

		// increase index
		m_tx_buffer_bytes_nb[m_tx_buffer_index] += 1;

		RING_BUFFER_POP(cdc_rb1);
	}

	// send if inactive and data is waiting in buffer or if the buffer is almost full
	if ((m_tx_buffer_bytes_nb[m_tx_buffer_index] > (NRF_DRV_USBD_EPSIZE * 3 /4)) ||
			(m_tx_buffer_bytes_nb[m_tx_buffer_index] && millis() - m_last_buffered > 1)) {

		usb_cdc_trigger_xfer();

	}

	while (app_usbd_event_queue_process())
	{
		/* Nothing to do */
	}
}

/**
 * Prints a single char to VCOM
 * @param c
 */
void usb_print(char c) {

	if (RING_BUFF_IS_NOT_FULL(cdc_rb1)) {
		RING_BUFFER_ADD_ATOMIC(cdc_rb1, c);

		m_last_buffered = millis();
	} else {

	}
//
//	uint16_t ind = m_tx_buffer_bytes_nb[m_tx_buffer_index];
//
//	ASSERT(ind <= NRF_DRV_USBD_EPSIZE);
//
//	if (m_tx_buffer_bytes_nb[m_tx_buffer_index] < NRF_DRV_USBD_EPSIZE) {
//		m_tx_buffer[m_tx_buffer_index][ind] = c;
//	} else {
//		// buffer full: trigger xfer to switch buffers
//		usb_cdc_trigger_xfer();
//
//		ASSERT(m_tx_buffer_bytes_nb[m_tx_buffer_index] == 0);
//
//		// process queue
//		usb_cdc_tasks();
//
//		// refresh index
//		ind = m_tx_buffer_bytes_nb[m_tx_buffer_index];
//		// store bytes
//		m_tx_buffer[m_tx_buffer_index][ind] = c;
//	}
//
//	// increase index
//	m_tx_buffer_bytes_nb[m_tx_buffer_index] += 1;
//
//	m_last_buffered = millis();

}

/**
 * Prints a char* to VCOM printf style
 * @param format
 */
void usb_printf(const char *format, ...) {

	va_list args;
	va_start(args, format);

	static char m_usb_char_buffer[128];

	memset(m_usb_char_buffer, 0, sizeof(m_usb_char_buffer));

	int length = vsnprintf(m_usb_char_buffer,
			sizeof(m_usb_char_buffer),
			format, args);

	NRF_LOG_DEBUG("Printfing %d bytes to VCOM", length);

	for (int i=0; i < length; i++) {

		usb_print(m_usb_char_buffer[i]);

	}

	// newline
	usb_print('\r');
	usb_print('\n');

	m_last_buffered = millis();

}
