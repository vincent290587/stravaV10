/*
 * fxos.cpp
 *
 *  Created on: 18 mrt. 2019
 *      Author: v.golle
 */


#include "fxos.h"
#include "parameters.h"
#include "RingBuffer.h"


static tHistoValue m_pi_buffer[PITCH_BUFFER_SIZE];
RingBuffer<tHistoValue> m_pitch_buffer(PITCH_BUFFER_SIZE, m_pi_buffer);

static float m_yaw = 0.0f;
static float m_pitch = 0.0f;

void fxos_calibration_start(void) {

}

bool fxos_get_yaw(float &yaw_rad) {
	yaw_rad = m_yaw;
	return true;
}

void fxos_set_yaw(float yaw_rad) {
	m_yaw = yaw_rad;
}

bool fxos_get_pitch(float &pitch_rad) {
	pitch_rad = m_pitch;
}

void fxos_set_pitch(float pitch_rad) {

	int16_t integ_pitch = (int16_t)((pitch_rad + 1.57) * 100.);
	uint16_t u_integ_pitch = (uint16_t)integ_pitch;

	if (m_pitch_buffer.isFull()) {
		m_pitch_buffer.popLast();
	}
	m_pitch_buffer.add(&u_integ_pitch);

	m_pitch = pitch_rad;
}

tHistoValue fxos_histo_read(uint16_t ind_) {

	tHistoValue *p_ret_val = m_pitch_buffer.get(ind_);

	ASSERT(p_ret_val);

	tHistoValue ret_val = p_ret_val[0];

	return ret_val;
}

uint16_t fxos_histo_size(void) {

	return m_pitch_buffer.size();
}
