/*
 * I2C.c
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#include "boards.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_gpio.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "i2c.h"

/* TWI instance ID. */
#define TWI_INSTANCE_ID     1

#define MAX_PENDING_TRANSACTIONS    20

NRF_TWI_MNGR_DEF(m_nrf_twi_mngr, MAX_PENDING_TRANSACTIONS, TWI_INSTANCE_ID);


/**
 *
 */
void i2c_init(void) {
    uint32_t err_code;

    nrf_drv_twi_config_t const config = {
       .scl                = SCL_PIN_NUMBER,
       .sda                = SDA_PIN_NUMBER,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_LOWEST,
       .clear_bus_init     = false
    };

    err_code = nrf_twi_mngr_init(&m_nrf_twi_mngr, &config);
    APP_ERROR_CHECK(err_code);

}

/**
 *
 * @param p_transaction
 */
void i2c_schedule(nrf_twi_mngr_transaction_t const * p_transaction) {

	/* Start master transfer */
	ret_code_t ret_val = nrf_twi_mngr_schedule(&m_nrf_twi_mngr, p_transaction);
	APP_ERROR_CHECK(ret_val);
}


/**
 *
 * @param p_transaction
 */
void i2c_perform(nrf_drv_twi_config_t const *    p_config,
        nrf_twi_mngr_transfer_t const * p_transfers,
        uint8_t                         number_of_transfers,
        void                            (* user_function)(void)) {

	/* Start master transfer */
	ret_code_t ret_val = nrf_twi_mngr_perform(&m_nrf_twi_mngr,
			p_config,
			p_transfers,
			number_of_transfers,
			user_function);
	APP_ERROR_CHECK(ret_val);
}


