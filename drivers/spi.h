/*
 * spi.h
 *
 *  Created on: 5 juin 2018
 *      Author: Vincent
 */

#ifndef DRIVERS_SPI_H_
#define DRIVERS_SPI_H_

#include "nrf_spi_mngr.h"

#ifdef __cplusplus
extern "C" {
#endif

void spi_init(void);

void spi_schedule(uint8_t const * p_tx_buffer,
		size_t          tx_length,
		uint8_t       * p_rx_buffer,
		size_t          rx_length);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_SPI_H_ */
