/*
 * diskio_nor.c
 *
 *  Created on: 9 nov. 2018
 *      Author: Vincent
 */


#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "ff.h"
#include "nrf.h"
#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_power.h"
#include "nrf_drv_clock.h"
#include "ring_buffer.h"
#include "sd_hal.h"
#include "Model.h"

#include "diskio_blkdev.h"
#include "nrf_block_dev_qspi.h"
#include "segger_wrapper.h"

#include "millis.h"
#include "app_error.h"
#include "app_util.h"
#include "app_timer.h"


#define NRF_QSPI_CONFIG                                        \
{                                                                       \
    .xip_offset  = NRFX_QSPI_CONFIG_XIP_OFFSET,                         \
    .pins = {                                                           \
       .sck_pin     = QSPI_SCK_PIN,                                 \
       .csn_pin     = QSPI_SS_PIN,                                      \
       .io0_pin     = QSPI_MOSI_PIN,                                \
       .io1_pin     = QSPI_MISO_PIN,                                \
       .io2_pin     = QSPI_IO2_PIN,                                 \
       .io3_pin     = QSPI_IO3_PIN,                                 \
    },                                                                  \
    .irq_priority   = (uint8_t)NRFX_QSPI_CONFIG_IRQ_PRIORITY,           \
    .prot_if = {                                                        \
        .readoc     = (nrf_qspi_readoc_t)NRFX_QSPI_CONFIG_READOC,       \
        .writeoc    = (nrf_qspi_writeoc_t)NRFX_QSPI_CONFIG_WRITEOC,     \
        .addrmode   = (nrf_qspi_addrmode_t)NRFX_QSPI_CONFIG_ADDRMODE,   \
        .dpmconfig  = false,                                            \
    },                                                                  \
    .phy_if = {                                                         \
        .sck_freq   = (nrf_qspi_frequency_t)NRFX_QSPI_CONFIG_FREQUENCY, \
        .sck_delay  = (uint8_t)NRFX_QSPI_CONFIG_SCK_DELAY,              \
        .spi_mode   = (nrf_qspi_spi_mode_t)NRFX_QSPI_CONFIG_MODE,       \
        .dpmen      = false                                             \
    },                                                                  \
}

/**
 * @brief  QSPI block device definition
 */
NRF_BLOCK_DEV_QSPI_DEFINE(
    m_block_dev_qspi,
    NRF_BLOCK_DEV_QSPI_CONFIG(
        512,
        NRF_BLOCK_DEV_QSPI_FLAG_CACHE_WRITEBACK,
        NRF_QSPI_CONFIG
     ),
     NFR_BLOCK_DEV_INFO_CONFIG("stravaV10", "QSPI", "0.01")
);


/**
 *
 */
void diskio_nor_init(void) {

	// clocks init
	ret_code_t err_code;
	err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    LOG_INFO("Starting LF clock");
    nrf_drv_clock_lfclk_request(NULL);
    while(!nrf_drv_clock_lfclk_is_running())
    {
        /* Just waiting */
    	nrf_delay_ms(1);
    }

	// Initialize FATFS disk I/O interface by providing the block device.
	static diskio_blkdev_t drives[] =
	{
			DISKIO_BLOCKDEV_CONFIG(NRF_BLOCKDEV_BASE_ADDR(m_block_dev_qspi, block_dev), perform_system_tasks_light)
	};

	diskio_blockdev_register(drives, ARRAY_SIZE(drives));

	fatfs_init();

	uint32_t blocks_per_mb = (1024uL * 1024uL) / m_block_dev_qspi.block_dev.p_ops->geometry(&m_block_dev_qspi.block_dev)->blk_size;
	uint32_t capacity = m_block_dev_qspi.block_dev.p_ops->geometry(&m_block_dev_qspi.block_dev)->blk_count / blocks_per_mb;
	LOG_INFO("Capacity: %d MB", capacity);

}
