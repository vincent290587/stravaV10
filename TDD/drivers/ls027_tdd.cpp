/*
 * ls027_tdd.c
 *
 *  Created on: 18 sept. 2018
 *      Author: Vincent
 */


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <millis.h>
#include <stdbool.h>
#include "ls027.h"
#include "GUI_connector.h"
#include "segger_wrapper.h"


#define LS027_BUFFER_SIZE               ((LS027_DISPLAY_HW_NOF_ROWS * LS027_DISPLAY_HW_NOF_COLUMNS) / 8)

#define LS027_COORD_TO_INDEX(X,Y)       ((Y*LS027_HW_WIDTH + X) / 8)

static const uint8_t set[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static const uint8_t clr[] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };

static bool m_is_color_inverted = false;

uint8_t ls027_tdd_buffer[LS027_BUFFER_SIZE];

/*******************************************************************************
 * Mutex
 ******************************************************************************/

static volatile uint8_t m_ls27_mutex_taken = 0;

static void _ls027_mutex_take(void) {

	while (m_ls27_mutex_taken) {

		w_task_delay(25);

		// mutex timeout
		m_ls27_mutex_taken -= 1;
	}

	m_ls27_mutex_taken = 10;

}

static void _ls027_mutex_give(void) {

	m_ls27_mutex_taken = 0;
}


/////////  STATIC FUNCTIONS

static void ls027_spi_buffer_clear(void) {

	_ls027_mutex_take();

	LOG_DEBUG("LS027 buffers cleared");

	if (!m_is_color_inverted) {
		memset(ls027_tdd_buffer, LS027_PIXEL_GROUP_WHITE, sizeof(ls027_tdd_buffer));
	} else {
		memset(ls027_tdd_buffer, LS027_PIXEL_GROUP_BLACK, sizeof(ls027_tdd_buffer));
	}

	_ls027_mutex_give();
}

/////////  FUNCTIONS

void LS027_Clear(void) {
	ls027_spi_buffer_clear();
}

void LS027_Init(void) {

	LOG_INFO("LS027 Init");

	GUI_connector_init();
}

void LS027_InvertColors(void) {
	m_is_color_inverted = !m_is_color_inverted;
}

void LS027_UpdateFull(void) {

	_ls027_mutex_take();

	GUI_UpdateLS027();

	LOG_INFO("LS027 Updated");

	_ls027_mutex_give();

}

void LS027_ToggleVCOM(void) {

}

uint16_t LS027_get_pixel(uint16_t x, uint16_t y) {

	uint16_t ret_color = 0;

	ret_color = ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] & set[x & 7];

	return ret_color;
}

#define BIT(nr)                 (1UL << (nr))
#define U32_C(x)                x ## U
#define BIT_MASK(h, l)          (((U32_C(1) << ((h) - (l) + 1)) - 1) << (l))


/**
 *
 * @param x Col number:  0..400
 * @param y Line number: 0..240
 * @param color Color to be printed
 */
static inline void setBufferPixelGroup(uint16_t x, uint16_t y, uint8_t nb, uint16_t color) {

	uint8_t mask = BIT_MASK((x & 0b111) + nb - 1, x & 0b111) & 0xFF;

	LOG_DEBUG("setBufferPixelGroup %03u %03u %03u 0x%02X", x, y, nb, mask);

	//we simply invert the pixel's color
	if (color == 2) {
		ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] ^= mask;
		return;
	}

	// fill buffer
	if (color ^ m_is_color_inverted) {
		ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] |= mask;
	} else {
		ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] &= ~mask;
	}

}

/**
 *
 * @param x Col number:  0..400
 * @param y Line number: 0..240
 * @param color Color to be printed
 */
static inline void setBufferPixel(uint16_t x, uint16_t y, uint16_t color) {

	//we simply invert the pixel's color
	if (color == 2) {
		ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] ^= set[x & 7];
		return;
	}

	// fill buffer
	if (color ^ m_is_color_inverted) {
		ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] |= set[x & 7];
	} else {
		ls027_tdd_buffer[LS027_COORD_TO_INDEX(x,y)] &= clr[x & 7];
	}

}

/*!
    @brief Draws a pixels with consecutive x in image buffer

    @param[in]  x
                The x position (0 based)
    @param[in]  y
                The y position (0 based)
 */
void LS027_drawPixelGroup(uint16_t x, uint16_t y, uint16_t nb, uint16_t color) {

	uint8_t index;

	// loop pixels
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

void LS027_drawPixel(uint16_t x, uint16_t y, uint16_t color) {

	setBufferPixel(x, y, color);

}
