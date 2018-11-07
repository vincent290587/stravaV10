/*
 * spim.c
 *
 *  Created on: 4 juin 2018
 *      Author: Vincent
 */

#include "millis.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "nrfx_spim.h"
#include "nrf_gpio.h"
#include "nrf_assert.h"
#include "nrf_delay.h"
#include "boards.h"
#include "Model.h"
#include "parameters.h"
#include "Model.h"
#include "spi.h"
#include "segger_wrapper.h"

#define SPIM_INSTANCE  0

static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(SPIM_INSTANCE);  /**< SPI instance. */

static sSpimConfig const *p_spi_config[1];

static volatile bool spi_xfer_done = true;  /**< Flag used to indicate that SPI instance completed the transfer. */

static nrfx_spim_config_t m_spi_config = NRFX_SPIM_DEFAULT_CONFIG;

static bool m_is_started = false;

/**
 *
 * @param p_event
 * @param p_context
 */
static void spim_event_handler(nrfx_spim_evt_t const * p_event,
                       void *                  p_context)
{
	W_SYSVIEW_RecordEnterISR();

    spi_xfer_done = true;

    NRF_LOG_DEBUG("SPI Xfer %u/%u byte", p_event->xfer_desc.tx_length, p_event->xfer_desc.rx_length);

    ASSERT(p_context);

    sSpimConfig **spi_config = (sSpimConfig**)p_context;

    if (m_tasks_id.ls027_id != TASK_ID_INVALID) {
    	events_set(m_tasks_id.ls027_id, TASK_EVENT_LS027_WAIT_SPI);
    }

    if (spi_config[0]) {

    	if (spi_config[0]->handler) {
    		spi_config[0]->handler(p_event, p_context);
    	}
    }

    W_SYSVIEW_RecordExitISR();
}

/**
 *
 */
static void _spi_start(void)
{
	ret_code_t err_code;

	do {

		err_code = nrfx_spim_init(&spi, &m_spi_config, spim_event_handler, (void*)p_spi_config);
		APP_ERROR_CHECK(err_code);

	} while (err_code);

	m_is_started = true;
	spi_xfer_done = true;

	nrf_delay_us(800);

}

/**
 *
 */
void spi_init(void)
{
	m_spi_config.ss_pin         = NRFX_SPIM_PIN_NOT_USED;
	m_spi_config.miso_pin       = NRFX_SPIM_PIN_NOT_USED;
	m_spi_config.mosi_pin       = LS027_MOSI_PIN;
	m_spi_config.sck_pin        = LS027_SCK_PIN;
	m_spi_config.frequency      = NRF_SPIM_FREQ_4M;
	m_spi_config.bit_order      = NRF_SPIM_BIT_ORDER_LSB_FIRST;

	spi_xfer_done = true;

	p_spi_config[0] = NULL;

	LOG_INFO("SPI init");

	_spi_start();
}

/**
 *
 */
void spi_uninit(void)
{
	LOG_DEBUG("SPI uninit");
	nrfx_spim_uninit(&spi);

	m_is_started = false;

	nrf_delay_us(800);
}

/**
 *
 * @param spi_config Can be NULL or a pointer to reconfigure
 * @param p_tx_buffer
 * @param tx_length
 * @param p_rx_buffer
 * @param rx_length
 * @return 0 if success
 */
int spi_schedule (sSpimConfig const * spi_config,
		uint8_t const * p_tx_buffer,
		size_t          tx_length,
		uint8_t       * p_rx_buffer,
		size_t          rx_length) {

	nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX(
			p_tx_buffer, tx_length,
			p_rx_buffer, rx_length);

	NRF_LOG_INFO("SPI Xfer %u/%u byte", tx_length, rx_length);

	ASSERT(m_is_started);

	if (!spi_xfer_done) {
		if (m_tasks_id.ls027_id != TASK_ID_INVALID) {
			events_wait(TASK_EVENT_LS027_WAIT_SPI);
		} else {
			while (!spi_xfer_done) {
				perform_system_tasks_light();
			}
		}
	}

	p_spi_config[0] = spi_config;

	ret_code_t err = nrfx_spim_xfer(&spi, &xfer_desc, 0);
	APP_ERROR_CHECK(err);

	if (!err) {
		// Xfer bytes
		spi_xfer_done = false;
	} else {
		return 1;
	}

	if (p_spi_config[0]) {
		if (p_spi_config[0]->blocking) {

			if (m_tasks_id.ls027_id != TASK_ID_INVALID) {
				events_wait(TASK_EVENT_LS027_WAIT_SPI);
			} else {
				while (!spi_xfer_done) {
					perform_system_tasks_light();
				}
			}

			LOG_DEBUG("Xfer took %ums", millis() - millis_);
		}

	}

	return 0;
}


