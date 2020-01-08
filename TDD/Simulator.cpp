/*
 * Simulator.cpp
 *
 *  Created on: 18 sept. 2018
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ff.h"
#include "millis.h"
#include "bme280.h"
#include "tdd_logger.h"
#include "uart_tdd.h"
#include "Model_tdd.h"
#include "Simulator.h"
#include "segger_wrapper.h"
#include "assert_wrapper.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE (128U)

/*******************************************************************************
 * Variables
 ******************************************************************************/

static TCHAR g_bufferRead[BUFFER_SIZE];  /* Read buffer */
static TCHAR g_bufferWrite[BUFFER_SIZE]; /* Write buffer */

static FIL* g_fileObject;   /* File object */

#ifdef LS027_GUI
#define NEW_POINT_PERIOD_MS       400
#else
#define NEW_POINT_PERIOD_MS       1000
#endif

static uint32_t nb_gps_loc = 0;

static float cur_speed = 20.0f;
static float alt_sim = 100.0f;

static void simulator_modes(void) {

	enum eSimulationStep {
		eSimulationStepCRS1,
		eSimulationStepMenu1,
		eSimulationStepFEC,
		eSimulationStepMenu2,
		eSimulationStepCRS2,
		eSimulationStepCRS3,
		eSimulationStepCRS4,
		eSimulationStepEnd,
	};

	static eSimulationStep m_step = eSimulationStepCRS1;
	static uint32_t m_next_event_ms = 15000;

	switch (m_step) {
	case eSimulationStepCRS1:
		if (millis() > m_next_event_ms) {
		    LOG_INFO("Going to FEC mode");

			vue.tasks(eButtonsEventCenter);
			vue.tasks(eButtonsEventRight);

			m_next_event_ms += 30000;

			m_step = eSimulationStepMenu1;
		}
		break;
	case eSimulationStepMenu1:
	{
		vue.tasks(eButtonsEventCenter);
		w_task_yield();

		m_step = eSimulationStepFEC;
	} break;
	case eSimulationStepFEC:
	if (millis() > m_next_event_ms) {
		LOG_INFO("Going to PRC mode");
		vue.tasks(eButtonsEventCenter);
		w_task_yield();
		vue.tasks(eButtonsEventRight);
		vue.tasks(eButtonsEventRight);
		vue.tasks(eButtonsEventRight);
		w_task_yield();

		m_step = eSimulationStepMenu2;
	} break;
	case eSimulationStepMenu2:
	{
		vue.tasks(eButtonsEventCenter);
		w_task_yield();
		vue.tasks(eButtonsEventRight);
		vue.tasks(eButtonsEventRight);
		w_task_yield();

		m_step = eSimulationStepCRS2;
	} break;
	case eSimulationStepCRS2:
	{
		vue.tasks(eButtonsEventCenter);
		w_task_yield();
		vue.tasks(eButtonsEventLeft);
		vue.tasks(eButtonsEventLeft);
		vue.tasks(eButtonsEventLeft);
		vue.tasks(eButtonsEventLeft);
		vue.tasks(eButtonsEventLeft);
		vue.tasks(eButtonsEventLeft);
		w_task_yield();

		m_next_event_ms = 250000;

		m_step = eSimulationStepCRS3;
	} break;
	case eSimulationStepCRS3:
	if (millis() > m_next_event_ms) {
		LOG_INFO("Going to CRS mode");

		vue.tasks(eButtonsEventCenter);

		m_step = eSimulationStepCRS4;
	} break;
	case eSimulationStepCRS4:
	{
		vue.tasks(eButtonsEventRight);
		vue.tasks(eButtonsEventRight);
		vue.tasks(eButtonsEventCenter);

		m_step = eSimulationStepEnd;
	} break;

	case eSimulationStepEnd:
		// no break
	default:
		break;
	}

}

extern float m_press_sim;

void simulator_simulate_altitude(float alti) {

	const float sea_level_pressure = 1015.0f;

	// res = 44330.0f * (1.0f - powf(atmospheric / sea_level_pressure, 0.1903f));

	m_press_sim = sea_level_pressure * powf(1.0f - alti / 44330.0f, 1.0f / 0.1903f);

	// sets the updated flag
	bme280_read_sensor();
}

