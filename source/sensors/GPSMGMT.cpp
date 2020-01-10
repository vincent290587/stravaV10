/*
 * GPS.cpp
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#include <GPSMGMT.h>
#include <stdio.h>
#include "uart.h"
#include "utils.h"
#include "gpio.h"
#include "millis.h"
#include "WString.h"
#include "LocusCommands.h"
#include "EPONmeaPacket.h"
#include "Screenutils.h"
#include "sd_functions.h"
#include "segger_wrapper.h"
#include "Locator.h"
#include "Model.h"
#include "boards.h"
#include "parameters.h"

TinyGPSCustom gps_sys(gps, "PMTK010", 2);  // system message

TinyGPSCustom gps_ack(gps, "PMTK001", 2);  // ACK, second element

#define GPS_DEFAULT_SPEED_BAUD     NRF_UARTE_BAUDRATE_9600

#define GPS_FAST_SPEED_BAUD        NRF_UARTE_BAUDRATE_115200
#define GPS_FAST_SPEED_CMD         PMTK_SET_NMEA_BAUD_115200
//#define GPS_FAST_SPEED_BAUD        NRF_UARTE_BAUDRATE_921600
//#define GPS_FAST_SPEED_CMD         PMTK_SET_NMEA_BAUD_921600


#define GPS_UART_SEND(X,Y) do { \
		uart_send(X, Y); } while(0)

#define SEND_TO_GPS(X) do { \
		const_char_to_buffer(X, buffer, sizeof(buffer)); \
		GPS_UART_SEND(buffer, strlen(X)); } while(0)

static uint8_t buffer[256];

static eGPSMgmtEPOState   m_epo_state  = eGPSMgmtEPOIdle;

static bool m_is_uart_on = false;
static bool m_uart_needs_reboot = true;

static nrf_uarte_baudrate_t m_uart_baud = GPS_DEFAULT_SPEED_BAUD;

static int _get_cmd_result(const char *result) {

	int int_res = atoi(result);
	return int_res;
}

static void gps_uart_stop() {

	if (m_is_uart_on) uart_uninit();
	m_is_uart_on = false;

}

static void gps_uart_start() {

	if (m_is_uart_on) return;

	if (m_uart_needs_reboot) {
		m_uart_baud = GPS_DEFAULT_SPEED_BAUD;
		LOG_WARNING("UART needs reboot");
	} else {
		m_uart_baud = GPS_FAST_SPEED_BAUD;
	}

	uart_init(m_uart_baud);
	m_is_uart_on = true;

	m_uart_needs_reboot = false;
}

static void gps_uart_update() {

	if (GPS_FAST_SPEED_BAUD > GPS_DEFAULT_SPEED_BAUD &&
			m_uart_baud == GPS_DEFAULT_SPEED_BAUD) {

		// change GPS baudrate
		SEND_TO_GPS(GPS_FAST_SPEED_CMD);

		// go to final baudrate
		delay_ms(10);
		gps_uart_stop();

		gps_uart_start();
	}
}

GPS_MGMT::GPS_MGMT() {

	m_epo_packet_ind = 0;
	m_epo_packet_nb = 0;
	m_power_state = eGPSMgmtStateInit;
	m_epo_state   = eGPSMgmtEPOIdle;
	m_is_stdby = false;

}

void GPS_MGMT::init(void) {

	// configure fix pin
	nrf_gpio_cfg_input(FIX_PIN, NRF_GPIO_PIN_PULLDOWN);

	nrf_gpio_cfg_output(GPS_R);
	gpio_clear(GPS_R);

	nrf_gpio_cfg_output(GPS_S);
	gpio_set(GPS_S);

	m_uart_needs_reboot = true;

	// HW reset
	this->reset();

}

/**
 * Auto sets the default UART baud rate and resets the GPS chip
 */
void GPS_MGMT::reset(void) {

	gps_uart_stop();

	gpio_clear(GPS_R);
	delay_ms(5);
	gpio_set(GPS_R);

	gps_uart_start();

	m_power_state = eGPSMgmtStateInit;
}

