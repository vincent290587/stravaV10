/*
 * g_structs.h
 *
 *  Created on: 21 déc. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_G_STRUCTS_H_
#define SOURCE_G_STRUCTS_H_

#include <stdint.h>
#include <stdbool.h>


typedef struct {
	bool isUpdated;
	uint8_t direction;
	uint32_t distance;
} sKomootNavigation;

typedef struct {
	uint8_t bpm;
	uint16_t rr;
} sHrmInfo;

typedef struct {
	uint32_t cadence;
	uint32_t speed;
} sBscInfo;

typedef struct {
	int32_t lat;
	int32_t lon;
	int32_t ele;
	int16_t speed;
	uint16_t heading;
	uint32_t secj;
	uint32_t date;
} sLnsInfo;

typedef struct {
	uint16_t power;
	uint16_t speed;
	uint8_t el_time;
} sFecInfo;

typedef union {
	struct {
		uint16_t hrm_devid;
		uint16_t bsc_devid;
		uint16_t gla_devid;
		uint16_t fec_devid;
		uint8_t version;
		uint8_t crc;
	};
	uint8_t* flat_user_params;
} sUserParameters;

extern sHrmInfo hrm_info;
extern sBscInfo bsc_info;
extern sFecInfo fec_info;
extern sKomootNavigation m_komoot_nav;

#endif /* SOURCE_G_STRUCTS_H_ */
