/*
 * diskio_sd.c
 *
 *  Created on: 30 mars 2020
 *      Author: vgol
 */

#include "boards.h"

#if defined (USE_MEMORY_SDC)

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "ff.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_power.h"
#include "nrf_drv_clock.h"
#include "ring_buffer.h"
#include "sd_hal.h"
#include "Model.h"

#include "diskio_blkdev.h"
#include "nrf_block_dev_sdc.h"
#include "segger_wrapper.h"

#include "millis.h"
#include "app_error.h"
#include "app_util.h"
#include "app_timer.h"



/**
 * @brief  SDC block device definition
 */
NRF_BLOCK_DEV_SDC_DEFINE(
    m_block_dev_sdc,
    NRF_BLOCK_DEV_SDC_CONFIG(
        SDC_SECTOR_SIZE,
        APP_SDCARD_CONFIG(SDC_MOSI_PIN, SDC_MISO_PIN, SDC_SCK_PIN, SDC_CS_PIN)
     ),
     NFR_BLOCK_DEV_INFO_CONFIG("Nordic", "SDC", "1.00")
);


bool configure_memory() {

	return true;
}

/**
 *
 */
void diskio_sdc_init(void) {

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
			DISKIO_BLOCKDEV_CONFIG(NRF_BLOCKDEV_BASE_ADDR(m_block_dev_sdc, block_dev), NULL)
	};

	diskio_blockdev_register(drives, ARRAY_SIZE(drives));

	for (int i=0; i< 3; i++) {
		int ret = fatfs_init();
		if (ret) {
			LOG_ERROR("FATFS INIT error %d", ret);
		}
	}

	uint32_t blocks_per_mb = (1024uL * 1024uL) / m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_size;
	uint32_t capacity = m_block_dev_sdc.block_dev.p_ops->geometry(&m_block_dev_sdc.block_dev)->blk_count / blocks_per_mb;
	LOG_INFO("Capacity: %d MB", capacity);

}

#endif
