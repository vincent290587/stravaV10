/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */
#ifndef BOARD_CUSTOM_H
#define BOARD_CUSTOM_H

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

#define FXOS_INT1         NRF_GPIO_PIN_MAP(0, 31)
#define FXOS_INT2         NRF_GPIO_PIN_MAP(0, 29)
#define FXOS_RST          NRF_GPIO_PIN_MAP(0, 2)

#define LS027_CS_PIN      NRF_GPIO_PIN_MAP(1, 6)

#define BCK_PIN           NRF_GPIO_PIN_MAP(1, 0)

#define GPS_R             NRF_GPIO_PIN_MAP(0, 26)

#define FIX_PIN           NRF_GPIO_PIN_MAP(0, 8)

#define KILL_PIN          NRF_GPIO_PIN_MAP(1, 10)

#define NEO_PIN           NRF_GPIO_PIN_MAP(0, 4)

//#define USB_PRES          NRF_GPIO_PIN_MAP(0, 6)

#ifdef __cplusplus
}
#endif

#endif // BOARD_CUSTOM_H
