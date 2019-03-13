/*
 * MS5637.h
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_MS5637_H_
#define LIBRARIES_MS5637_H_


/* 7 bits i2c address of module */
#define MS5637_ADDR                    (0x76)

/* module commands */
#define CMD_RESET                      (0x1E)
#define CMD_PROM_READ(offs)            (0xA0+(offs<<1)) /* Offset 0-7 */
#define CMD_START_D1(oversample_level) (0x40 + 2*(int)oversample_level)
#define CMD_START_D2(oversample_level) (0x50 + 2*(int)oversample_level)
#define CMD_READ_ADC 0x00

#define MS5637_CRC_INDEX         0
#define MS5637_COEFFICIENT_COUNT 7



#ifdef __cplusplus
extern "C" {
#endif

void ms5637_init(void);

void ms5637_read_sensor(void);

void ms5637_refresh(void);

float ms5637_get_pressure(void);

float ms5637_get_temp(void);

void ms5637_clear_flags(void);

bool ms5637_is_data_ready(void);

bool is_ms5637_updated(void);


#ifdef __cplusplus
}
#endif

#endif /* LIBRARIES_MS5637_H_ */
