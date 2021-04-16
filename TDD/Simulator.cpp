/*
 * Simulator.cpp
 *
 *  Created on: 18 sept. 2018
 *      Author: Vincent
 */

#include <random>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "ff.h"
#include "millis.h"
#include "bme280.h"
#include "gpio.h"
#include "boards.h"
#include "tdd_logger.h"
#include "uart_tdd.h"
#include "usb_cdc.h"
#include "Model.h"
#include "Simulator.h"
#include "sd_functions.h"
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

static FILE* g_fileObject;   /* File object */

#ifdef LS027_GUI
#define NEW_POINT_PERIOD_MS       400
#else
#define NEW_POINT_PERIOD_MS       1000
#endif

static uint32_t nb_gps_loc = 0;

static float last_dl;
static float cur_speed = 20.0f;
static float alt_sim = 100.0f;

static std::default_random_engine s_generator;
static std::normal_distribution<float> distr_speed(0.f, .4f);

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
			w_task_yield();
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

		m_next_event_ms = millis() + 15000;

		m_step = eSimulationStepEnd;
	} break;

	case eSimulationStepEnd:
	if (millis() > m_next_event_ms) {
		// continue to cycle through screens
		vue.tasks(eButtonsEventRight);

		m_next_event_ms = millis() + 15000;
	} break;
	default:
		break;
	}

}

void print_mem_state(void) {

	static int max_mem_used = 0;
	int tot_point_mem = 0;
	tot_point_mem += Point::getObjectCount() * sizeof(Point);
	tot_point_mem += Point2D::getObjectCount() * sizeof(Point2D);
	tot_point_mem += segMngr.getNbSegs() * sizeof(sSegmentData);

	if (tot_point_mem > max_mem_used) max_mem_used = tot_point_mem;

	LOG_INFO(">> Allocated pts: %d 2D %d 3D / mem %d o / %d o",
			Point2D::getObjectCount(), Point::getObjectCount(),
			tot_point_mem, max_mem_used);
}

void simulator_simulate_altitude(float alti) {

	const float sea_level_pressure = 1015.0f;

	// res = 44330.0f * (1.0f - powf(atmospheric / sea_level_pressure, 0.1903f));
	static std::default_random_engine generator;
	static std::normal_distribution<float> distr_alt(0.0, 0.25f);

	alti += distr_alt(generator);

	tdd_logger_log_float(TDD_LOGGING_ALT_SIM, alti);

	bme280_set_pressure(sea_level_pressure * powf(1.0f - alti / 44330.0f, 1.0f / 0.1903f));

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
	tdd_logger_log_name(TDD_LOGGING_BARO_DIFF, "baro_corr");
	tdd_logger_log_name(TDD_LOGGING_CUR_POWER, "power");
	tdd_logger_log_name(TDD_LOGGING_CUR_SPEED, "speed");

	g_fileObject = fopen("./../TDD/GPX_simu.csv", "r");

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
	static uint32_t hrm_timestamp = 0;
	hrm_info.bpm = 120 + (rand() % 65);
	if(millis() - hrm_timestamp > hrm_info.rr) {
		hrm_info.rr = 900 + (rand() % 24);
//		static int i=0;
//		hrm_info.rr = i++ %2==0 ? 700 : 900;
		hrm_info.timestamp = millis();

		hrm_timestamp = millis();
	}

	// FEC simulation
	fec_info.power = rand() % 500;
	uint32_t millis_ = millis() / 1000;
	fec_info.el_time = (uint8_t)(millis_ & 0xFF);
	w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_FEC_INFO);
	w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_FEC_POWER);
}

static void _fxos_sim(float cur_a, float cur_a0) {

	static uint32_t last_point_ms = 0;
	if (millis() - last_point_ms < 40) return;

	static std::default_random_engine generator;
	static std::normal_distribution<float> distr_alt_x(0.0, 800);
	static std::normal_distribution<float> distr_alt_y(0.0, 400);
	static std::normal_distribution<float> distr_alt_z(0.0, 800);

	float val_x = 2048.f * sinf(cur_a + cur_a0);
	float val_z = 2048.f * cosf(cur_a + cur_a0);

	val_x += distr_alt_x(generator);
	float val_y = distr_alt_y(generator);
	val_z += distr_alt_z(generator);

	// add noise
	fxos_set_xyz(val_x, val_y, val_z);
}