void simulator_init(void) {

	tdd_logger_init("simu.txt");

	tdd_logger_log_name(TDD_LOGGING_TIME, "time");
	tdd_logger_log_name(TDD_LOGGING_P2D, "p2d");
	tdd_logger_log_name(TDD_LOGGING_P3D, "p3d");
	tdd_logger_log_name(TDD_LOGGING_SEG_DIST, "dist");
	tdd_logger_log_name(TDD_LOGGING_NB_SEG_ACT, "nb_act");
	tdd_logger_log_name(TDD_LOGGING_HP, "hp");
	tdd_logger_log_name(TDD_LOGGING_ALPHA, "a");
	tdd_logger_log_name(TDD_LOGGING_ALPHA0, "a_0");
	tdd_logger_log_name(TDD_LOGGING_SIM_SLOPE, "sim_slope");
	tdd_logger_log_name(TDD_LOGGING_EST_SLOPE, "est_slope");
	tdd_logger_log_name(TDD_LOGGING_ALT_SIM, "alti_sim");
	tdd_logger_log_name(TDD_LOGGING_ALT_EST, "alti_est");
	tdd_logger_log_name(TDD_LOGGING_TOT_CLIMB, "climb");
	tdd_logger_log_name(TDD_LOGGING_CUR_POWER, "power");
	tdd_logger_log_name(TDD_LOGGING_CUR_SPEED, "speed");

	g_fileObject = fopen("GPX_simu.csv", "r");

	m_app_error.hf_desc.crc = SYSTEM_DESCR_POS_CRC;
	m_app_error.hf_desc.stck.pc = 0x567896;

	m_app_error.special = SYSTEM_DESCR_POS_CRC;

	m_app_error.err_desc.crc = SYSTEM_DESCR_POS_CRC;
	snprintf(m_app_error.err_desc._buffer,
			sizeof(m_app_error.err_desc._buffer),
			"Error 0x123456 in file /mnt/e/Nordic/Projects/Perso/stravaV10/TDD/Simulator.cpp:48");

	m_app_error.saved_data.att.climb = 562.;
	m_app_error.saved_data.att.dist = 17700;
	m_app_error.saved_data.att.nbsec_act = 2780;
	m_app_error.saved_data.att.pr = 3;
	m_app_error.saved_data.att.date.date = 211218;

	m_app_error.saved_data.crc = calculate_crc((uint8_t*)&m_app_error.saved_data.att, sizeof(m_app_error.saved_data.att));
}

static void _fec_sim(void) {

	static uint32_t last_point_ms = 0;
	if (millis() - last_point_ms < 250) return;
	last_point_ms = millis();

	// HRM simulation
	hrm_info.bpm = 120 + (rand() % 65);

	// FEC simulation
	fec_info.power = rand() % 500;
	fec_info.speed = 20.;
	fec_info.el_time++;
	w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_FEC_INFO);
	w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_FEC_POWER);
}

static void _sensors_sim(void) {

	static uint32_t last_point_ms = 0;
	if (millis() - last_point_ms < 100) return;

	static float cur_a = toRadians(5.0f);
	static const float cur_a0 = toRadians(3.4f);
	static uint32_t sim_nb = 0;

	alt_sim += tanf(cur_a) * cur_speed * (millis() - last_point_ms) / 3600.f; // over 1 second

	if (++sim_nb > 2000) {

		cur_a = -cur_a;

		sim_nb = 0;
	}

	fxos_set_yaw(cur_a + cur_a0);
	simulator_simulate_altitude(alt_sim);

	last_point_ms = millis();

	tdd_logger_log_float(TDD_LOGGING_SIM_SLOPE, 100 * tanf(cur_a));
	tdd_logger_log_float(TDD_LOGGING_ALT_SIM, alt_sim);

	tdd_logger_flush();
}

