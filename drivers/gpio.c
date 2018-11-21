/*
 * gpio.c
 *
 *  Created on: 19 sept. 2018
 *      Author: Vincent
 */


#include "nrf_gpio.h"
#include "boards.h"


void gpio_set(uint32_t gpio_nb_) {

	nrf_gpio_pin_set(gpio_nb_);

}

void gpio_clear(uint32_t gpio_nb_) {

	nrf_gpio_pin_clear(gpio_nb_);

}

bool gpio_get(uint32_t gpio_nb_) {

	return nrf_gpio_pin_read(gpio_nb_)!=0;

}