static void _sensors_sim(void) {

	static uint32_t last_point_ms = 0;
	static float cur_a = toRadians(5.0f); // = 8.75 %
	static const float cur_a0 = toRadians(3.4f);
	static uint32_t sim_nb = 0;

	_fxos_sim(cur_a, cur_a0);

	if (millis() - last_point_ms < BARO_REFRESH_PER_MS) return;

	alt_sim += tanf(cur_a) * last_dl * (millis() - last_point_ms) / 1000.f; // over 1 second

	if (++sim_nb > 2000) {

		cur_a = -cur_a;

		sim_nb = 0;
	}

	simulator_simulate_altitude(alt_sim);

	LOG_DEBUG("Simulating sensors loop");

	// have to simulate the baro sleep...
	extern void timer_handler(void * p_context);
	timer_handler(NULL);

	last_point_ms = millis();

	tdd_logger_log_float(TDD_LOGGING_SIM_SLOPE, 100 * tanf(cur_a));

	tdd_logger_flush();
}

static void _loc_sim(void) {

	if (!g_fileObject) {
		LOG_ERROR("No simulation file found");
		exit(-3);
	}

	static uint32_t last_point_ms = 0;
	if (millis() - last_point_ms < NEW_POINT_PERIOD_MS) return;
	if (millis() < 10000) {

		gpio_clear(FIX_PIN);

		if (gpio_get(GPS_R) && gpio_get(GPS_S) ) {
			const char *e_gprmc = "$GPRMC,,V,,,,,,,,,,N*53\r\n"
					"$GNGGA,,,,,,0,6,1.93,34.9,M,17.8,M,,*5B\r\n"
					"$GPGSV,3,1,10,42,54,137,,14,51,128,46,31,48,015,48,16,42,243,*72\r\n"
					"$GPGSV,3,2,10,29,23,054,44,193,20,175,37,03,17,298,,27,15,190,37*47\r\n"
					"$GPGSV,3,3,10,22,10,176,,25,06,043,38*74\r\n";

			// send to uart_tdd
			for (int i=0; i < strlen(e_gprmc); i++)
				uart_rx_handler(e_gprmc[i]);
		}

		last_point_ms = millis();
		return;
	}

	last_point_ms = millis();

	simulator_modes();

	if (fgets(g_bufferRead, sizeof(g_bufferRead)-1, g_fileObject)) {

		float lon, lat, alt;
		static float rtime;
		float data[4];
		char *pch;
		uint16_t pos = 0;

		LOG_INFO("Simulating coordinate...");

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

		static Point lastPoint;

		last_dl = lastPoint.dist(lat, lon);
		if (last_dl > 50.f) {
			last_dl = 50.f;
		}

		cur_speed = 3.6f * last_dl;
		if (rtime - lastPoint._rtime > .5f) {
			cur_speed /= (rtime - lastPoint._rtime);
		}

		cur_speed += distr_speed(s_generator);

		lastPoint._lat = lat;
		lastPoint._lon = lon;
		lastPoint._rtime = rtime;

#ifdef TDD_RANDOMIZE
		int rnd_add;
		rnd_add = (rand() % 20) - 10;
		lat += (float)rnd_add / 150000.;
		rnd_add = (rand() % 20) - 10;
		lon += (float)rnd_add / 150000.;
#endif

		static std::default_random_engine generator;
		static std::normal_distribution<float> distr_alt(0.0, 0.1f);

		if (millis() < 60000 &&
				(gpio_get(GPS_R) && gpio_get(GPS_S)) ) {

			gpio_set(FIX_PIN);

			// build make NMEA sentence
			GPRMC gprmc_(lat, lon, cur_speed, (int)rtime);
			int nmea_length = gprmc_.toString(g_bufferWrite, sizeof(g_bufferWrite));

			LOG_WARNING("Sentence: %s", g_bufferWrite);

			// send to uart_tdd
			for (int i=0; i < nmea_length; i++)
				uart_rx_handler(g_bufferWrite[i]);

			GPGGA gpgga_(lat, lon, alt_sim + 32.3f + distr_alt(generator), (int)rtime);
			nmea_length = gpgga_.toString(g_bufferWrite, sizeof(g_bufferWrite));

			LOG_WARNING("Sentence: %s", g_bufferWrite);

			// send to uart_tdd
			for (int i=0; i < nmea_length; i++)
				uart_rx_handler(g_bufferWrite[i]);

			const char *e_gpgsv = "$GNVTG,286.99,T,,M,0.62,N,1.15,K,A*2E\r\n";

			// send to uart_tdd
			for (int i=0; i < strlen(e_gpgsv); i++)
				uart_rx_handler(e_gpgsv[i]);

			if (++nb_gps_loc == 1) {
				sLocationData loc_data;
				loc_data.alt = 9.5;
				loc_data.lat = lat;
				loc_data.lon = lon;
				loc_data.utc_time = 15 * 3600 + 5 * 60 + 39;
				loc_data.date = 11218;

				gps_mgmt.startHostAidingEPO(loc_data, 350);
			}
		} else {
			sLnsInfo lns_info;
			lns_info.lat = lat * 10000000.;
			lns_info.lon = lon * 10000000.;
			lns_info.ele = (alt_sim + 32.3f + distr_alt(generator)) * 100.;
			lns_info.secj = (int)rtime;
			lns_info.utc_timestamp = date_to_timestamp(lns_info.secj, 1, 12, 2018);
			lns_info.date = 11218;
			lns_info.heading = 5;
			lns_info.speed = cur_speed * 10.;
			locator_dispatch_lns_update(&lns_info);

			gpio_clear(FIX_PIN);

			if (gpio_get(GPS_R) && gpio_get(GPS_S) ) {
				const char *e_gpgsv = ""
						"$GNGGA,,,,,,0,6,1.93,34.9,M,17.8,M,,*5B\r\n"
						"$GNVTG,286.99,T,,M,0.62,N,1.15,K,A*2E\r\n";

				// send to uart_tdd
				for (int i=0; i < strlen(e_gpgsv); i++)
					uart_rx_handler(e_gpgsv[i]);
			}
		}

	} else {
		fclose(g_fileObject);

		// test msc mode
		usb_cdc_start_msc();

#ifdef TDD_RANDOMIZE
		static int nb_tests = 0;
		g_fileObject = fopen("./../TDD/GPX_simu.csv", "r");
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

		fit_terminate();

		// print memory state
		print_mem_state();

		assert(Point2D::getObjectCount() == 1);
		assert(Point::getObjectCount() == 3);

		exit(0);

	}
}

