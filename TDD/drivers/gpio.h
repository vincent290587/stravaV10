/*
 * gpio.h
 *
 *  Created on: 19 sept. 2018
 *      Author: Vincent
 */

#ifndef DRIVERS_GPIO_H_
#define DRIVERS_GPIO_H_

#include <stdint.h>

/**
 * @brief Macro for mapping port and pin numbers to values understandable for nrf_gpio functions.
 */
#define NRF_GPIO_PIN_MAP(port, pin) (((port) << 5) | ((pin) & 0x1F))

#define FXOS_INT1         NRF_GPIO_PIN_MAP(0, 31)
#define FXOS_INT2         NRF_GPIO_PIN_MAP(0, 29)
#define FXOS_RST          NRF_GPIO_PIN_MAP(0, 2)

#define SST_CS            NRF_GPIO_PIN_MAP(1, 2)

#define SDC_CS_PIN        NRF_GPIO_PIN_MAP(1, 4)

#define LS027_CS_PIN      NRF_GPIO_PIN_MAP(1, 6)

#define BCK_PIN           NRF_GPIO_PIN_MAP(1, 0)

#define SPK_IN            NRF_GPIO_PIN_MAP(0, 26)

#define FIX_PIN           NRF_GPIO_PIN_MAP(0, 8)

#define KILL_PIN          NRF_GPIO_PIN_MAP(1, 10)

#define NEO_PIN           NRF_GPIO_PIN_MAP(0, 4)


#ifdef __cplusplus
extern "C" {
#endif

void gpio_set(uint16_t gpio_nb_);

uint8_t gpio_get(uint16_t gpio_nb_);

void register_btn_press(uint8_t btn_index);

void btn_task(void);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_GPIO_H_ */
