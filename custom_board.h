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

// LEDs definitions for PCA10036
#define LEDS_NUMBER    1

#define LED_START      14
#define LED_1          14
#define LED_STOP       14

#define LEDS_ACTIVE_STATE 0

#define LEDS_INV_MASK  LEDS_MASK

#define LEDS_LIST { LED_1 }

#define BSP_LED_0      LED_1
#define BSP_LED_1      LED_2
#define BSP_LED_2      LED_3
#define BSP_LED_3      LED_4

#define BUTTONS_NUMBER 3

#define BUTTON_START   17
#define BUTTON_1       17
#define BUTTON_2       18
#define BUTTON_3       19
#define BUTTON_STOP    19
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1, BUTTON_2, BUTTON_3 }

#define BSP_BUTTON_0   BUTTON_1
#define BSP_BUTTON_1   BUTTON_2
#define BSP_BUTTON_2   BUTTON_3

#define SDA_PIN_NUMBER   14
#define SCL_PIN_NUMBER   15

#define RX_PIN_NUMBER    7
#define TX_PIN_NUMBER    6
#define CTS_PIN_NUMBER   11
#define RTS_PIN_NUMBER   8
#define HWFC             false

#define SPI_MISO_PIN     26  // SPI MISO signal.
#define SPI_SS_PIN       28  // SPI CSN signal.
#define SPI_MOSI_PIN     27  // SPI MOSI signal.
#define SPI_SCK_PIN      25  // SPI SCK signal.


// Low frequency clock source to be used by the SoftDevice
#define NRF_CLOCK_LFCLKSRC      {.source       = NRF_CLOCK_LF_SRC_XTAL,      \
                                 .rc_ctiv      = 0,                          \
                                 .rc_temp_ctiv = 0,                          \
                                 .accuracy     = NRF_CLOCK_LF_ACCURACY_20_PPM}


#define INT_PIN           4
#define LDO_PIN           30
#define KILL_PIN          30
#define NEO_PIN           7
#define LED_PIN           14
#define BCK_PIN           8
#define AT42_COUT         BUTTON_2
#define SHARP_CS          29
#define SDC_CS_PIN        30

#define LS027_CS_PIN      0

#define BOARD_PPS_PIN     1
#define BOARD_FIX_PIN     2

#ifdef __cplusplus
}
#endif

#endif // BOARD_CUSTOM_H
