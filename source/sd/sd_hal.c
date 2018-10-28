/*
 * sd_hal.c
 *
 *  Created on: 6 juin 2018
 *      Author: Vincent
 */

#include <stdio.h>
#include <string.h>
#include "boards.h"
#include "nrf_qspi.h"
#include "segger_wrapper.h"

#include "ff.h"
#include "diskio_blkdev.h"
#include "nrf_block_dev_sdc.h"


static DSTATUS disk_state = STA_NOINIT;
static FATFS fs;

static bool m_is_fat_mounted = false;


#define QSPI_STD_CMD_WRSR   0x01
#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99

static void configure_memory()
{
	uint32_t err_code;
	nrf_qspi_cinstr_conf_t cinstr_cfg = {
			.opcode    = QSPI_STD_CMD_RSTEN,
			.length    = NRF_QSPI_CINSTR_LEN_1B,
			.io2_level = true,
			.io3_level = true,
			.wipwait   = true,
			.wren      = true
	};

	// Send reset enable
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
	APP_ERROR_CHECK(err_code);

	// Send reset command
	cinstr_cfg.opcode = QSPI_STD_CMD_RST;
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
	APP_ERROR_CHECK(err_code);

	delay_ms(3);

	// Get device ID
	cinstr_cfg.opcode = 0x9F;
	cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_4B;
	uint8_t recv[3] = {0};
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, recv);
	APP_ERROR_CHECK(err_code);
	LOG_INFO("JEDEC ID: 0x%02X 0x%02X 0x%02X", recv[0], recv[1], recv[2]);

	// write status register
    cinstr_cfg.opcode = 0x01;
    uint8_t temporary[2] = {0};
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_3B;
    err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, &temporary, NULL);
    APP_ERROR_CHECK(err_code);

	// Global unlock SST26VF
	cinstr_cfg.opcode = 0x98;
	cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
	err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
	APP_ERROR_CHECK(err_code);

    // Switch to 4-io-qspi mode
//    cinstr_cfg.opcode = 0x38;
//    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_1B;
//    err_code = nrfx_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
//    APP_ERROR_CHECK(err_code);
}

void format_memory() {
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

}

#define QSPI_TEST_DATA_SIZE 32
static uint8_t m_buffer_tx[QSPI_TEST_DATA_SIZE];
static uint8_t m_buffer_rx[QSPI_TEST_DATA_SIZE];


void test_memory(void)
{
    uint32_t i;
    uint32_t err_code;

    NRF_LOG_INFO("QSPI write and read example");

    srand(0);
    for (i = 0; i < QSPI_TEST_DATA_SIZE; ++i) {
      m_buffer_tx[i] = (uint8_t)rand();
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
	for (uint32_t retries = 3; retries && disk_state; --retries)
	{
		disk_state = disk_initialize(0);

		configure_memory();

		if (!disk_state) break;
	    delay_ms(30);
	}

	//test_memory();

	configure_memory();

	if (disk_state)
	{
		LOG_INFO("Disk initialization failed.");


		return -1;
	}

	LOG_INFO("Mounting volume...");
	ff_result = f_mount(&fs, "", 1);
	if (ff_result)
	{
		if (ff_result == FR_NO_FILESYSTEM)
		{
			LOG_ERROR("Mount failed. Filesystem not found. Please format device.");
			fatfs_mkfs();
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

