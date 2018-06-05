/*
 * Attitude.h
 *
 *  Created on: 29 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_ATTITUDE_H_
#define SOURCE_MODEL_ATTITUDE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	float lat;
	float lon;
	float alt;
	float speed;
	float course;
} SLoc;

typedef struct {
	uint32_t secj;
	uint32_t date;
	uint32_t timestamp;
} SDate;

typedef struct {
	SLoc  loc;
	SDate date;
	float climb;
	float vit_asc;
	float dist;
	int16_t pwr;
	uint16_t nbpts;
	uint8_t nbact;
	uint8_t pr;
	uint16_t nbsec_act;
	uint16_t next;
} SAtt;

typedef struct {
	int16_t pwr;
} SFec;

class Attitude {
public:
	Attitude();

	void addNewDate(SDate &date_);
	void addNewLocation(SLoc& loc_, SDate &date_);
	void addNewFECPoint(SFec& fec_);

private:
	float m_last_save_dist;

	bool m_is_init;
};

#endif /* SOURCE_MODEL_ATTITUDE_H_ */
