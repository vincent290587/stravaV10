/*
 * spim.c
 *
 *  Created on: 4 juin 2018
 *      Author: Vincent
 */


#include "app_util_platform.h"
#include "app_error.h"
#include "nrfx_spim.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "parameters.h"
#include "spi.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define SPI_INSTANCE  0

#ifdef _DEBUG_SPI
/**< SPI instance index. */
static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
#else
/**< SPI instance index. */
NRF_SPI_MNGR_DEF(m_nrf_spi_mngr, 10, SPI_INSTANCE);
#endif

static nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG;

/**
 *
 */
void spi_init(void)
{

#ifdef _DEBUG_SPI
    spi_config.frequency      = NRF_SPIM_FREQ_4M;
    spi_config.ss_pin         = LS027_CS_PIN;
    spi_config.miso_pin       = SPI_MISO_PIN;
    spi_config.mosi_pin       = SPI_MOSI_PIN;
    spi_config.sck_pin        = SPI_SCK_PIN;
//    spi_config.dcx_pin        = NRFX_SPIM_PIN_NOT_USED;
//    spi_config.use_hw_ss      = true;
//    spi_config.ss_active_high = true;
//    spi_config.ss_duration    = 25U;
    spi_config.bit_order      = NRF_SPIM_BIT_ORDER_LSB_FIRST;

	//APP_ERROR_CHECK(nrfx_spim_init(&spi, &spi_config, NULL, NULL));
#else
	static nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin   = SPI_SS_PIN;
	spi_config.miso_pin = SPI_MISO_PIN;
	spi_config.mosi_pin = SPI_MOSI_PIN;
	spi_config.sck_pin  = SPI_SCK_PIN;
	spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST;
	spi_config.frequency = NRF_DRV_SPI_FREQ_4M;

	APP_ERROR_CHECK(nrf_spi_mngr_init(&m_nrf_spi_mngr, &spi_config));
#endif
	NRF_LOG_INFO("SPI configured");
}

/**
 *
 * @param p_transaction
 */
void spi_schedule (uint8_t const * p_tx_buffer,
		size_t          tx_length,
		uint8_t       * p_rx_buffer,
		size_t          rx_length) {

#ifdef _DEBUG_SPI
	nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX(
			p_tx_buffer, tx_length,
			p_rx_buffer, rx_length);

	NRF_LOG_INFO("Preparing SPI transfer of %u bytes", tx_length);

	nrf_spim_frequency_set(spi.p_reg, spi_config.frequency);

	nrf_spim_configure(spi.p_reg, spi_config.mode, spi_config.bit_order);

	nrf_spim_int_disable(spi.p_reg, NRF_SPIM_INT_END_MASK);

	// Xfer bytes
	APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &xfer_desc, 0));


	nrf_spim_int_enable(spi.p_reg, NRF_SPIM_INT_END_MASK);

	nrf_spim_configure(spi.p_reg, spi_config.mode, NRF_SPIM_BIT_ORDER_MSB_FIRST);

//	while (!spi_xfer_done) {
//		app_sched_execute();
//
//		if (NRF_LOG_PROCESS() == false)
//		{
//			nrf_pwr_mgmt_run();
//		}
//	}
#else
	/* Start master transfer */
	ret_code_t ret_val = nrf_spi_mngr_schedule(&m_nrf_spi_mngr, p_transaction);
	APP_ERROR_CHECK(ret_val);
#endif

}


