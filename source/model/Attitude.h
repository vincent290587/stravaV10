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
#include "hardfault.h"
#include "Locator.h"
#include "parameters.h"

#ifdef ANT_STACK_SUPPORT_REQD
#include "fec.h"
#endif

typedef struct {
	SLoc  loc;
	SDate date;
	SSensors sensors;
	SEle  alti;
} SAttTime;

typedef struct {
	SLoc  loc;
	SDate date;
	float climb;
	float vit_asc;
	int8_t slope;
	float dist;
	int16_t pwr;
	uint16_t nbpts;
	uint8_t nbact;
	uint8_t pr;
	uint16_t nbsec_act;
	uint16_t next;
} SAtt;

#define SYSTEM_DESCR_POS_CRC      0xDB

typedef struct {
	HardFault_stack_t stck;
	uint8_t crc;
} sAppHardFaultDesc;

typedef struct {
	char _buffer[210];
	uint32_t pc;
	uint32_t id;
	uint8_t crc;
} sAppErrorDesc;

typedef struct {
 	SAtt att;
	uint8_t crc;
} sAppSavedData;

typedef struct {
	uint8_t special;
	uint32_t void_id;
	uint32_t task_id;
	sAppErrorDesc err_desc;
	sAppSavedData saved_data;
	sAppHardFaultDesc hf_desc;
} sAppErrorDescr;

#if defined(__cplusplus)
#include "AltiBaro.h"
class Attitude {
public:
	Attitude(AltiBaro &_baro);

	void reset(void);

	void addNewDate(SDate *date_);
	void addNewLocation(SLoc& loc_, SDate &date_, eLocationSource source_);

	void addNewFECPoint(sFecInfo& fec_);

	void addNewSIMPoint(SLoc& loc_, SDate& date_);

	void  computeFusion(void);
	float computePower(float speed_);

private:
	float dv;
	float m_climb;
	float m_speed_ms;
	float m_last_save_dist;
	float m_last_stored_ele;
	float m_cur_ele;

	bool m_is_init;
	bool m_is_acc_init;
	bool m_is_alt_init;

	AltiBaro &m_baro;

	SAttTime m_st_buffer[ATT_BUFFER_NB_ELEM];
	uint16_t m_st_buffer_nb_elem;

	float filterElevation(SLoc& loc_, eLocationSource source_);
	float computeElevation(SLoc& loc_, eLocationSource source_);
	void  computeDistance(SLoc& loc_, SDate &date_);
};
#endif

#endif /* SOURCE_MODEL_ATTITUDE_H_ */