void simulator_tasks(void) {

	tdd_logger_log_int(TDD_LOGGING_TIME, millis());

	if (millis() < 500) {

		tdd_logger_start();
		return;
	}

	tdd_logger_log_int(TDD_LOGGING_P2D, Point2D::getObjectCount());
	tdd_logger_log_int(TDD_LOGGING_P3D, Point::getObjectCount());

	_fec_sim();
	_loc_sim();
	_sensors_sim();

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

	res += ",231.8,11218,004.2,W";

	int sum = 0;
	for (int i = 1; i < res.length(); i++) {
		sum ^= (uint8_t) res[i];
	}

	sprintf(x, "*%02X\r\n", sum);
	res += x;

	res.toCharArray(buffer_, max_size_);

	return res.length();
}

GPGGA::GPGGA(float latitude, float longitude, float altitude, int sec_jour) {

	c_latitude = latitude;
	c_longitude = longitude;
	_altitude = altitude;
	_date = sec_jour;

}

void GPGGA::coordtoString(char* buffer_, size_t max_size_, uint16_t prec, float value) {

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

int GPGGA::toString(char *buffer_, size_t max_size_) {

	int heure, minute, sec;
	char x[256];
	String tmp, res = "$GPGGA,";

	memset(buffer_, 0, max_size_);

	heure = _date / 3600;
	minute = (_date % 3600) / 60;
	sec = _date % 60;

	sprintf(x, "%02d%02d%02d,", heure, minute, sec);
	res += x;

	this->coordtoString(x, 256, 8, c_latitude);
	res += x;
	res += ",N,";
	this->coordtoString(x, 256, 9, c_longitude);
	res += x;
	if (c_longitude > 0) res += ",E,";
	else res += ",W,";

	res.replace(" ", "0");

	// fix quality + sats in view + hdop
	res += "1,05,2.5,";

	// altitude in meters
	sprintf(x, "%5.1f", _altitude);
	tmp = x;

	tmp.replace(",", ".");
	tmp.replace(" ", "0");

	res += tmp;
	res += ",M";

	// height of geoid + blank + blank
	res += ",0.0,M,,,";

	int sum = 0;
	for (int i = 1; i < res.length(); i++) {
		sum ^= (uint8_t) res[i];
	}

	sprintf(x, "*%02X\r\n", sum);
	res += x;

	res.toCharArray(buffer_, max_size_);

	return res.length();
}
