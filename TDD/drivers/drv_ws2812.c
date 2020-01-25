/**
 * Copyright (c) 2019, Nordic Semiconductor ASA
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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "GUI_connector.h"

#include "drv_ws2812.h"

#define WS2812_T1H                  (14U | 0x8000U)
#define WS2812_T0H                  (6U | 0x8000U)

#define LED_CHAIN_TOTAL_BYTE_WIDTH  (DRV_WS2812_LED_CHAIN_PIXELS_COUNT_MAX * 3U)
#define LED_CHAIN_TOTAL_BIT_WIDTH   (LED_CHAIN_TOTAL_BYTE_WIDTH * 8U)

typedef struct
{
    uint8_t g;
    uint8_t r;
    uint8_t b;
} rgb_color_t;

static uint32_t m_color = 0;

static void make_rgb_color(rgb_color_t *rgb_color, uint32_t color)
{
    rgb_color->b = (uint8_t)color;
    color >>= 8;
    rgb_color->g = (uint8_t)color;
    color >>= 8;
    rgb_color->r = (uint8_t)color;
}

static void convert_rgb_to_pwm_sequence(void)
{

}

uint32_t drv_ws2812_init(uint8_t dout_pin)
{   

}

uint32_t drv_ws2812_display(drv_ws2812_refresh_callback_t p_callback, void * p_callback_param)
{
    uint32_t result = 0;
    neopixel_update((m_color & 0xFF0000) >> 16, (m_color & 0xFF00) >> 8, m_color & 0xFF);
    return result;
}

uint32_t drv_ws2812_refresh(drv_ws2812_refresh_callback_t p_callback, void * p_callback_param)
{
    uint32_t result = 0;

    return result;
}

bool drv_ws2812_is_refreshing(void)
{
    return false;
}

void drv_ws2812_set_pixel(uint32_t pixel_no, uint32_t color)
{

}

void drv_ws2812_set_pixel_all(uint32_t color)
{
	m_color = color;
}
