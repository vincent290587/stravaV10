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
#include "uart_tdd.h"
#include "Model_tdd.h"
#include "Simulator.h"
#include "segger_wrapper.h"

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
#define NEW_POINT_PERIOD_MS       50
#endif

static uint32_t last_point_ms = 0;
static uint32_t nb_gps_loc = 0;

void simulator_init(void) {

	g_fileObject = fopen("GPX_simu.csv", "r");

	m_app_error.special = 0xDB;
	m_app_error.crc_att = 0xFB;
	snprintf(m_app_error._buffer, sizeof(m_app_error._buffer), "Error 0x123456 in file /mnt/e/Nordic/Projects/Perso/stravaV10/TDD/Simulator.cpp:48");
	m_app_error.saved_att.climb = 562.;
	m_app_error.saved_att.dist = 17700;
	m_app_error.saved_att.pr = 3;
	m_app_error.saved_att.date.date = 211218;
}

void simulator_tasks(void) {

	if (!g_fileObject) {
		printf("No simulation file found");
		exit(-3);
	}

	if (millis() < 5000) {
		return;
	}

	if (millis() - last_point_ms < NEW_POINT_PERIOD_MS) return;

	// HRM simulation
	hrm_info.bpm = 120 + (rand() % 65);

	// FEC simulation
	fec_info.power = rand() % 500;
	fec_info.speed = 20.;
	fec_info.el_time++;
	events_set(m_tasks_id.boucle_id, TASK_EVENT_FEC_INFO);
	events_set(m_tasks_id.boucle_id, TASK_EVENT_FEC_POWER);

	print_mem_state();

	last_point_ms = millis();

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

		baro.setAlti(alt);

		if (pos == 4) {
			// file contains the rtime
			rtime = data[3];
		} else {
			// rtime is missing: generate it
			rtime += 1;
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

		mes_segments._segs.empty();

		exit(0);

	}

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
