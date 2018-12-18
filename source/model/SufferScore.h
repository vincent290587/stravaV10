/*
 * SufferScore.h
 *
 *  Created on: 18 déc. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_SUFFERSCORE_H_
#define SOURCE_MODEL_SUFFERSCORE_H_

#define HRM_Z1_LIM			80
#define HRM_Z2_LIM			120
#define HRM_Z3_LIM			144
#define HRM_Z4_LIM			165
#define HRM_Z5_LIM			176


#define HRM_Z1_PTS_PER_HOUR			(16./3600.)
#define HRM_Z2_PTS_PER_HOUR			(33./3600.)
#define HRM_Z3_PTS_PER_HOUR			(72./3600.)
#define HRM_Z4_PTS_PER_HOUR			(85./3600.)
#define HRM_Z5_PTS_PER_HOUR			(95./3600.)

#define HRM_ZONES_NB        5

#include <stdint.h>

class SufferScore {
public:
	SufferScore();

	void addHrmData(int hrm_meas, uint32_t timestamp);

	float getScore() const {
		return m_score;
	}

private:
	float m_score;

	uint32_t m_last_timestamp;
	float m_hrm_bins[HRM_ZONES_NB];

	void updateScore(void);
};


#endif /* SOURCE_MODEL_SUFFERSCORE_H_ */
