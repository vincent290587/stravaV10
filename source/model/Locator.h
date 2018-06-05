/*
 * Locator.h
 *
 *  Created on: 19 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_LOCATOR_H_
#define SOURCE_MODEL_LOCATOR_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX_SATELLITES     40
#define ACTIVE_VAL         5

typedef struct
{
	int active;
	int elevation;
	int azimuth;
	int snr;
} sSatellite;

extern sSatellite sats[MAX_SATELLITES];

typedef enum {
	eLocationSourceNone,
	eLocationSourceSimu,
	eLocationSourceNRF,
	eLocationSourceGPS,
} eLocationSource;

typedef struct {
	float lat;
	float lon;
	float alt;
	float speed;
	float course;
	uint32_t utc_time;
	uint32_t utc_timestamp;
	uint32_t date;
} sLocationData;

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

uint32_t locator_encode_char(char c);

#if defined(__cplusplus)
}

#include "Sensor.h"
#include "Attitude.h"

/**
 *
 */
class Locator {
public:
	Locator();

	void tasks();

	void displayGPS2(void);

	eLocationSource getDate(SDate& date_);
	eLocationSource getPosition(SLoc& loc_, SDate& date_);

	eLocationSource getUpdateSource();

	bool isUpdated();

	uint32_t getLastUpdateAge();

	Sensor<sLocationData> nrf_loc;
	Sensor<sLocationData> sim_loc;
	Sensor<sLocationData> gps_loc;

private:
	bool anyChanges;

	uint16_t m_nb_nrf_pos;
};

#endif /* _cplusplus */
#endif /* SOURCE_MODEL_LOCATOR_H_ */
