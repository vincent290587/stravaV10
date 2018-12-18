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

#define HRM_Z1_PTS_PER_HOUR			(25./3600.)
#define HRM_Z2_PTS_PER_HOUR			(60./3600.)
#define HRM_Z3_PTS_PER_HOUR			(115./3600.)
#define HRM_Z4_PTS_PER_HOUR			(250./3600.)
#define HRM_Z5_PTS_PER_HOUR			(300./3600.)

#define HRM_ZONES_NB        5

#include <stdint.h>

class SufferScore {
public:
	SufferScore();

	void addHrmData(int hrm_meas);

	float getScore() const {
		return m_score;
	}

private:
	float m_score;

	uint16_t m_hrm_bins[HRM_ZONES_NB];

	void updateScore(void);
};


#endif /* SOURCE_MODEL_SUFFERSCORE_H_ */
