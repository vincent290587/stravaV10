/*
 * ls027.c
 *
 *  Created on: 29 sept. 2017
 *      Author: Vincent
 */

#include <stdbool.h>
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "parameters.h"
#include "spi.h"
#include "boards.h"
#include "nordic_common.h"
#include "segger_wrapper.h"

#include "ls027.h"
#include "ls027_splash.h"


#define LS027_HW_SPI_BUFFER_SIZE   (1 + LS027_BUFFER_SIZE + (240*2) + 1)

static uint8_t LS027_SpiBuf[LS027_HW_SPI_BUFFER_SIZE]; /* buffer for the display */

/* some aspects of the protocol are pretty timing sensitive... */
#define LS027_BIT_WRITECMD   (0x01)
#define LS027_BIT_VCOM       (0x02)
#define LS027_BIT_CLEAR      (0x04)

#define LS027_M0_L           (0x00)
#define LS027_M0_H           (0x01)
#define LS027_M1_L           (0x00)
#define LS027_M1_H           (0x02)
#define LS027_M2_L           (0x00)
#define LS027_M2_H           (0x04)


#define LS027_TOGGLE_VCOM    do { m_M1_bit = m_M1_bit ? 0x00 : LS027_BIT_VCOM; } while(0);

static const uint8_t set[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static const uint8_t clr[] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint8_t m_M1_bit;

static bool m_is_color_inverted = true;

static sSpimConfig m_spi_ls027_cfg;

/*******************************************************************************
 * Functions
 ******************************************************************************/

static void ls027_cs_on() {

	nrf_gpio_pin_set(LS027_CS_PIN);

}

static void ls027_cs_off(nrfx_spim_evt_t const * p_event,
        void *                  p_context) {

	nrf_gpio_pin_clear(LS027_CS_PIN);

}

static void ls027_spi_buffer_clear(ret_code_t result, void * p_user_data) {
	NRF_LOG_DEBUG("LS027 buffers cleared");

	if (!m_is_color_inverted) {
		memset(LS027_SpiBuf, LS027_PIXEL_GROUP_WHITE, sizeof(LS027_SpiBuf));
	} else {
		memset(LS027_SpiBuf, LS027_PIXEL_GROUP_BLACK, sizeof(LS027_SpiBuf));
	}
}

static void ls027_spi_init() {

	nrf_gpio_pin_clear(LS027_CS_PIN);
	nrf_gpio_cfg_output(LS027_CS_PIN);

	m_spi_ls027_cfg.handler        = ls027_cs_off;
	m_spi_ls027_cfg.blocking       = false;
}

static int ls027_prepare_buffer(void)
{
	uint16_t addr = 0;

	LS027_SpiBuf[addr++] = m_M1_bit | LS027_M0_H | LS027_M2_L;

	LS027_TOGGLE_VCOM;

	/* Set the address for lines and dummy data */
	for (int i=0; i < LS027_HW_HEIGHT; i++) {

		// line address
		LS027_SpiBuf[addr++] = i + 1;

		// data bytes
		addr += LS027_HW_WIDTH >> 3u;

		// dummy data
		LS027_SpiBuf[addr++] = 0x00;

	}

	// dummy data
	LS027_SpiBuf[addr++] = 0x00;

	ASSERT(addr == sizeof(LS027_SpiBuf));

	return 0;
}

static void ls027_hw_clear(void) {

	static uint8_t ls027_clear_buffer[2];

	ls027_clear_buffer[0] = m_M1_bit | LS027_M0_L | LS027_M2_H;
	ls027_clear_buffer[1] = 0x00;

	LS027_TOGGLE_VCOM;

	ls027_cs_on();

	/* Start master transfer */
	spi_schedule(&m_spi_ls027_cfg, ls027_clear_buffer, 2, NULL, 0);

}

/**
 *
 * @param x Col number:  0..400
 * @param y Line number: 0..240
 */
static inline uint16_t getBufferPixel(uint16_t x, uint16_t y) {

	uint16_t ret_color = 0;

	ret_color = LS027_SpiBuf[2 + (y*LS027_HW_WIDTH + x) / 8 + 2 * y] & set[x & 7];

	return ret_color;
}


#define BIT(nr)                 (1UL << (nr))
#define U32_C(x)                x ## U
#define BIT_MASK(h, l)          (((U32_C(1) << ((h) - (l) + 1)) - 1) << (l))

/**
 *
 * @param x Col number:  0..400
 * @param y Line number: 0..240
 * @param nb Number of pixels to target
 * @param color Color to be printed
 */
static inline void setBufferPixelGroup(uint16_t x, uint16_t y, uint8_t nb, uint16_t color) {

	uint8_t mask = BIT_MASK((x & 0b111) + nb - 1, x & 0b111) & 0xFF;

	LOG_DEBUG("setBufferPixelGroup %03u %03u %03u 0x%02X", x, y, nb, mask);

	//we simply invert the pixel's color
	if (color == 2) {
		LS027_SpiBuf[2 + (y*LS027_HW_WIDTH + x) / 8 + 2 * y] ^= mask;
		return;
	}

	// fill buffer
	if (color ^ m_is_color_inverted) {
		LS027_SpiBuf[2 + (y*LS027_HW_WIDTH + x) / 8 + 2 * y] |= mask;
	} else {
		LS027_SpiBuf[2 + (y*LS027_HW_WIDTH + x) / 8 + 2 * y] &= ~mask;
	}

}

/**
 *
 * @param x Col number:  0..400
 * @param y Line number: 0..240
 * @param color Color to be printed
 */
static inline void setBufferPixel(uint16_t x, uint16_t y, uint16_t color) {

	// we simply invert the pixel's color
	if (color == 2) {
		LS027_SpiBuf[2 + (y*LS027_HW_WIDTH + x) / 8 + 2 * y] ^= set[x & 7];
		return;
	}

	// set the good color
	if (color ^ m_is_color_inverted) {
		LS027_SpiBuf[2 + (y*LS027_HW_WIDTH + x) / 8 + 2 * y] |= set[x & 7];
	} else {
		LS027_SpiBuf[2 + (y*LS027_HW_WIDTH + x) / 8 + 2 * y] &= clr[x & 7];
	}

}

/*
 ** ===================================================================
 **     Method      :  LS027_Clear (component SharpMemDisplay)
 **     Description :
 **         Clears the whole display memory.
 **     Parameters  : None
 **     Returns     : Nothing
 ** ===================================================================
 */
void LS027_Clear(void)
{
	ls027_spi_buffer_clear(0, NULL);
	return;
}

/*
 ** ===================================================================
 **     Method      :  LS027_Init (component SharpMemDisplay)
 **     Description :
 **         Display driver initialization
 **     Parameters  : None
 **     Returns     : Nothing
 ** ===================================================================
 */
void LS027_Init(void)
{
	/* Set the vcom bit to a defined state */
	m_M1_bit = LS027_BIT_VCOM;

	ls027_spi_init();

	ls027_hw_clear();

	// copy buffer
	uint16_t offset = 0;
	for (int i=0; i < LS027_BUFFER_SIZE; i++) {

		offset = 2 * (8 * i / LS027_HW_WIDTH);

		LS027_SpiBuf[2 + i + offset] = SPLASH_BMP[LS027_BUFFER_SIZE - i - 1];

	}

	LS027_UpdateFull();
}

void LS027_InvertColors(void)
{
	m_is_color_inverted = !m_is_color_inverted;
}

/*!
    @brief Draws a single pixel in image buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)
 */
void LS027_drawPixel(uint16_t x, uint16_t y, uint16_t color) {
	setBufferPixel(x, y, color);
}

/*!
    @brief Draws a pixels with consecutive x in image buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)
 */
void LS027_drawPixelGroup(uint16_t x, uint16_t y, uint16_t nb, uint16_t color) {

	// loop pixels
	uint8_t index;
	while (nb) {
		index = x & 0b111;

		if (nb < 8 - index) {
			setBufferPixelGroup(x, y, nb, color);
			index = 8 - nb;
		} else {
			setBufferPixelGroup(x, y, 8 - index, color);
		}

		LOG_DEBUG("setBufferPixelGroup %03u %03u %03u %d", x, y, nb, index);

		// next group of pixels
		nb -= 8 - index;
		x  += 8 - index;
	}

}

/*
 ** ===================================================================
 **     Method      :  LS027_ToggleVCOM (component SharpMemDisplay)
 **     Description :
 **         Command used if EXTMODE=L. This function toggles the VCOM if
 **         no other command is executed
 **     Parameters  : None
 **     Returns     : Nothing
 ** ===================================================================
 */
void LS027_ToggleVCOM(void)
{
	/* send toggle VCOM command */
	static uint8_t ls027_vcom_buffer[2];

	ls027_vcom_buffer[0] = m_M1_bit | LS027_M0_L | LS027_M2_L;
	ls027_vcom_buffer[1] = 0x00;

	LS027_TOGGLE_VCOM;

	ls027_cs_on();

	/* Start master transfer */
	spi_schedule(&m_spi_ls027_cfg, ls027_vcom_buffer, 2, NULL, 0);

	ls027_cs_off(NULL, NULL);

	return;
}

/*
 ** ===================================================================
 **     Method      :  LS027_UpdateFull (component SharpMemDisplay)
 **     Description :
 **         Updates the whole display
 **     Parameters  : None
 **     Returns     : Nothing
 ** ===================================================================
 */

void LS027_UpdateFull(void)
{
	// prepare the SPI commands
	ls027_prepare_buffer();

	ls027_cs_on();

	/* Start master transfer */
	spi_schedule(&m_spi_ls027_cfg, LS027_SpiBuf, LS027_HW_SPI_BUFFER_SIZE, NULL, 0);
}
