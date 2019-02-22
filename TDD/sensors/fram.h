/*
 * fram.h
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#ifndef SOURCE_SENSORS_FRAM_H_
#define SOURCE_SENSORS_FRAM_H_

#include <stdint.h>
#include <stdbool.h>


#ifdef	__cplusplus
extern "C" {
#endif

void fram_init_sensor(void);

bool fram_read_block(uint16_t block_addr, uint8_t *readout, uint16_t length);

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length);

#ifdef	__cplusplus
}
#endif

#endif /* SOURCE_SENSORS_FRAM_H_ */
