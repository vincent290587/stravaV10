/**
 * Copyright (c) 2017 - 2018, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup bootloader_open_usb_main main.c
 * @{
 * @ingroup bootloader_open_usb
 * @brief Bootloader project main file for Open DFU over USB.
 *
 */

#include <stdint.h>
#include "boards.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "app_error.h"
#include "app_error_weak.h"

#include "nrf_drv_systick.h"
#include "nrf_clock.h"

#include "boards.h"

#define PIN_NB     13

const uint32_t pin_table[PIN_NB] = {
		SDA_PIN_NUMBER,
		SCL_PIN_NUMBER,

		TX_PIN_NUMBER,
		RX_PIN_NUMBER,

		LS027_MOSI_PIN,
		LS027_SCK_PIN,
		LS027_CS_PIN,

		QSPI_MISO_PIN,
		QSPI_MOSI_PIN,
		QSPI_SCK_PIN,
		QSPI_SS_PIN,
		QSPI_IO2_PIN,
		QSPI_IO3_PIN
};



void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    NRF_LOG_ERROR("Received a fault! id: 0x%08x, pc: 0x%08x, info: 0x%08x", id, pc, info);

}



/**@brief Function for application main entry.
 */
int main(void)
{
    uint32_t ret_val;

    ret_val = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(ret_val);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

    /* Init systick driver */
    nrf_drv_systick_init();

    NRF_LOG_INFO("HW tester started");
    NRF_LOG_FLUSH();

    // init pins
    const uint16_t max_size = PIN_NB;

    for (uint16_t i=0; i < max_size; i++) {
    	nrf_gpio_cfg_output(pin_table[i]);
    	nrf_gpio_pin_clear(pin_table[i]);
    }

    for (;;) {

        for (uint16_t i=0; i < max_size; i++) {
        	nrf_gpio_pin_toggle(pin_table[i]);
        }
        NRF_LOG_INFO("inverting pins");
        nrf_drv_systick_delay_ms(100);

    }
}

/**
 * @}
 */
