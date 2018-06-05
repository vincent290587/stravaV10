/*
 * I2C.h
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#ifndef DRIVERS_I2C_H_
#define DRIVERS_I2C_H_

#include <stdint.h>
#include <stdbool.h>
#include "nrf_twi_mngr.h"


#ifdef __cplusplus
extern "C" {
#endif

void i2c_init(void);

void i2c_schedule(nrf_twi_mngr_transaction_t const * p_transaction);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_I2C_H_ */
