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


static s32_t my_spiffs_read(u32_t addr, u32_t size, u8_t *dst) {
	//my_spi_read(addr, size, dst);
	sst26_read(addr, (uint32_t*)dst, size);

	return SPIFFS_OK;
}

static s32_t my_spiffs_write(u32_t addr, u32_t size, u8_t *src) {
	//my_spi_write(addr, size, src);
	sst26_write(addr, (uint32_t*)src, size);

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
	cfg.phys_size = SST26_NSECTORS*SST26_SECTOR_SIZE; // use all spi flash
	cfg.phys_addr = 0; // start spiffs at start of spi flash
	cfg.phys_erase_block = SST26_SECTOR_SIZE; // according to datasheet
	cfg.log_block_size   = SST26_SECTOR_SIZE; // let us not complicate things
	cfg.log_page_size = LOG_PAGE_SIZE; // as we said

	cfg.hal_read_f = my_spiffs_read;
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
	printf("mount res: %i\n", res);
}


void nor_test(void) {

	char buf[12];

	_my_spiffs_mount();

	// Surely, I've mounted spiffs before entering here

	spiffs_file fd = SPIFFS_open(&fs, "my_file", SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
	if (SPIFFS_write(&fs, fd, (u8_t *)"Hello world", 12) < 0) printf("errno %i\n", SPIFFS_errno(&fs));
	SPIFFS_close(&fs, fd);

	fd = SPIFFS_open(&fs, "my_file", SPIFFS_RDWR, 0);
	if (SPIFFS_read(&fs, fd, (u8_t *)buf, 12) < 0) printf("errno %i\n", SPIFFS_errno(&fs));
	SPIFFS_close(&fs, fd);

	LOG_INFO("--> %s <--\n", buf);

}
