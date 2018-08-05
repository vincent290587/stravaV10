/*
 * nor.h
 *
 *  Created on: 25 juil. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_SD_NOR_H_
#define SOURCE_SD_NOR_H_

#ifdef __cplusplus
extern "C" {
#endif

void nor_init(void);

void nor_read_error(void);

void nor_save_error(uint32_t id, uint32_t pc, uint32_t info);

void nor_test(void);

#ifdef __cplusplus
}
#endif

#endif /* SOURCE_SD_NOR_H_ */
