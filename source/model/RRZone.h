/*
 * PowerZone.h
 *
 *  Created on: 14 mrt. 2019
 *      Author: v.golle
 */

#ifndef SOURCE_MODEL_RRZONE_H_
#define SOURCE_MODEL_RRZONE_H_


#include <stdint.h>
#include "g_structs.h"
#include "BinnedData.h"


#define RR_ZONES_NB        6


class RRZone : public BinnedData {
public:
	RRZone();

	void addRRData(sHrmInfo &hrm_info, uint32_t timestamp);

	virtual uint32_t getTimeMax(void);
	virtual uint32_t getTimeTotal(void);

	virtual uint32_t getTimeZX(uint16_t i);

	uint32_t getValZX(uint16_t i);

	virtual uint32_t getNbBins(void);
	virtual uint32_t getCurBin(void);

private:
	uint32_t m_tot_time;
	uint32_t m_last_bin;
	uint32_t m_last_timestamp;
	float m_rr_bins[RR_ZONES_NB];
	float m_tm_bins[RR_ZONES_NB];

};


#endif /* SOURCE_MODEL_POWERZONE_H_ */
