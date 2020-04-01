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
/** @file
 *
 * @{
 * @ingroup zigbee_examples
 * @brief   Simple WS2812-based LED chain driver.
 * @note
 * The physical geometry of LED chain (for example, matrix, ring) is out of scope of this driver.
 * It should be handled by upper-layer module.
 */

#ifndef DRV_WS2812_H__
#define DRV_WS2812_H__

#include <stdint.h>
#include <stdbool.h>

#include "sdk_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**@def DRV_WS2812_LED_CHAIN_PIXELS_COUNT_MAX
 *
 * @brief Maximum number of the WS2812 LEDs in chain supported by the WS2812 driver.
 *
 * @note This value has a direct impact on the amount of RAM required by the driver and
 * on the execution time of @ref drv_ws2812_refresh. Use as little RAM as possible.
 */
#ifndef DRV_WS2812_LED_CHAIN_PIXELS_COUNT_MAX
#define DRV_WS2812_LED_CHAIN_PIXELS_COUNT_MAX    (40U)
#endif

/**@def DRV_WS2812_PWM_INSTANCE_NO
 *
 * @brief Number of the nrfx PWM instance used by the WS2812 driver.
 *
 * @note The number specified in this define is related to the number of the instance in sdk_config.h.
 * You must enable the corresponding instance for the driver to work correctly. For example, when using
 * the instance number 2, set @ref NRFX_PWM2_ENABLED to 1. The PWM module is used exclusively by the driver.
 */
#ifndef DRV_WS2812_PWM_INSTANCE_NO
#define DRV_WS2812_PWM_INSTANCE_NO      0
#endif

/**@brief Typedef of function pointer being called when ws2812 LED chain has just been refreshed.
 *
 * @param p_param   Opaque pointer passed from the application.
 */
typedef void ( * drv_ws2812_refresh_callback_t)(void * p_param);

/**@brief Function for initializing the WS2812 LED chain driver.
 *
 * @param[in] dout_pin  GPIO pin used as DOUT (to be connected to the DIN pin of the first
 *                      WS2812 LED in the chain). Use @ref NRF_GPIO_PIN_MAP to specify value.
 *
 * @retval NRF_SUCCESS     Initialization successful
 * @retval Other           Error during initialization.
 */
uint32_t drv_ws2812_init(uint8_t dout_pin);

/**@brief Function for sending the LED state buffer to the LED chain. Must be called to update the LED visible state.
 *
 * @param[in] p_callback        Pointer to a function called, when LED chain has been refreshed. This function is
 *                              called within ISR context. Pass NULL if no user function should be called.
 * @param[in] p_callback_param  Opaque pointer passed as a parameter when calling p_callback.
 *                              Meaning of the pointer is completely up to the application.
 *
 * @retval NRF_SUCCESS     Operation performed successfully. Callback (if supplied) will be called.
 * @retval NRF_ERROR_BUSY  Previous refresh has not finished yet.
 * @retval Other           Error when performing the operation.
 *
 * @note Calls @ref drv_ws2812_refresh
 */
uint32_t drv_ws2812_display(drv_ws2812_refresh_callback_t p_callback, void * p_callback_param);

/**@brief Function for refreshing the LED chain.
 *
 * @param[in] p_callback        Pointer to a function called, when LED chain has been refreshed
 *                              (After sending data to LED chain and waiting RET Code).
 *                              This function is called within ISR context. Pass NULL if no user function
 *                              should be called.
 * @param[in] p_callback_param  Opaque pointer passed as a parameter when calling p_callback.
 *                              Meaning of the pointer is completely up to the application.
 *
 * Call this function when you only need a refresh of the visible LED state.
 * The function is called by @ref drv_ws2812_display. It can take a significant amount of time
 * to complete (around 30 µs per pixel * @ref WS2812_LED_CHAIN_PIXELS_COUNT_MAX + around 125us (RET Code))
 *
 * @note It is not recommend to use this function from an ISR.
 *
 * @retval NRF_ERROR_BUSY  Previous refresh has not finished yet. Given callback will not be called.
 * @retval NRF_SUCCESS     Refresh has been successfully started. Given callback (if supplied) will be called.
 */
uint32_t drv_ws2812_refresh(drv_ws2812_refresh_callback_t p_callback, void * p_callback_param);

/**@brief Function for checking if the driver is performing the refresh of the LED chain.
 *
 * @retval true     Driver is busy with performing the refresh.
 * @retval false    Driver is in the idle state.
 */
bool drv_ws2812_is_refreshing(void);

/**@brief Function for setting the specified pixel in the LED state buffer to the specified color.
 *
 * @param[in] pixel_no  Number of the pixel in the LED chain.
 *                      Specify a value in the range from 0 to @ref WS2812_LED_CHAIN_PIXELS_COUNT_MAX-1.
 *                      Values out of the range are ignored.
 * @param[in] color     Color to be set. Use the RGB format. Bits 23 to 16 are for the red component,
 *                      bits 15 to 8 are for the green component, and bits 7 to 0 are for the blue component.
 *
 * @note Call @ref drv_ws2812_display to update the LED chain from the frame buffer.
 */
void drv_ws2812_set_pixel(uint32_t pixel_no, uint32_t color);

/**@brief Function for setting all pixels in the LED state buffer to the specified color.
 *
 * @param[in] color     Color to be set. Use the RGB format, as described for @ref drv_ws2812_set_pixel.
 *
 * @note Call @ref drv_ws2812_display to update the LED chain from the frame buffer.
 */
void drv_ws2812_set_pixel_all(uint32_t color);

#ifdef __cplusplus
}
#endif

#endif // DRV_WS2812_H__

/**
 * @}
 */