void GPS_MGMT::runWDT(void) {

	static uint32_t last_toggled = 0;

	if (m_epo_state != eGPSMgmtEPOIdle || this->isStandby()) return;

	// check if GPS is in a good state
	if (millis() - last_toggled > 3000 &&
			gps.time.age() > 3000 &&
			gps.time.age() != (-1)UL) {

		last_toggled = millis();

		LOG_WARNING("GPS WDT timeout: %u", gps.time.age());

		gps_uart_stop();

		if (m_uart_baud != GPS_DEFAULT_SPEED_BAUD) {

			m_uart_needs_reboot = true;
			m_power_state = eGPSMgmtStateRunSlow;

		} else if (GPS_DEFAULT_SPEED_BAUD != GPS_FAST_SPEED_BAUD) {

			m_uart_needs_reboot = false;
			m_power_state = eGPSMgmtStateRunFast;

		}

		gps_uart_start();

		LOG_WARNING("Resetting GPS....");
		vue.addNotif("GPS", "Resetting...", 5, eNotificationTypeComplete);
	}
}

bool GPS_MGMT::isFix(void) {
	return gpio_get(FIX_PIN);
}

void GPS_MGMT::standby(void) {

	this->standby(true);

}

void GPS_MGMT::awake(void) {

	this->standby(false);

}

void GPS_MGMT::standby(bool is_standby) {

	if (is_standby) {

		m_is_stdby = true;

		// set to standby
		SEND_TO_GPS(PMTK_STANDBY);

		gpio_clear(GPS_S);
	} else {

		gpio_set(GPS_S);

		m_is_stdby = false;

		SEND_TO_GPS(PMTK_AWAKE);
	}

}

bool GPS_MGMT::isStandby(void) {
	return m_is_stdby;
}

bool GPS_MGMT::isEPOUpdating(void) {
	return m_epo_state == eGPSMgmtEPOIdle;
}

void GPS_MGMT::startEpoUpdate(void) {

//	LOG_INFO("EPO update started\r\n");

	//m_epo_state = eGPSMgmtEPOStart;

	//this->awake();
}

void GPS_MGMT::tasks(void) {

	if (gps_ack.isUpdated()) {
		LOG_WARNING("GPS ack %d message %ums", _get_cmd_result(gps_ack.value()), millis());
	}

	switch (m_power_state) {
	case eGPSMgmtStateInit:
		if (gps_sys.isUpdated()) {
			(void)gps_sys.value();
			LOG_WARNING("GPS sys message (%ums)", millis());

			m_power_state = eGPSMgmtStateRunSlow;
		} else {
			return;
		}
		break;
	case eGPSMgmtStateRunSlow:
	{
#if GPS_USE_COLD_START
		SEND_TO_GPS(PMTK_COLD);
		delay_us(500);
#endif
		// update GPS UART speed
		gps_uart_update();

		m_power_state = eGPSMgmtStateRunFast;
	}
	break;
	case eGPSMgmtStateRunFast:
	default:
		break;
	}

	switch (m_epo_state) {
	case eGPSMgmtEPOIdle:
	{
	}
		break;

	case eGPSMgmtEPOStart:
	{
		m_epo_packet_nb = 0;
		m_epo_packet_ind = 0;

		// determine the time
		int iYr, iMo, iDay, iHr;
		if (!locator.getGPSDate(iYr, iMo, iDay, iHr)) {
			LOG_INFO("EPO start: date not valid");

			if (millis() < 2*60*1000) return;
			else m_epo_state = eGPSMgmtEPOIdle; // stop the process completely

		}
		int current_gps_hour = utc_to_gps_hour(iYr, iMo, iDay, iHr);

		int size = epo_file_size();
		bool res  = epo_file_start(current_gps_hour);

		if (size <= 0) {

			LOG_ERROR("EPO start failure: file empty");

			m_epo_state = eGPSMgmtEPOIdle;

		} else if (!res) {

			LOG_ERROR("EPO start date wrong");

			m_epo_state = eGPSMgmtEPOIdle;

		} else {
			LOG_INFO("EPO size: %ld bytes\r\n", size);

			m_epo_state = eGPSMgmtEPORunning;

			vue.addNotif("GPSMGMT: ", "EPO update started", 4, eNotificationTypeComplete);
		}
	}
	break;

	case eGPSMgmtEPORunning:
	{
		// fill the packet
		if (m_epo_packet_ind < EPO_SAT_SEGMENTS_NB) {

			sEpoPacketSatData epo_data;
			memset(&epo_data, 0, sizeof(epo_data));

			int ret_code = epo_file_read(&epo_data, sizeof(epo_data.sat));

			if (ret_code < 0) {
				// error
			} else if (ret_code == 1) {
				// end
			} else {
				epo_data.sat_number += 1;
			}

			uint16_t written = antenova_epo_packet(&epo_data, buffer, sizeof(buffer));

			// prepare the packet to be sent
			if (epo_data.sat_number < EPO_SAT_SEGMENTS_NB) {
				epo_data.sat_number += 1;

				m_epo_packet_ind++;

				LOG_INFO("EPO sending packet sat#%u", epo_data.sat_number);

				GPS_UART_SEND(buffer, written);
			} else {
				// process is finished
				m_epo_packet_ind = 0xFFFF;

				epo_data.sat_number = m_epo_packet_ind;

				LOG_INFO("EPO last packet\r\n");

				GPS_UART_SEND(buffer, written);
			}

			// TODO check remove
			//m_epo_state = eGPSMgmtEPOWaitForEvent;
		}
	}
	break;

	case eGPSMgmtEPOWaitForEvent:
	{
	}
	break;

	case eGPSMgmtEPOEnd:
	{
		LOG_INFO("EPO update end\r\n");

		vue.addNotif("GPSMGMT: ", "EPO update success", 5, eNotificationTypeComplete);

		// the file should be deleted only upon success
		(void)epo_file_stop(m_epo_packet_ind == 0xFFFF);

		m_epo_state = eGPSMgmtEPOIdle;

	}
	break;

	default:
		break;
	}
}

