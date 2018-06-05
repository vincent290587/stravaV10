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

void spi_schedule(nrf_spi_mngr_transaction_t const * p_transaction);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_SPI_H_ */
