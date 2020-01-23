/*
 * gpio.h
 *
 *  Created on: 19 sept. 2018
 *      Author: Vincent
 */

#ifndef DRIVERS_GPIO_H_
#define DRIVERS_GPIO_H_

#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

void gpio_set(uint16_t gpio_nb_);

void gpio_clear(uint16_t gpio_nb_);

uint8_t gpio_get(uint16_t gpio_nb_);

void register_btn_press(uint8_t btn_index);

void btn_task(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_GPIO_H_ */
