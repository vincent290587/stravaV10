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

#define I2C_READ_REG_NO_STOP(addr, p_reg_addr, p_buffer, byte_cnt) \
    NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, NRF_TWI_MNGR_NO_STOP), \
    NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_READ_REG_REP_START(addr, p_reg_addr, p_buffer, byte_cnt) \
    NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, 0), \
    NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_WRITE_REG(addr, p_reg_addr, p_buffer, byte_cnt) \
    NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, NRF_TWI_MNGR_NO_STOP), \
    NRF_TWI_MNGR_WRITE(addr, p_buffer, byte_cnt, 0)

#define I2C_WRITE(addr, p_data, byte_cnt) \
    NRF_TWI_MNGR_WRITE(addr, p_data, byte_cnt, 0)


#ifdef __cplusplus
extern "C" {
#endif

void i2c_init(void);

void i2c_scan(void);

void i2c_schedule(nrf_twi_mngr_transaction_t const * p_transaction);

void i2c_perform(nrf_drv_twi_config_t const *    p_config,
        nrf_twi_mngr_transfer_t const * p_transfers,
        uint8_t                         number_of_transfers,
        void                            (* user_function)(void));


extern bool i2c_read8(uint8_t address, uint8_t *val);

extern bool i2c_read_n(uint8_t address, uint8_t *val, unsigned int len);

extern bool i2c_write_n(uint8_t address, uint8_t *val, unsigned int len);

extern bool i2c_write8(uint8_t address, uint8_t val);

extern bool i2c_write8_cont(uint8_t address, uint8_t val);

extern bool i2c_read_reg_8(uint8_t address, uint8_t reg, uint8_t *val);

extern bool i2c_write_reg_8(uint8_t address, uint8_t reg, uint8_t val);

extern bool i2c_read_reg_n(uint8_t address, uint8_t reg, uint8_t *val, unsigned int len);



#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_I2C_H_ */
