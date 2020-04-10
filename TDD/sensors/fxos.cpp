/*
 * fxos.cpp
 *
 *  Created on: 18 mrt. 2019
 *      Author: v.golle
 */


#include "fxos.h"
#include "Model.h"
#include "parameters.h"
#include "RingBuffer.h"
#include "UserSettings.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MAX_ACCEL_AVG_COUNT 75U

/* multiplicative conversion constants */
#define DegToRad 0.017453292f
#define RadToDeg 57.295779f

/*******************************************************************************
 * Variables
 ******************************************************************************/

int16_t g_Ax_buff[MAX_ACCEL_AVG_COUNT] = {0};
int16_t g_Ay_buff[MAX_ACCEL_AVG_COUNT] = {0};
int16_t g_Az_buff[MAX_ACCEL_AVG_COUNT] = {0};

int16_t g_Ax_Raw = 0;
int16_t g_Ay_Raw = 0;
int16_t g_Az_Raw = 0;


float g_Yaw = 0;
float g_Yaw_LP = 0;
float g_Pitch = 0;
float g_Roll = 0;

static tHistoValue m_pi_buffer[PITCH_BUFFER_SIZE];
RingBuffer<tHistoValue> m_pitch_buffer(PITCH_BUFFER_SIZE, m_pi_buffer);

static float m_yaw = 0.0f;
static float m_pitch = 0.0f;

void fxos_readChip(void) {

}

void fxos_calibration_start(void) {

}

bool fxos_get_yaw(float &yaw_rad) {
	yaw_rad = m_yaw;
	return true;
}

void fxos_set_yaw(float yaw_rad) {
	m_yaw = yaw_rad;
}

bool fxos_init(void) {

	return true;
}

void fxos_tasks()
{
	uint16_t i = 0;

	float g_Ax = 0.f;
	float g_Ay = 0.f;
	float g_Az = 0.f;

	for (i = 0; i < MAX_ACCEL_AVG_COUNT; i++)
	{
		g_Ax += (float)g_Ax_buff[i];
		g_Ay += (float)g_Ay_buff[i];
		g_Az += (float)g_Az_buff[i];
	}

	g_Ax /= MAX_ACCEL_AVG_COUNT;
	g_Ay /= MAX_ACCEL_AVG_COUNT;
	g_Az /= MAX_ACCEL_AVG_COUNT;

	if (!isnormal(g_Az)) {
		return;
	}

	/* Calculate pitch angle g_Pitch and sin, cos*/
	g_Pitch  = atan2f( g_Ax , g_Az);

	m_pitch = g_Pitch;

	int16_t integ_pitch = (int16_t)((g_Pitch + 1.57f) * 100.);
	uint16_t u_integ_pitch = (uint16_t)integ_pitch;

	if (m_pitch_buffer.isFull()) {
		m_pitch_buffer.popLast();
	}
	m_pitch_buffer.add(&u_integ_pitch);

	LOG_DEBUG("FXOS Updated");

	static int is_init = 0;
	if (!is_init && u_settings.isConfigValid()) {

		sMagCal &mag_cal = u_settings.getMagCal();
		// check if we have a previous calibration
		if (mag_cal.is_present) {

			LOG_INFO("Magnetometer calibration found");

			//vue.addNotif("Event", "Magnetometer calibration found", 4, eNotificationTypeComplete);
		}

		is_init = 1;

	}


}


bool fxos_get_pitch(float &pitch_rad) {
	pitch_rad = m_pitch;
	return true;
}

void fxos_set_xyz(float g_Ax_Raw, float g_Ay_Raw, float g_Az_Raw) {

	/* Oversample accelerometer */
	static uint16_t index = 0;

	g_Ax_buff[index] = g_Ax_Raw;
	g_Ay_buff[index] = g_Ay_Raw;
	g_Az_buff[index] = g_Az_Raw;

	if (++index >= MAX_ACCEL_AVG_COUNT) {
		index = 0;
	}

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
