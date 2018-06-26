/*
 * sd_hal.c
 *
 *  Created on: 6 juin 2018
 *      Author: Vincent
 */

#include <stdio.h>
#include <string.h>
#include "boards.h"
#include "segger_wrapper.h"

#include "ff.h"
#include "diskio_blkdev.h"
#include "nrf_block_dev_sdc.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

/**
 * @brief  SDC block device definition
 * */
NRF_BLOCK_DEV_SDC_DEFINE(
        m_block_dev_sdc,
        NRF_BLOCK_DEV_SDC_CONFIG(
                SDC_SECTOR_SIZE,
                APP_SDCARD_CONFIG(SPI_MOSI_PIN, SPI_MISO_PIN, SPI_SCK_PIN, SDC_CS_PIN)
         ),
         NFR_BLOCK_DEV_INFO_CONFIG("Nordic", "SDC", "1.00")
);

static FATFS fs;

/*******************************************************************************
 * Code
 ******************************************************************************/

int sd_functions_init(void) {

	FRESULT ff_result;
	DSTATUS disk_state = STA_NOINIT;

	// Initialize FATFS disk I/O interface by providing the block device.
	static diskio_blkdev_t drives[] =
	{
			DISKIO_BLOCKDEV_CONFIG(NRF_BLOCKDEV_BASE_ADDR(m_block_dev_sdc, block_dev), NULL)
	};

	diskio_blockdev_register(drives, ARRAY_SIZE(drives));

	NRF_LOG_INFO("Initializing disk 0 (SDC)...");
	for (uint32_t retries = 3; retries && disk_state; --retries)
	{
		disk_state = disk_initialize(0);
	}
	if (disk_state)
	{
		NRF_LOG_INFO("Disk initialization failed.");
		return -1;
	}

	uint32_t blocks_per_mb = (1024uL * 1024uL) / m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_size;
	uint32_t capacity = m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_count / blocks_per_mb;
	NRF_LOG_INFO("Capacity: %d MB", capacity);

	NRF_LOG_INFO("Mounting volume...");
	ff_result = f_mount(&fs, "", 1);
	if (ff_result)
	{
		NRF_LOG_INFO("Mount failed.");
		return -2;
	}
	NRF_LOG_INFO("Volume mounted");

	return 0;
}
