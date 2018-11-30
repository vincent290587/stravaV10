/*
 * sd_hal.c
 *
 *  Created on: 6 juin 2018
 *      Author: Vincent
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ff.h"
#include "diskio.h"
#include "segger_wrapper.h"


static DSTATUS disk_state = STA_NOINIT;
static FATFS fs;

static bool m_is_fat_mounted = false;


/**
 *
 * @return True if fatfs is init
 */
bool is_fat_init(void) {
	return m_is_fat_mounted;
}

/**
 *
 */
void format_memory(void) {

}

/**
 *
 * @return 0 if success
 */
int fatfs_init(void) {

	FRESULT ff_result;

	if (m_is_fat_mounted) return 0;

    memset(&fs, 0, sizeof(FATFS));

	LOG_INFO("Initializing disk 0 (SDC)...");
	for (uint32_t retries = 3; retries && disk_state; --retries)
	{
		disk_state = disk_initialize(0);
	}
	if (disk_state)
	{
		LOG_INFO("Disk initialization failed.");
		return -1;
	}

	LOG_INFO("Mounting volume...");
	ff_result = f_mount(&fs, "", 1);
	if (ff_result)
	{
		LOG_INFO("Mount failed.");
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
	disk_state = STA_NOINIT;

	m_is_fat_mounted = false;

	return 0;
}

