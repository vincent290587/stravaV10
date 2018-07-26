/*
 * nor.c

 *
 *  Created on: 25 juil. 2018
 *      Author: Vincent
 */
#include <stdbool.h>
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "parameters.h"
#include "boards.h"
#include "nordic_common.h"
#include "segger_wrapper.h"
#include "sst26.h"
#include "sst26_hal.h"
#include "spiffs.h"


#define SPI_FLASH_SEC_SIZE     SST26_SECTOR_SIZE
#define LOG_PAGE_SIZE          256


static u8_t spiffs_work_buf[LOG_PAGE_SIZE*2];
static u8_t spiffs_fds[32*4];
static u8_t spiffs_cache_buf[(LOG_PAGE_SIZE+32)*4];
static spiffs fs;
static bool m_is_spiffs_started = false;


static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst) {

	while (SST26_OP_DONE != sst26_read(addr, dst, size));

	return SPIFFS_OK;
}

static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src) {

	sst26_write(addr, src, size);

	return SPIFFS_OK;
}

static s32_t my_spiffs_erase(u32_t addr, u32_t size) {

	if ((size & (SPI_FLASH_SEC_SIZE - 1)) != 0 ||
			(addr & (SPI_FLASH_SEC_SIZE - 1)) != 0) {
		LOG_DEBUG("_spif_erase called with addr=%x, size=%d\r\n", addr, size);
		return SPIFFS_ERR_INTERNAL;
	}
	const uint32_t sector = addr / SPI_FLASH_SEC_SIZE;
	const uint32_t sectorCount = size / SPI_FLASH_SEC_SIZE;
	for (uint32_t i = 0; i < sectorCount; ++i) {

		if (SST26_OP_DONE != sst26_sector_erase(sector + i)) {
			LOG_DEBUG("_spif_erase addr=%x size=%d i=%d\r\n", addr, size, i);
			return SPIFFS_ERR_INTERNAL;
		}
	}
	return SPIFFS_OK;
}

static void _my_spiffs_mount() {

	// initiate communication with hardware
	sst26_validate_id();

	spiffs_config cfg;

	cfg.hal_read_f  = my_spiffs_read;
	cfg.hal_write_f = my_spiffs_write;
	cfg.hal_erase_f = my_spiffs_erase;

	int res = SPIFFS_mount(&fs,
				&cfg,
				spiffs_work_buf,
				spiffs_fds,
				sizeof(spiffs_fds),
				spiffs_cache_buf,
				sizeof(spiffs_cache_buf),
				0);

	if (res != 0) {

		LOG_INFO("SPIFFS Mount fail, formatting");

		SPIFFS_unmount(&fs);

		res = sst26_chip_erase();

		sst26_global_block_unlock();

		LOG_INFO("Chip erase res: %i", res);

		res = SPIFFS_format(&fs);

		LOG_INFO("SPIFFS format res: %i", res);

		res = SPIFFS_mount(&fs,
					&cfg,
					spiffs_work_buf,
					spiffs_fds,
					sizeof(spiffs_fds),
					spiffs_cache_buf,
					sizeof(spiffs_cache_buf),
					0);
	}

	LOG_INFO("SPIFFS Mount res: %i\n", res);

	if (res == 0) m_is_spiffs_started = true;
}


void nor_init(void) {

	_my_spiffs_mount();

}

void nor_test(void) {

	if (!m_is_spiffs_started) {
		_my_spiffs_mount();
		return;
	}

	char buf[12] = {0};

	// Surely, I've mounted spiffs before entering here

	spiffs_file fd = SPIFFS_open(&fs, "my_file", SPIFFS_APPEND | SPIFFS_CREAT | SPIFFS_RDWR, 0);
	if (SPIFFS_write(&fs, fd, (u8_t *)"Hello world", 12) < 0) {
		LOG_INFO("errno %i\n", SPIFFS_errno(&fs));
	} else {
		LOG_INFO("File created %i\n", SPIFFS_errno(&fs));
	}
	SPIFFS_close(&fs, fd);

	fd = SPIFFS_open(&fs, "my_file", SPIFFS_RDWR, 0);
	if (SPIFFS_read(&fs, fd, (u8_t *)buf, 12) < 0) LOG_INFO("errno %i\n", SPIFFS_errno(&fs));
	SPIFFS_close(&fs, fd);

	LOG_INFO("--> %s <--\n", buf);

}
