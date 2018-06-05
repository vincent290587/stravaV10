/*
 * spim.c
 *
 *  Created on: 4 juin 2018
 *      Author: Vincent
 */


#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "spi.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/**< SPI instance index. */
#define SPI_INSTANCE  0

NRF_SPI_MNGR_DEF(m_nrf_spi_mngr, 10, SPI_INSTANCE);

/**
 *
 */
void spi_init(void)
{
	static nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin   = SPI_SS_PIN;
	spi_config.miso_pin = SPI_MISO_PIN;
	spi_config.mosi_pin = SPI_MOSI_PIN;
	spi_config.sck_pin  = SPI_SCK_PIN;
	spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST;
	spi_config.frequency = NRF_DRV_SPI_FREQ_4M;

	APP_ERROR_CHECK(nrf_spi_mngr_init(&m_nrf_spi_mngr, &spi_config));

	NRF_LOG_INFO("SPI configured");
}

/**
 *
 * @param p_transaction
 */
void spi_schedule(nrf_spi_mngr_transaction_t const * p_transaction) {

	/* Start master transfer */
	ret_code_t ret_val = nrf_spi_mngr_schedule(&m_nrf_spi_mngr, p_transaction);
	APP_ERROR_CHECK(ret_val);
}