/**
 *
 * @param c Input character
 * @return Error code
 */
uint32_t gps_encode_char(char c) {

	//LOG_RAW_INFO(c);

	if (eGPSMgmtEPOIdle == m_epo_state) {

		locator_encode_char(c);

	} else {

		LOG_RAW_INFO(c);

	}

	return 0;
}

/**
 *
 * @param loc_data
 * @param age_
 */
void GPS_MGMT::startHostAidingEPO(sLocationData& loc_data, uint32_t age_) {

	LOG_INFO("Host aiding alt: %d", (int)loc_data.alt);

	uint16_t _year  = 2000 + (loc_data.date % 100);
	uint16_t _month = (loc_data.date / 100) % 100;
	uint16_t _day   = (loc_data.date / 10000) % 100;

	memset(buffer, 0, sizeof(buffer));

	uint32_t value = loc_data.utc_time;
	uint8_t hours   = (uint8_t) (value / 3600);
	value -= hours * 3600;
	uint8_t minutes = (uint8_t) (value / 60);
	value -= minutes * 60;
	uint8_t seconds = (uint8_t) (value % 60);

	int res = snprintf((char*)buffer, sizeof(buffer), "$PMTK741,%.6f,%.6f,%d,%u,%u,%u,%02u,%02u,%02u",
			loc_data.lat, loc_data.lon, (int)loc_data.alt, _year, _month, _day,
			hours, minutes, seconds);

	if (res <= 0) return;

	ASSERT((uint32_t)res + 10 < sizeof(buffer));

	// handle checksum
	uint8_t ret = 0;
	for (uint16_t i = 1; i < res; i++) {
		ret ^= buffer[i];
	}

	res += snprintf((char*)buffer + res, sizeof(buffer) - res, "*%02X\r\n", ret);

	GPS_UART_SEND(buffer, res);

	LOG_INFO("Host aiding: %s", (char*)buffer);

	vue.addNotif("EPO", "Host aiding sent", 5, eNotificationTypeComplete);
}

/**
 *
 * @param interval
 */
void GPS_MGMT::setFixInterval(uint16_t interval) {

	String cmd = "$PMTK220," + interval;

	memset(buffer, 0, sizeof(buffer));

	cmd.toCharArray((char*)buffer, sizeof(buffer), 0);

	// handle checksum
	uint8_t ret = 0;
	for (uint16_t i = 1; i < cmd.length(); i++) {
		ret ^= buffer[i];
	}

	snprintf((char*)buffer + cmd.length(), sizeof(buffer) - cmd.length(), "*%02X\r\n", ret);

	GPS_UART_SEND(buffer, cmd.length() + 5);

}
