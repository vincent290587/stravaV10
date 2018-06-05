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

#ifdef __cplusplus
extern "C" {
#endif

void i2c_init(void);

bool i2c_device_present(uint8_t address);

void i2c_scan();

bool i2c_read8(uint8_t address, uint8_t *val);

bool i2c_read_n(uint8_t address, uint8_t *val, unsigned int len);

bool i2c_write_n(uint8_t address, uint8_t *val, unsigned int len);

bool i2c_write8(uint8_t address, uint8_t val);

bool i2c_write8_cont(uint8_t address, uint8_t val);

bool i2c_read_reg_8(uint8_t address, uint8_t reg, uint8_t *val);

bool i2c_write_reg_8(uint8_t address, uint8_t reg, uint8_t val);

bool i2c_read_reg_n(uint8_t address, uint8_t reg, uint8_t *val, unsigned int len);


#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_I2C_H_ */
