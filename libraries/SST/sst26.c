/*
 * SST26VF064B Flash Memory Driver Source File
 *
 * This header file contains the source code for controlling the external flash
 * memory module.
 *
 * License:
 *   All rights reserved to Kien.
 */

#include <stdbool.h>
#include "app_util_platform.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "parameters.h"
#include "boards.h"
#include "nordic_common.h"
#include "sst26_hal.h"
#include "sst26.h"
#include "segger_wrapper.h"

/*****************************************************************
 *                       LOCAL FUNCTIONS
 ****************************************************************/


static bool _send_cmd(uint8_t cmd) {

	LOG_DEBUG("SST cmd %02X", cmd);

    return spi_send_sync(&cmd, 1);
}

static bool _send_address(int32_t addr) {
    uint8_t addr_bytes[3];

    addr_bytes[0] = (uint8_t) ((addr >> 16) & 0xFF);
    addr_bytes[1] = (uint8_t) ((addr >> 8) & 0xFF);
    addr_bytes[2] = (uint8_t) ((addr) & 0xFF);

    return spi_send_sync(addr_bytes, 3);
}

static uint8_t _read_status(void) {
    uint8_t cmd = SST26_RDSR;
    uint8_t res[2];

    _chip_enable_low();
    while (!spi_transceive_sync(&cmd, 1, res, sizeof (res)));
    _chip_enable_high();

    return (res[1]);
}

static bool _is_busy(void) {
	LOG_DEBUG("Busy polling");
    return ((bool) (_read_status() & SST26_SR_WIP));
}

static sst26_op_t _write_inside_page(int32_t address, const uint8_t *data, size_t length) {
	LOG_DEBUG("Writing %i bytes at 0x%06X", length, address);

	if (length==2) LOG_DEBUG("WBytes = 0x%02X 0x%02X", data[0], data[1]);

    while (_is_busy());

    sst26_write_enable();

    _chip_enable_low();
    if (!_send_cmd(SST26_PP)) return SST26_OP_ERROR;
    if (!_send_address(address)) return SST26_OP_ERROR;
    if (!spi_send_sync(data, length)) return SST26_OP_ERROR;
    _chip_enable_high();

    return SST26_OP_DONE;
}

/*****************************************************************
 *                       GLOBAL FUNCTIONS
 ****************************************************************/

sst26_op_t sst26_write_enable() {
    _chip_enable_low();
    if (!_send_cmd(SST26_WREN)) return SST26_OP_ERROR;
    _chip_enable_high();

    return SST26_OP_DONE;
}

sst26_op_t sst26_write_disable() {
    _chip_enable_low();
    if (!_send_cmd(SST26_WRDI)) return SST26_OP_ERROR;
    _chip_enable_high();

    return SST26_OP_DONE;
}

bool sst26_validate_id() {
    uint8_t cmd = SST26_RDID;
    uint8_t res[4] = {0};

    sst_init_spi();

    nrf_delay_ms(10);

    _chip_enable_low();
    while (!spi_transceive_sync(&cmd, sizeof(cmd), res, sizeof (res)));
    _chip_enable_high();

    if (res[1] == SST26_MANUFACTURER &&
            res[2] == SST26_MEMORY_TYPE &&
            res[3] == SST26_CAPACITY) {
        LOG_INFO("Valid JEDEC-ID 0x%08X", (uint32_t) (res[3] + (res[2] << 8) + (res[1] << 16)));
    } else {
        LOG_ERROR("Wrong JEDEC-ID 0x%08X", (uint32_t) (res[3] + (res[2] << 8) + (res[1] << 16)));
        return false;
    }

    sst26_global_block_unlock();

    uint8_t stat = _read_status();
    LOG_INFO("Status 0x%02X", stat);

    return true;
}

sst26_op_t sst26_sector_erase(int32_t sector_address) {
    while (_is_busy());

    sst26_write_enable();

    _chip_enable_low();
    if (!_send_cmd(SST26_SE)) return SST26_OP_ERROR;
    if (!_send_address(sector_address)) return SST26_OP_ERROR;
    _chip_enable_high();

    return SST26_OP_DONE;
}

sst26_op_t sst26_block_erase(int32_t block_address) {
    while (_is_busy());

    sst26_write_enable();

    _chip_enable_low();
    if (!_send_cmd(SST26_BE)) return SST26_OP_ERROR;
    if (!_send_address(block_address)) return SST26_OP_ERROR;
    _chip_enable_high();

    return SST26_OP_DONE;
}

sst26_op_t sst26_chip_erase() {
    while (_is_busy());

    sst26_write_enable();

    _chip_enable_low();
    if (!_send_cmd(SST26_CE)) return SST26_OP_ERROR;
    _chip_enable_high();

    while (_is_busy());

    return SST26_OP_DONE;
}

sst26_op_t sst26_global_block_unlock() {
    sst26_write_enable();

    _chip_enable_low();
    if (!_send_cmd(SST26_ULBPR)) return SST26_OP_ERROR;
    _chip_enable_high();

    return SST26_OP_DONE;
}

static sst26_op_t _sst26_write_less_page(int address, const uint8_t *data, size_t length) {
    // Page program works for only one page at once and if a page program tries
    // to write data on 2+ different pages (jumps from one page to another),
    // only the first page will get programmed.
    int32_t next_page_addr = ((address & 0xFFFF00) + SST26_PAGE_SIZE);
    if ((address + length) > next_page_addr) {
        uint8_t length_p1 = next_page_addr - address;
        // Fill first page
        while (SST26_OP_DONE != _write_inside_page(address, data, length_p1));
        // Write remaining data in next page (only 2 pages bc length < 256)
        while (SST26_OP_DONE != _write_inside_page(next_page_addr, data + length_p1, length - length_p1));
    } else {
    	while (SST26_OP_DONE != _write_inside_page(address, data, length));
    }

    return SST26_OP_DONE;
}

sst26_op_t sst26_write(int address, const uint8_t *data, size_t length) {

	uint16_t cur_ind = 0;
    while (length > SST26_PAGE_SIZE) {

    	_sst26_write_less_page(address+cur_ind, data+cur_ind, SST26_PAGE_SIZE);

    	cur_ind += SST26_PAGE_SIZE;
    	length  -= SST26_PAGE_SIZE;

    }

    // write remaining bytes
    if (length){
    	_sst26_write_less_page(address+cur_ind, data+cur_ind, length);
    }

    return SST26_OP_DONE;
}

sst26_op_t sst26_read(int address, uint8_t *data, size_t length) {
    LOG_DEBUG("Reading %i bytes from 0x%06X", length, address);
    while (_is_busy());

    _chip_enable_low();
    if (!_send_cmd(SST26_READ)) return SST26_OP_ERROR;
    if (!_send_address(address)) return SST26_OP_ERROR;
    if (!spi_receive_sync(data, length)) return SST26_OP_ERROR;
    _chip_enable_high();

    if (length==2) LOG_DEBUG("RBytes = 0x%02X 0x%02X", data[0], data[1]);

    return SST26_OP_DONE;
}
