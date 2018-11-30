/*
 * sd_hal.h
 *
 *  Created on: 15 juin 2018
 *      Author: Vincent
 */

#ifndef SOURCE_SD_SD_HAL_H_
#define SOURCE_SD_SD_HAL_H_

#include <stdbool.h>

#ifdef	__cplusplus
extern "C" {
#endif

int fatfs_init(void);

bool is_fat_init(void);

int fatfs_uninit(void);

void format_memory(void);

void fmkfs_memory(void);

void test_memory(void);

#ifdef	__cplusplus
}
#endif

#endif /* SOURCE_SD_SD_HAL_H_ */
