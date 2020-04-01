/*
 * UserSettings.cpp
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "fram.h"
#include "Model.h"
#include "parameters.h"
#include "UserSettings.h"
#include "ant_device_manager.h"
#include "segger_wrapper.h"

static sUserParameters s_m_params;

/**
 * Calculates the CRC8 of an array
 */
static uint8_t _calculate_crc( uint8_t *addr, uint16_t len) {
	uint8_t crc=0;
	for (uint8_t i=0; i<len;i++) {
		uint8_t inbyte = addr[i];
		for (uint8_t j=0;j<8;j++) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix)
				crc ^= 0x8C;
			inbyte >>= 1;
		}
	}

	return crc;
}


sUserParameters *user_settings_get(void) {
	return &s_m_params;
}


UserSettings::UserSettings() : m_params(s_m_params) {
	m_is_init = false;
}

void UserSettings::sync(uint8_t force) {

	if (!m_is_init || force) {

		if (!fram_read_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters)))
			return;

		m_is_init = true;
	}
}

bool UserSettings::isConfigValid(void) {

	this->sync();

	uint8_t crc = _calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

	if (crc != m_params.crc) {
		LOG_ERROR("User parameters: wrong CRC, found 0x%02X", crc);
		return false;
	}

	if (FRAM_SETTINGS_VERSION != m_params.version) {
		LOG_ERROR("User parameters: wrong version, found %u", m_params.version);
		return false;
	} else {
		LOG_WARNING("User parameters V%u read correctly", m_params.version);
	}

	return true;
}

bool UserSettings::writeConfig(void) {

	m_params.crc = _calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

	bool res = fram_write_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters));

	LOG_WARNING("User params written");

	return res;
}

void UserSettings::dump(void) {

	uint8_t *data = &m_params.flat_user_params;

	LOG_INFO("Dumping config: ");
	for (uint16_t i=0; i < sizeof(sUserParameters); i++) {

		LOG_INFO("[%u]", data[i]);

	}

}

bool UserSettings::resetConfig(void) {

	m_is_init = false;

	memset(&m_params, 0, sizeof(sUserParameters));

	m_params.version   = FRAM_SETTINGS_VERSION;
	m_params.FTP       = USER_FTP;
	m_params.weight    = USER_WEIGHT;
	m_params.hrm_devid = HRM_DEVICE_NUMBER;
	m_params.bsc_devid = BSC_DEVICE_NUMBER;
	m_params.fec_devid = TACX_DEVICE_NUMBER;
	m_params.gla_devid = GLASSES_DEVICE_NUMBER;

	LOG_WARNING("User params factory reset");

	return this->writeConfig();
}

void UserSettings::enforceConfigVersion(void) {

	this->sync(1);

	if (!this->isConfigValid()) {
		this->resetConfig();
	}

	// config should be good
}
