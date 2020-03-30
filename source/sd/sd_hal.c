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
#include "task_manager_wrapper.h"

#include "ff.h"
#include "diskio_blkdev.h"


static DSTATUS disk_state = STA_NOINIT;
static FATFS fs;

static bool m_is_fat_mounted = false;

#define MEM_TEST_DATA_SIZE 512
static uint8_t m_buffer_tx[MEM_TEST_DATA_SIZE];
static uint8_t m_buffer_rx[MEM_TEST_DATA_SIZE];

static void fatfs_mkfs(void);

extern bool configure_memory();

void format_memory() {

#ifdef USE_MEMORY_NOR
	LOG_WARNING("Process of erasing first block start");

	for (uint16_t i = 0; i < 512; i++) {

		uint32_t err_code = nrfx_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, i * 4096U);
		APP_ERROR_CHECK(err_code);

		LOG_WARNING("Block %d erased...", i);

		w_task_delay(25);
	}
#else

#endif

	return;
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

    LOG_WARNING("Test memory start...");

    for (i = 0; i < MEM_TEST_DATA_SIZE; ++i) {
      m_buffer_tx[i] = (uint8_t)i;
    }
    m_buffer_tx[0] = 0xDB;

    // put memory in given state
    //configure_memory();

#if defined (USE_MEMORY_NOR)
    LOG_WARNING("Process of erasing first block start");
    err_code = nrfx_qspi_erase(QSPI_ERASE_LEN_LEN_4KB, 0);
    APP_ERROR_CHECK(err_code);
#endif

    w_task_delay(25);
    LOG_WARNING("Process of writing data start");
    err_code = disk_write(0, m_buffer_tx, 0, 1);
    APP_ERROR_CHECK(err_code);

    w_task_delay(20);
    disk_ioctl(0, CTRL_SYNC, 0);

    LOG_WARNING("Process of reading data start");

    err_code = disk_read(0, m_buffer_rx, 0, 1);
    APP_ERROR_CHECK(err_code);

    w_task_delay(20);
    LOG_WARNING("Data read");

    int cnt = 0;
    for (i = 0; i < MEM_TEST_DATA_SIZE; i++) {
      if (m_buffer_rx[i] != m_buffer_tx[i]) {
        cnt++;
      }
    }
    LOG_WARNING("--> %u different bytes", cnt);

    LOG_WARNING("Compare...");
    if (memcmp(m_buffer_tx, m_buffer_rx, MEM_TEST_DATA_SIZE) == 0)
    {
    	LOG_WARNING("Data consistent");
    }
    else
    {
    	LOG_WARNING("Data inconsistent");
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

    w_task_yield();

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

	LOG_INFO("Initializing disk 0 ...");
	LOG_FLUSH();
	for (uint32_t retries = 5; retries; --retries)
	{
		disk_state = disk_initialize(0);

		if (!disk_state) break;

		LOG_INFO("Disk initialization failed %u", disk_state);

		delay_ms(1000);
	}

	if (disk_state)
	{
		return -1;
	}

	bool stat;
	for (uint32_t retries = 5; retries; --retries)
	{
		stat =  configure_memory();

		if (stat) break;

		LOG_INFO("Disk initialization failed %u", disk_state);

		delay_ms(1000);
	}

	if (!stat)
	{
		return -2;
	}


	LOG_INFO("Mounting volume...");
	ff_result = f_mount(&fs, "", 1);
	if (ff_result)
	{
		if (ff_result == FR_NO_FILESYSTEM)
		{
			LOG_ERROR("Mount failed. Filesystem not found. Please format device.");
//			fatfs_mkfs();
		}
		else
		{
			LOG_ERROR("Mount failed: %u", ff_result);
		}
	    return -3;
	}
	LOG_INFO("Volume mounted");

	m_is_fat_mounted = true;

	return 0;
}

/**
 *
 * @return
 */
int fatfs_uninit(void) {

	if (!m_is_fat_mounted) return 1;

	LOG_INFO("Un-initializing disk 0...");
	disk_state = disk_uninitialize(0);

	m_is_fat_mounted = false;

	return 0;
}

