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
#include "spi.h"
#include "segger_wrapper.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


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

	W_SYSVIEW_OnTaskStopExec(SPI_TASK);

    spi_xfer_done = true;

    NRF_LOG_DEBUG("SPI Xfer %u/%u byte", p_event->xfer_desc.tx_length, p_event->xfer_desc.rx_length);

    ASSERT(p_context);

    sSpimConfig **spi_config = (sSpimConfig**)p_context;

    if (spi_config[0]) {

    	if (spi_config[0]->handler) {
    		//NRF_LOG_INFO("SPI handler");
    		spi_config[0]->handler(p_event, p_context);
    	}
    }
}

/**
 *
 */
void spi_init(void)
{

	m_spi_config.ss_pin         = SPI_SS_PIN;
	m_spi_config.miso_pin       = SPI_MISO_PIN;
	m_spi_config.mosi_pin       = SPI_MOSI_PIN;
	m_spi_config.sck_pin        = SPI_SCK_PIN;

	spi_xfer_done = true;

	p_spi_config[0] = NULL;

	LOG_INFO("SPI init");
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
void spi_uninit(void)
{
	LOG_DEBUG("SPI uninit");
	nrfx_spim_uninit(&spi);

	m_is_started = false;

	nrf_delay_us(200);
}

/**
 *
 * @param p_transaction
 */
void spi_reconfigure (sSpimConfig const * spi_config) {

	ASSERT(spi_config);

	// Adapt the configuration to the current transfer
	if (spi_config != p_spi_config[0]) {

		if (!spi_xfer_done) {
			NRF_LOG_WARNING("SPI Waiting...");
			uint32_t millis_ = millis();
			do {
				// perform system tasks
				perform_system_tasks();

				if (millis() - millis_ > 500) {
					NRF_LOG_ERROR("SPI timeout");
					nrfx_spim_abort(&spi);
					spi_xfer_done = true;
					break;
				}

			} while (!spi_xfer_done);

		}

		// not first transfer
		if (m_is_started) spi_uninit();

		if (!p_spi_config[0]) {
			NRF_LOG_WARNING("SPI configuration set");
		} else {
			NRF_LOG_WARNING("SPI configuration changed");
		}

		// reconfigure
		m_spi_config.bit_order      = spi_config->bit_order;
		m_spi_config.frequency      = spi_config->frequency;

		_spi_start();

		p_spi_config[0] = spi_config;
	}

	ASSERT(m_is_started);

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

	NRF_LOG_DEBUG("SPI Xfer %u/%u byte", tx_length, rx_length);

//	if (!spi_xfer_done) {
//		NRF_LOG_WARNING("SPI Waiting...");
//		millis_ = millis();
//		do {
//			NRF_LOG_PROCESS();
//
//			if (millis() - millis_ > 500) {
//				NRF_LOG_ERROR("SPI timeout");
//				// TODO cancel transfer
//				break;
//			}
//
//		} while (!spi_xfer_done);
//	}

	// potential reconfiguration
	if (spi_config) {
		// reconfiguration
		spi_reconfigure(spi_config);
	}

	if (!m_is_started) {
		// first ever transfer
		_spi_start();
	}

	ASSERT(m_is_started);

	ret_code_t err = nrfx_spim_xfer(&spi, &xfer_desc, 0);
	APP_ERROR_CHECK(err);

	if (!err) {

		W_SYSVIEW_OnTaskStartExec(SPI_TASK);

		// Xfer bytes
		spi_xfer_done = false;
	}

	wdt_reload();

	if (p_spi_config[0]) {
		if (p_spi_config[0]->blocking) {
			// wait for last transfer to finish
			uint32_t millis_ = millis();
			do {
				// perform system tasks
				perform_system_tasks();

				if (millis() - millis_ > 200) {
					LOG_ERROR("SPI timeout TX=%u RX=%u",
							tx_length, rx_length);
					nrfx_spim_abort(&spi);
					spi_xfer_done = true;
					return 1;
				}

			} while (!spi_xfer_done);

			LOG_DEBUG("Xfer took %ums", millis() - millis_);

			W_SYSVIEW_OnTaskStopExec(SPI_TASK);
		}
	}

	return 0;
}


