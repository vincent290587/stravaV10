/*
 * GPS.h
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_SENSORS_GPSMGMT_H_
#define SOURCE_SENSORS_GPSMGMT_H_

#include "stdint.h"
#include "Locator.h"

typedef enum {
	eGPSMgmtStateInit,
	eGPSMgmtStateRunSlow,
	eGPSMgmtStateRunFast,
} eGPSMgmtState;

typedef enum {
	eGPSMgmtEPOIdle,
	eGPSMgmtEPOStart,
	eGPSMgmtEPORunning,
	eGPSMgmtEPOWaitForEvent,
	eGPSMgmtEPOEnd,
} eGPSMgmtEPOState;

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

uint32_t gps_encode_char(char c);

#if defined(__cplusplus)
}

class GPS_MGMT {
public:
	GPS_MGMT();

	void init(void);
	bool isFix(void);
	bool isStandby(void);
	bool isEPOUpdating(void);

	void runWDT();

	void standby(void);
	void awake(void);
	void standby(bool is_standby);
	void reset(void);

	void setFixInterval(uint16_t interval);
	void startHostAidingEPO(sLocationData& loc_data, uint32_t age_);

	void startEpoUpdate(void);
	void tasks(void);

	eGPSMgmtState getPowerState() const {
		return m_power_state;
	}

private:
	eGPSMgmtState m_power_state;

	bool m_is_stdby;

	uint16_t m_epo_packet_ind;
	uint16_t m_epo_packet_nb;
};

#endif /* _cplusplus */
#endif /* SOURCE_SENSORS_GPSMGMT_H_ */
