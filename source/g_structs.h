/*
 * g_structs.h
 *
 *  Created on: 21 déc. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_G_STRUCTS_H_
#define SOURCE_G_STRUCTS_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef PACKED
#define PACKED __attribute__((packed))
#endif

typedef struct PACKED {
	char*  str;
	size_t length;
} sCharArray;

typedef struct PACKED {
	bool isUpdated;
	uint8_t direction;
	uint32_t distance;
} sKomootNavigation;

typedef struct PACKED {
	uint16_t cumul_crank_rev;
	uint16_t last_crank_evt;       // Unit is in seconds with a resolution of 1/1024.
	uint16_t first_crank_angle;    // Unit is in degrees with a resolution of 1, starts at midnight for right pedal
	union {
		int16_t inst_force_mag_array[64];  // The unit is in newtons with a resolution of 1
		int16_t inst_torque_mag_array[64]; // Unit is in newton/meter with a resolution of 1/32
	};
	uint16_t array_size;
	int16_t inst_power;
} sPowerVector;

typedef struct PACKED {
	uint8_t bpm;
	uint16_t rr;
	uint32_t timestamp;
} sHrmInfo;

typedef struct PACKED {
	uint32_t cadence;
	uint32_t speed;
} sBscInfo;

typedef struct PACKED {
	int32_t lat;
	int32_t lon;
	int32_t ele;
	int16_t speed;
	uint16_t heading;
	uint32_t secj;
	uint32_t date;
	uint32_t utc_timestamp;
} sLnsInfo;

typedef struct PACKED {
	uint16_t power;
	uint16_t el_time;
} sFecInfo;

typedef struct PACKED {
	int16_t calib[3];
	uint16_t is_present;
} sMagCal;

typedef union PACKED {
	struct PACKED {
		uint16_t hrm_devid;
		uint16_t bsc_devid;
		uint16_t gla_devid;
		uint16_t fec_devid;
		uint16_t FTP;
		uint16_t weight;
		uint16_t version;
		sMagCal mag_cal;
		uint16_t crc;
	};
	uint8_t flat_user_params;
} sUserParameters;

extern sHrmInfo hrm_info;
extern sBscInfo bsc_info;
extern sFecInfo fec_info;
extern sKomootNavigation m_komoot_nav;

#endif /* SOURCE_G_STRUCTS_H_ */
