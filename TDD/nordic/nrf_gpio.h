/*
 * nrf_gpio.h
 *
 *  Created on: 3 déc. 2019
 *      Author: vgol
 */

#ifndef TDD_DRIVERS_NRF_GPIO_H_
#define TDD_DRIVERS_NRF_GPIO_H_

#include <stdint.h>


/**
 * @brief Enumerator used for selecting the pin to be pulled down or up at the time of pin
 * configuration.
 */
typedef enum
{
    NRF_GPIO_PIN_NOPULL   = 0, ///<  Pin pull-up resistor disabled.
    NRF_GPIO_PIN_PULLDOWN = 1, ///<  Pin pull-down resistor enabled.
    NRF_GPIO_PIN_PULLUP   = 2,   ///<  Pin pull-up resistor enabled.
} nrf_gpio_pin_pull_t;

/**
 * @brief Macro for mapping port and pin numbers to values understandable for nrf_gpio functions.
 */
#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))


#ifdef __cplusplus
extern "C" {
#endif

void nrf_gpio_cfg_output(uint32_t pin_number);

void nrf_gpio_cfg_input(uint32_t pin_number, nrf_gpio_pin_pull_t pull_config);

#ifdef __cplusplus
}
#endif

#endif /* TDD_DRIVERS_NRF_GPIO_H_ */
