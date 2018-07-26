/*
 * sst26_hal.c
 *
 *  Created on: 25 juil. 2018
 *      Author: Vincent
 */

#include <stdbool.h>
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "parameters.h"
#include "boards.h"
#include "spi.h"
#include "nordic_common.h"
#include "segger_wrapper.h"
#include "sst26.h"


static sSpimConfig m_spi_sst_cfg;



void _chip_enable_low() {
	nrf_gpio_pin_clear(SST_CS);
}

void _chip_enable_high() {
	nrf_gpio_pin_set(SST_CS);
}

bool spi_send_sync(const uint8_t *data, size_t data_nb) {

	spi_reconfigure(&m_spi_sst_cfg);

	/* Start master transfer */
	return (0 == spi_schedule(&m_spi_sst_cfg, data, data_nb, NULL, 0));
}

bool spi_receive_sync(uint8_t *data, size_t data_nb) {

	spi_reconfigure(&m_spi_sst_cfg);

	/* Start master transfer */
	return (0 == spi_schedule(&m_spi_sst_cfg, NULL, 0, data, data_nb));
}

bool spi_transceive_sync(const uint8_t *data_in, size_t data_in_nb, uint8_t *data_out, size_t data_out_nb) {

	spi_reconfigure(&m_spi_sst_cfg);

	/* Start master transfer */
	return (0 == spi_schedule(&m_spi_sst_cfg, data_in, data_in_nb, data_out, data_out_nb));
}

void sst_init_spi(void) {

	nrf_gpio_pin_set(SST_CS);
	nrf_gpio_cfg_output(SST_CS);

    m_spi_sst_cfg.frequency      = NRF_SPIM_FREQ_4M;
    m_spi_sst_cfg.handler        = NULL;
    m_spi_sst_cfg.bit_order      = NRF_SPIM_BIT_ORDER_MSB_FIRST;
    m_spi_sst_cfg.blocking       = true;

}