static void _loc_sim(void) {

	if (!g_fileObject) {
		LOG_ERROR("No simulation file found");
		exit(-3);
	}

	static uint32_t last_point_ms = 0;
	if (millis() - last_point_ms < NEW_POINT_PERIOD_MS) return;
	last_point_ms = millis();

	simulator_modes();

	if (fgets(g_bufferRead, sizeof(g_bufferRead)-1, g_fileObject)) {

		float lon, lat, alt;
		static float rtime;
		float data[4];
		char *pch;
		uint16_t pos = 0;

		lat = 0; lon = 0; alt = 0;// on se met au bon endroit
		pch = strtok (g_bufferRead, " ");
		while (pch != NULL && pos < 4)
		{
			data[pos] = strtof(pch, 0);
			pch = strtok (NULL, " ");
			pos++;
		}

		lat   = data[0];
		lon   = data[1];
		alt   = data[2];

		LOG_DEBUG("Input alt.:  %f", alt);

		if (pos == 4) {
			// file contains the rtime
			rtime = data[3];
		} else {
			// rtime is missing: generate it
			rtime += 1.;
		}

#ifdef TDD_RANDOMIZE
		int rnd_add;
		rnd_add = (rand() % 20) - 10;
		lat += (float)rnd_add / 150000.;
		rnd_add = (rand() % 20) - 10;
		lon += (float)rnd_add / 150000.;
#endif

#ifdef LOC_SOURCE_GPS
		// build make NMEA sentence
		GPRMC gprmc_(lat, lon, 0., (int)rtime);
		int nmea_length = gprmc_.toString(g_bufferWrite, sizeof(g_bufferWrite));

		LOG_WARNING("Sentence: %s", g_bufferWrite);

		// send to uart_tdd
		for (int i=0; i < nmea_length; i++)
			uart_rx_handler(g_bufferWrite[i]);

		if (++nb_gps_loc == 1) {
			sLocationData loc_data;
			loc_data.alt = 9.5;
			loc_data.lat = lat;
			loc_data.lon = lon;
			loc_data.utc_time = 15 * 3600 + 5 * 60 + 39;
			loc_data.date = 11218;

			gps_mgmt.startHostAidingEPO(loc_data, 350);
		}
#else
		sLnsInfo lns_info;
		lns_info.lat = lat * 10000000.;
		lns_info.lon = lon * 10000000.;
		lns_info.ele = (alt_sim + 32.3f) * 100.;
		lns_info.secj = (int)rtime;
		lns_info.date = 11218;
		lns_info.speed = cur_speed * 10.;
		locator_dispatch_lns_update(&lns_info);
#endif

	} else {
		fclose(g_fileObject);

#ifdef TDD_RANDOMIZE
		static int nb_tests = 0;
		g_fileObject = fopen("GPX_simu.csv", "r");
		if (++nb_tests < 750) {
			LOG_WARNING("Starting next simulation");
			return;
		}
		LOG_WARNING("%u simulations run", nb_tests);
#endif
		LOG_WARNING("Reached end of simulation file");

		mes_segments.clear();
		mes_parcours._parcs.clear();
		mes_points.removeAll();

		// print memory state
		print_mem_state();

		assert(Point2D::getObjectCount() == 1);
		assert(Point::getObjectCount() == 2);

		exit(0);

	}
}

void simulator_tasks(void) {

	tdd_logger_log_int(TDD_LOGGING_TIME, millis());

	if (millis() < 500) {

		tdd_logger_start();
		return;
	}

	_fec_sim();
	_sensors_sim();
	_loc_sim();

	tdd_logger_log_int(TDD_LOGGING_P2D, Point2D::getObjectCount());
	tdd_logger_log_int(TDD_LOGGING_P3D, Point::getObjectCount());

}

GPRMC::GPRMC(float latitude, float longitude, float vitesse, int sec_jour) {

	c_latitude = latitude;
	c_longitude = longitude;
	_vitesse = vitesse;
	_date = sec_jour;

}

void GPRMC::coordtoString(char* buffer_, size_t max_size_, uint16_t prec, float value) {

	float val1, val2;

	String format = "%";
	format += prec;
	format += ".4f";

	val1 = (int)value;
	val2 = value - val1;
	val1 *= 100;
	val2 *= 60;

	sprintf(buffer_, format.c_str(), val1 + val2);

}

int GPRMC::toString(char *buffer_, size_t max_size_) {

	int heure, minute, sec;
	char x[256];
	String tmp, res = "$GPRMC,";

	memset(buffer_, 0, max_size_);

	heure = _date / 3600;
	minute = (_date % 3600) / 60;
	sec = _date % 60;

	sprintf(x, "%02d%02d%02d,A,", heure, minute, sec);
	res += x;

	this->coordtoString(x, 256, 8, c_latitude);
	res += x;
	res += ",N,";
	this->coordtoString(x, 256, 9, c_longitude);
	res += x;
	if (c_longitude > 0) res += ",E,";
	else res += ",W,";

	res.replace(" ", "0");

	// speed in knots
	sprintf(x, "%5.1f", _vitesse * 0.539957);
	tmp = x;

	tmp.replace(",", ".");
	tmp.replace(" ", "0");

	res += tmp;

	res += ",231.8,171115,004.2,W";

	int sum = 0;
	for (int i = 1; i < res.length(); i++) {
		sum ^= (uint8_t) res[i];
	}

	sprintf(x, "*%02X\r\n", sum);
	res += x;

	res.toCharArray(buffer_, max_size_);

	return res.length();
}
