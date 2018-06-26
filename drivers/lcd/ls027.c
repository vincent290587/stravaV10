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


#define LS027_HW_SPI_BUFFER_SIZE   (1 + LS027_BUFFER_SIZE + (240*2) + 1)

static uint8_t LS027_SpiBuf[LS027_HW_SPI_BUFFER_SIZE]; /* buffer for the display */

/* some aspects of the protocol are pretty timing sensitive... */
#define LS027_BIT_WRITECMD   (0x01)
#define LS027_BIT_VCOM       (0x02)
#define LS027_BIT_CLEAR      (0x04)
#define LS027_TOGGLE_VCOM    do { LS027_sharpmem_vcom = LS027_sharpmem_vcom ? 0x00 : LS027_BIT_VCOM; } while(0);
#define adagfxswap(a, b) { int16_t t = a; a = b; b = t; }

static const uint8_t set[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };
static const uint8_t clr[] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint8_t LS027_sharpmem_vcom;

static bool m_is_color_inverted = true;

extern nrf_spi_mngr_t m_nrf_spi_mngr;

/*******************************************************************************
 * Functions
 ******************************************************************************/

static void ls027_cs_on() {

	nrf_gpio_pin_set(LS027_CS_PIN);

}

static void ls027_cs_off() {

	nrf_gpio_pin_clear(LS027_CS_PIN);

}

static void ls027_spi_init() {

	nrf_gpio_pin_clear(LS027_CS_PIN);
	nrf_gpio_cfg_output(LS027_CS_PIN);

}

static void ls027_spi_buffer_clear(ret_code_t result, void * p_user_data)
{
	NRF_LOG_INFO("LS027 refreshed");

	ls027_cs_off();

	if (!m_is_color_inverted) {
		memset(LS027_SpiBuf, LS027_PIXEL_GROUP_WHITE, sizeof(LS027_SpiBuf));
	} else {
		memset(LS027_SpiBuf, LS027_PIXEL_GROUP_BLACK, sizeof(LS027_SpiBuf));
	}
}


static int ls027_prepare_buffer(void)
{
	uint16_t addr = 0;

	LS027_SpiBuf[addr++] = (LS027_BIT_WRITECMD | LS027_sharpmem_vcom);

	LS027_TOGGLE_VCOM;

	/* Set the address for lines and dummy data */
	for (int i=0; i < LS027_HW_HEIGHT; i++) {

		// line address
		LS027_SpiBuf[addr++] = i + 1;

		// data bytes
		addr += LS027_HW_WIDTH / 8;

		// dummy data
		LS027_SpiBuf[addr++] = 0x00;

	}

	// dummy data
	LS027_SpiBuf[addr++] = 0x00;

	ASSERT(addr == sizeof(LS027_SpiBuf));

	return 0;
}

/**
 *
 * @param x Col number:  0..400
 * @param y Line number: 0..240
 */
static uint16_t getBufferPixel(uint16_t x, uint16_t y) {

	uint16_t ret_color = 0;

	ret_color = LS027_SpiBuf[2 + (y*LS027_HW_WIDTH + x) / 8 + 2 * y] & set[x & 7];

	return ret_color;
}

/**
 *
 * @param x Col number:  0..400
 * @param y Line number: 0..240
 * @param color Color to be printed
 */
static void setBufferPixel(uint16_t x, uint16_t y, uint16_t color) {

	bool _is_color_inverted = m_is_color_inverted;

	//we simply invert the pixel's color
	if (color == 2) {
		_is_color_inverted = false;
		color = getBufferPixel(x, y) ? 0:1;
	}

	// fill buffer
	if ((color && !_is_color_inverted) ||
			(!color && _is_color_inverted)) {
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
	static uint8_t ls027_clear_buffer[2];

	ls027_clear_buffer[0] = LS027_sharpmem_vcom | LS027_BIT_CLEAR;
	ls027_clear_buffer[1] = 0x00;

	LS027_TOGGLE_VCOM;

	/* Start master transfer */
	spi_schedule(ls027_clear_buffer, 2, NULL, 0);

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
	LS027_sharpmem_vcom = LS027_BIT_VCOM;

	ls027_spi_buffer_clear(0, NULL);

	ls027_spi_init();
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

	ls027_vcom_buffer[0] = LS027_sharpmem_vcom;
	ls027_vcom_buffer[1] = 0x00;

	LS027_TOGGLE_VCOM;

	/* Start master transfer */
	spi_schedule(ls027_vcom_buffer, 2, NULL, 0);

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
	spi_schedule(LS027_SpiBuf, LS027_HW_SPI_BUFFER_SIZE, NULL, 0);

	ls027_cs_off();

	ls027_spi_buffer_clear(0, NULL);
}
