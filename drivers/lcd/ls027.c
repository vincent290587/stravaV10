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

static nrf_drv_spi_config_t  ls027_spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;

extern nrf_spi_mngr_t m_nrf_spi_mngr;

static void ls027_cs_on(void * p_user_data) {
	nrf_gpio_pin_set(LS027_CS_PIN);
	nrf_delay_us(3);
}

static void ls027_cs_off(ret_code_t result, void * p_user_data) {
	nrf_delay_us(3);
	nrf_gpio_pin_clear(LS027_CS_PIN);

	APP_ERROR_CHECK(result);
}

static void ls027_spi_init() {

	ls027_cs_off(0, NULL);
	nrf_gpio_cfg_output(LS027_CS_PIN);

	ls027_spi_config.ss_pin   = NRF_DRV_SPI_PIN_NOT_USED;
	ls027_spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED;
	ls027_spi_config.mosi_pin = SPI_MOSI_PIN;
	ls027_spi_config.sck_pin  = SPI_SCK_PIN;
	ls027_spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST;
	ls027_spi_config.frequency = NRF_DRV_SPI_FREQ_2M;

}

static void ls027_spi_buffer_clear(ret_code_t result, void * p_user_data)
{
	NRF_LOG_DEBUG("LS027 refreshed");

	ls027_cs_off(result, p_user_data);

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

	// Below structures have to be "static" - they cannot be placed on stack
	// since the transaction is scheduled and these structures most likely
	// will be referred after this function returns
	static nrf_spi_mngr_transfer_t NRF_SPI_MNGR_BUFFER_LOC_IND ls027_spi_xfer = \
			NRF_SPI_MNGR_TRANSFER(ls027_clear_buffer, 2, NULL, 0);

	static nrf_spi_mngr_transaction_t const transaction_clear =
	{
			.begin_callback      = ls027_cs_on,
			.end_callback        = ls027_cs_off,
			.p_user_data         = NULL,
			.p_transfers         = &ls027_spi_xfer,
			.number_of_transfers = 1,
			.p_required_spi_cfg  = &ls027_spi_config
	};

	/* Start master transfer */
	spi_schedule(&transaction_clear);

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

	// Below structures have to be "static" - they cannot be placed on stack
	// since the transaction is scheduled and these structures most likely
	// will be referred after this function returns
	static nrf_spi_mngr_transfer_t NRF_SPI_MNGR_BUFFER_LOC_IND ls027_spi_xfer = \
			NRF_SPI_MNGR_TRANSFER(ls027_vcom_buffer, 2, NULL, 0);

	static nrf_spi_mngr_transaction_t const transaction_vcom =
	{
			.begin_callback      = ls027_cs_on,
			.end_callback        = ls027_cs_off,
			.p_user_data         = NULL,
			.p_transfers         = &ls027_spi_xfer,
			.number_of_transfers = 1,
			.p_required_spi_cfg  = &ls027_spi_config
	};

	/* Start master transfer */
	spi_schedule(&transaction_vcom);

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

	// program the task
	// Below structures have to be "static" - they cannot be placed on stack
	// since the transaction is scheduled and these structures most likely
	// will be referred after this function returns
	static nrf_spi_mngr_transfer_t NRF_SPI_MNGR_BUFFER_LOC_IND ls027_spi_xfer = \
			NRF_SPI_MNGR_TRANSFER(LS027_SpiBuf, LS027_HW_SPI_BUFFER_SIZE, NULL, 0);

	static nrf_spi_mngr_transaction_t const transaction_data =
	{
			.begin_callback      = ls027_cs_on,
			.end_callback        = ls027_spi_buffer_clear,
			.p_user_data         = NULL,
			.p_transfers         = &ls027_spi_xfer,
			.number_of_transfers = 1,
			.p_required_spi_cfg  = &ls027_spi_config
	};

	/* Start master transfer */
	spi_schedule(&transaction_data);
}
