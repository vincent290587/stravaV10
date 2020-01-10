/*
 * nrf_gpio.h
 *
 *  Created on: 3 déc. 2019
 *      Author: vgol
 */

#ifndef TDD_DRIVERS_NRF_GPIO_H_
#define TDD_DRIVERS_NRF_GPIO_H_


/**
 * @brief Macro for mapping port and pin numbers to values understandable for nrf_gpio functions.
 */
#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))


#endif /* TDD_DRIVERS_NRF_GPIO_H_ */
