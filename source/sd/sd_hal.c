/*
 * sd_hal.c
 *
 *  Created on: 6 juin 2018
 *      Author: Vincent
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "boards.h"
#include "millis.h"
#include "nrfx_qspi.h"
#include "nor_defines.h"
#include "sd_hal.h"
#include "segger_wrapper.h"

#include "ff.h"
#include "diskio_blkdev.h"


static DSTATUS disk_state = STA_NOINIT;
static FATFS fs;

static bool m_is_fat_mounted = false;

#define QSPI_TEST_DATA_SIZE 32
static uint8_t m_buffer_tx[QSPI_TEST_DATA_SIZE];
static uint8_t m_buffer_rx[QSPI_TEST_DATA_SIZE];

static void fatfs_mkfs(void);

extern bool configure_memory();

void format_memory() {

	LOG_WARNING("Formatting...");
	uint32_t err_code;
	nrf_qspi_cinstr_conf_t cinstr_cfg = {
			.opcode    = 0xc7,
			.length    = NRF_QSPI_CINSTR_LEN_1B,
			.io2_level = true,
			.io3_level = true,
			.wipwait   = true,
			.wren      = true
	};

	// Global format
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
	APP_ERROR_CHECK(err_code);

	LOG_WARNING("Memory formatted");
}

void fmkfs_memory(void) {

	LOG_WARNING("Make FS start");

	fatfs_mkfs();

	LOG_WARNING("FS created !");
}

void test_memory(void)
{
    uint32_t i;
    uint32_t err_code;

    NRF_LOG_INFO("QSPI write and read example");

    for (i = 0; i < QSPI_TEST_DATA_SIZE; ++i) {
      m_buffer_tx[i] = (uint8_t)i;
    }
    m_buffer_tx[0] = 0xDB;

    configure_memory();

    NRF_LOG_INFO("Process of erasing first block start");
    err_code = nrfx_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, 0);
    APP_ERROR_CHECK(err_code);

    delay_ms(1000);

    NRF_LOG_INFO("Process of writing data start");
    err_code = nrfx_qspi_write(m_buffer_tx, QSPI_TEST_DATA_SIZE, 0);
    APP_ERROR_CHECK(err_code);

    delay_ms(30);

    NRF_LOG_INFO("Process of reading data start");
    err_code = nrfx_qspi_read(m_buffer_rx, QSPI_TEST_DATA_SIZE, 0);

    delay_ms(30);
    NRF_LOG_INFO("Data read");

    int cnt = 0;
    for (int i = 0; i < QSPI_TEST_DATA_SIZE; i++) {
      if (m_buffer_rx[i] != m_buffer_tx[i]) {
        cnt++;
      }
    }
    NRF_LOG_INFO("--> %u different bytes", cnt);

    NRF_LOG_INFO("Compare...");
    if (memcmp(m_buffer_tx, m_buffer_rx, QSPI_TEST_DATA_SIZE) == 0)
    {
        NRF_LOG_INFO("Data consistent");
    }
    else
    {
        NRF_LOG_INFO("Data inconsistent");
    }

}

/**
 *
 * @return True if fatfs is init
 */
bool is_fat_init(void) {
	return m_is_fat_mounted;
}


static void fatfs_mkfs(void)
{
    FRESULT ff_result;

    LOG_INFO("\r\nCreating filesystem...");
    static uint8_t buf[512];
    ff_result = f_mkfs("", FM_FAT, 1024, buf, sizeof(buf));
    if (ff_result != FR_OK)
    {
        LOG_ERROR("Mkfs failed.");
        return;
    }

    LOG_INFO("Mounting volume...");
    ff_result = f_mount(&fs, "", 1);
    if (ff_result != FR_OK)
    {
        LOG_ERROR("Mount failed.");
        return;
    }

    LOG_INFO("Done");
}

/**
 *
 * @return 0 if success
 */
int fatfs_init(void) {

	FRESULT ff_result;

	if (m_is_fat_mounted) return 0;

    memset(&fs, 0, sizeof(FATFS));

	LOG_INFO("Initializing disk 0 (QSPI)...");
	LOG_FLUSH();
	for (uint32_t retries = 5; retries && disk_state; --retries)
	{
		disk_state = disk_initialize(0);
		configure_memory();

		if (!disk_state) break;

		LOG_INFO("Disk initialization failed %u", disk_state);

		delay_ms(1000);
	}

	//test_memory();

	if (disk_state)
	{
		return -1;
	}

	LOG_INFO("Mounting volume...");
	ff_result = f_mount(&fs, "", 1);
	if (ff_result)
	{
		if (ff_result == FR_NO_FILESYSTEM)
		{
			LOG_ERROR("Mount failed. Filesystem not found. Please format device.");
			//fatfs_mkfs();
		}
		else
		{
			LOG_ERROR("Mount failed: %u", ff_result);
		}
	    return -2;
	}
	LOG_INFO("Volume mounted");

	m_is_fat_mounted = true;

	return 0;
}

/**
 *
 * @return always 0
 */
int fatfs_uninit(void) {

	if (!m_is_fat_mounted) return 0;

	LOG_INFO("Un-initializing disk 0...");
	disk_state = disk_uninitialize(0);

	m_is_fat_mounted = false;

	return 0;
}

