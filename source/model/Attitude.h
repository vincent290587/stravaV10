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
#include "Locator.h"
#include "parameters.h"

#ifdef ANT_STACK_SUPPORT_REQD
#include "fec.h"
#endif

typedef struct {
	SLoc  loc;
	SDate date;
	int16_t pwr;
} SAttTime;

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


class Attitude {
public:
	Attitude();

	void addNewDate(SDate &date_);
	void addNewLocation(SLoc& loc_, SDate &date_, eLocationSource source_);

#ifdef ANT_STACK_SUPPORT_REQD
	void addNewFECPoint(sFecInfo& fec_);
#endif

private:
	float m_last_save_dist;
	float m_last_stored_ele;
	float m_cur_ele;
	float m_climb;
	float m_vit_asc;
	float m_power;

	bool m_is_init;
	bool m_is_alt_init;

	SAttTime m_st_buffer[ATT_BUFFER_NB_ELEM];
	uint16_t m_st_buffer_nb_elem;

	void computeElevation(void);
	void majPower(float speed_);
};

#endif /* SOURCE_MODEL_ATTITUDE_H_ */
