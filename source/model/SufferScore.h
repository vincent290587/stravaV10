/*
 * SufferScore.h
 *
 *  Created on: 18 déc. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_SUFFERSCORE_H_
#define SOURCE_MODEL_SUFFERSCORE_H_

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
