/*
 * custom_board_v2.h
 *
 *  Created on: 30 nov. 2018
 *      Author: Vincent
 */

#ifndef CUSTOM_BOARD_V2_H_
#define CUSTOM_BOARD_V2_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions
#define LEDS_NUMBER    1

#define LED_1            NRF_GPIO_PIN_MAP(1, 3)
#define LED_START        LED_1
#define LED_STOP         LED_1

#define LEDS_ACTIVE_STATE 0

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_LIST { LED_1 }

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4

#define BUTTONS_NUMBER 3

//#define BUTTON_START   17
#define BUTTON_1       NRF_GPIO_PIN_MAP(0, 14)
#define BUTTON_2       NRF_GPIO_PIN_MAP(0, 13)
#define BUTTON_3       NRF_GPIO_PIN_MAP(0, 11)
//#define BUTTON_STOP    19
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2
#define BSP_BUTTON_2   BUTTON_3

#define SDA_PIN_NUMBER    NRF_GPIO_PIN_MAP(0, 26)
#define SCL_PIN_NUMBER    NRF_GPIO_PIN_MAP(0, 25)

#define TX_PIN_NUMBER     NRF_GPIO_PIN_MAP(0, 5)
#define RX_PIN_NUMBER     NRF_GPIO_PIN_MAP(0, 7)
#define CTS_PIN_NUMBER    0xFF
#define RTS_PIN_NUMBER    0xFF
#define HWFC              false

#define LS027_MOSI_PIN    NRF_GPIO_PIN_MAP(0, 16)
#define LS027_SCK_PIN     NRF_GPIO_PIN_MAP(0, 15)
#define LS027_CS_PIN      NRF_GPIO_PIN_MAP(0, 17)

#define QSPI_MISO_PIN     NRF_GPIO_PIN_MAP(0, 24)
#define QSPI_MOSI_PIN     NRF_GPIO_PIN_MAP(0, 19)
#define QSPI_SCK_PIN      NRF_GPIO_PIN_MAP(0, 18)
#define QSPI_SS_PIN       NRF_GPIO_PIN_MAP(0, 22)
#define QSPI_IO2_PIN      NRF_GPIO_PIN_MAP(0, 23)
#define QSPI_IO3_PIN      NRF_GPIO_PIN_MAP(0, 21)

#define FXOS_INT1         NRF_GPIO_PIN_MAP(0, 28)
#define FXOS_RST          NRF_GPIO_PIN_MAP(0, 27)

#define BCK_PIN           NRF_GPIO_PIN_MAP(1, 8)

#define GPS_R             NRF_GPIO_PIN_MAP(0, 3)
#define GPS_S             NRF_GPIO_PIN_MAP(1, 15)
#define FIX_PIN           NRF_GPIO_PIN_MAP(1, 14)

#define KILL_PIN          NRF_GPIO_PIN_MAP(0, 12)

#define NEO_PIN           NRF_GPIO_PIN_MAP(1, 13)

#define VEML_PRESENT
#define USE_MEMORY_NOR


#ifdef __cplusplus
}
#endif


#endif /* CUSTOM_BOARD_V2_H_ */
