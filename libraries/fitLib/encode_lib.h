/*
 * encode_lib.h
 *
 *  Created on: 20 mai 2020
 *      Author: vgol
 */

#ifndef SRC_ENCODE_LIB_H_
#define SRC_ENCODE_LIB_H_

#include <stdint.h>

typedef enum {
	eEncodingCommandNone,
	eEncodingCommandStart,
	eEncodingCommandStop,
} eEncodingCommand;

typedef enum {
	eUserDataPosStart,
	eUserDataPosEnd,
} eUserDataPos;


typedef struct {
	eEncodingCommand cmd;
	uint32_t timestamp;
	int32_t lat;
	int32_t lon;
	int32_t alt_cm;
	uint8_t cad;
	uint8_t hrm;
	int16_t grade_pc;
	int8_t  temp_c;
} sEncodingData;

#ifdef __cplusplus
extern "C" {
#endif

void UserData_GoTo(eUserDataPos pos);
long UserData_Ftell(void);
void WriteDataBare(const void *data, uint16_t data_size);

int encode_process(sEncodingData * const p_data);

#ifdef __cplusplus
}
#endif

#endif /* SRC_ENCODE_LIB_H_ */
