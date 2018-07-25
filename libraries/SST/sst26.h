/*
 * SST26VF064B Flash Memory Driver Header File
 *
 * This header file provides prototypes and definitions for controlling the
 * external flash memory module.
 *
 * License:
 *   All rights reserved to Kien.
 */
#ifndef _SST26VF064B_H_
#define _SST26VF064B_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * Transfer results
 */
typedef enum {
    SST26_OP_ERROR = 0,
    SST26_OP_DONE,
    SST26_OP_INCOMPLETE,
    SST26_OP_SPI_BUSY
} sst26_op_t;

/**
 * Requests the JEDEC-ID and compares it with the expected value.
 *
 * @returns true if the ID matches the expected one, false otherwise.
 */
bool sst26_validate_id();

/**
 * Enables access to data in flash chip.
 *
 * @returns operation result. SST26_OP_DONE expected if successful.
 */
sst26_op_t sst26_global_block_unlock();

/**
 * Sets the Write-Enable-Latch. This function shall be called prior to any
 * write operation.
 *
 * @returns operation result. SST26_OP_DONE expected if successful.
 */
sst26_op_t sst26_write_enable();

/**
 * Clears the Write-Enable-Latch. This register is automatically cleared after
 * most write operations, which means that this function is typically not used.
 *
 * @returns operation result. SST26_OP_DONE expected if successful.
 */
sst26_op_t sst26_write_disable();

/**
 * Erases the sector corresponding to the given address.
 *
 * @param sector_address address which belongs to the block to be erased.
 *
 * @returns operation result. SST26_OP_DONE expected if successful.
 */
sst26_op_t sst26_sector_erase(int32_t sector_address);

/**
 * Erases the block corresponding to the given address.
 *
 * @param block_address address which belongs to the block to be erased.
 *
 * @returns operation result. SST26_OP_DONE expected if successful.
 */
sst26_op_t sst26_block_erase(int32_t block_address);

/**
 * Erases the entire memory of the chip.
 *
 * @returns operation result. SST26_OP_DONE expected if successful.
 */
sst26_op_t sst26_chip_erase();

/**
 * Writes the length bytes of data in memory.
 * NOTE: Don't write more than 256 bytes (1 page) at a time.
 *
 * @param address initial address.
 * @param data array containing the data to be written.
 * @param length count of bytes to be written.
 *
 * @returns operation result. SST26_OP_DONE expected if successful.
 */
sst26_op_t sst26_write(int address, const uint32_t *data, size_t length);

/**
 * Puts in the data buffer the length bytes read from memory.
 *
 * @param address initial address.
 * @param data empty buffer for placing the data.
 * @param length count of bytes to be read.
 *
 * @returns operation result. SST26_OP_DONE expected if successful.
 */
sst26_op_t sst26_read(int address, uint32_t *data, size_t length);

#endif /* _SST26VF064B_H_ */
