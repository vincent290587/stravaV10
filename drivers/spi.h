/*
 * spi.h
 *
 *  Created on: 5 juin 2018
 *      Author: Vincent
 */

#ifndef DRIVERS_SPI_H_
#define DRIVERS_SPI_H_

#include "nrfx_spim.h"

/**
 * @brief SPIM master driver instance configuration structure.
 */
typedef struct
{
    nrfx_spim_evt_handler_t handler;
    bool blocking;
} sSpimConfig;

#ifdef __cplusplus
extern "C" {
#endif

void spi_init(void);

void spi_uninit(void);

int spi_schedule(sSpimConfig const * spi_config,
		uint8_t const * p_tx_buffer,
		size_t          tx_length,
		uint8_t       * p_rx_buffer,
		size_t          rx_length);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_SPI_H_ */
