/*
 * fram.c
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */


#include "fram.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fram_init_sensor() {

	LOG_WARNING("FRAM init done");

}

bool fram_read_block(uint16_t block_addr, uint8_t *readout, uint16_t length) {

	return true;
}

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length) {

	return true;
}
