/*
 * custom_board_v1.h
 *
 *  Created on: 30 nov. 2018
 *      Author: Vincent
 */

#ifndef CUSTOM_BOARD_V1_H_
#define CUSTOM_BOARD_V1_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions
#define LEDS_NUMBER    1

//#define LED_START      14
#define LED_1            NRF_GPIO_PIN_MAP(0, 19)
//#define LED_STOP       14

#define LEDS_ACTIVE_STATE 0

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_LIST { LED_1 }

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4

#define BUTTONS_NUMBER 3

//#define BUTTON_START   17
#define BUTTON_1       NRF_GPIO_PIN_MAP(0, 17)
#define BUTTON_2       NRF_GPIO_PIN_MAP(0, 15)
#define BUTTON_3       NRF_GPIO_PIN_MAP(0, 13)
//#define BUTTON_STOP    19
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2
#define BSP_BUTTON_2   BUTTON_3

#define SDA_PIN_NUMBER   NRF_GPIO_PIN_MAP(1, 15)
#define SCL_PIN_NUMBER   NRF_GPIO_PIN_MAP(1, 13)

#define TX_PIN_NUMBER    NRF_GPIO_PIN_MAP(1, 9)
#define RX_PIN_NUMBER    NRF_GPIO_PIN_MAP(0, 12)
#define CTS_PIN_NUMBER   11
#define RTS_PIN_NUMBER   8
#define HWFC             false

#define LS027_MOSI_PIN    NRF_GPIO_PIN_MAP(0, 22)
#define LS027_SCK_PIN     NRF_GPIO_PIN_MAP(0, 20)

#define QSPI_MISO_PIN     NRF_GPIO_PIN_MAP(0, 24)
#define QSPI_MOSI_PIN     NRF_GPIO_PIN_MAP(1, 4)
#define QSPI_SCK_PIN      NRF_GPIO_PIN_MAP(0, 6) // 6 for proto
#define QSPI_SS_PIN       NRF_GPIO_PIN_MAP(1, 2)
#define QSPI_IO2_PIN      NRF_QSPI_PIN_NOT_CONNECTED
#define QSPI_IO3_PIN      NRF_QSPI_PIN_NOT_CONNECTED

#define FXOS_INT1         NRF_GPIO_PIN_MAP(0, 31)
#define FXOS_INT2         NRF_GPIO_PIN_MAP(0, 29)
#define FXOS_RST          NRF_GPIO_PIN_MAP(0, 2)

#define LS027_CS_PIN      NRF_GPIO_PIN_MAP(1, 6)

#define BCK_PIN           NRF_GPIO_PIN_MAP(1, 0)

#define GPS_R             NRF_GPIO_PIN_MAP(0, 26) // 26 for proto

#define GPS_S             NRF_GPIO_PIN_MAP(0, 25) // unused

#define FIX_PIN           NRF_GPIO_PIN_MAP(0, 8)

#define KILL_PIN          NRF_GPIO_PIN_MAP(1, 10)

#define NEO_PIN           NRF_GPIO_PIN_MAP(0, 4)

//#define USB_PRES          NRF_GPIO_PIN_MAP(0, 6)

#define VEML_PRESENT

#ifdef __cplusplus
}
#endif

#endif /* CUSTOM_BOARD_V1_H_ */
