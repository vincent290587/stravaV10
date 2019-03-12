/*
 * UserSettings.cpp
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "utils.h"
#include "fram.h"
#include "Model.h"
#include "parameters.h"
#include "UserSettings.h"
#include "ant_device_manager.h"
#include "segger_wrapper.h"

static sUserParameters s_m_params;


sUserParameters *user_settings_get(void) {
	return &s_m_params;
}


UserSettings::UserSettings() : m_params(s_m_params) {
	m_is_init = false;
}

bool UserSettings::isConfigValid(void) {

	if (!m_is_init) {

		if (!fram_read_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters)))
			return false;

		m_is_init = true;
	}

	uint8_t crc = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

	if (crc != m_params.crc) {
		LOG_ERROR("User parameters: wrong CRC, found 0x%02X", crc);
		return false;
	}

	if (FRAM_SETTINGS_VERSION != m_params.version) {
		LOG_ERROR("User parameters: wrong version, found %u", m_params.version);

		this->resetConfig();
	} else {
		LOG_WARNING("User parameters V%u read correctly", m_params.version);
	}

	return true;
}

bool UserSettings::resetConfig(void) {

	m_is_init = false;

	memset(&m_params, 0, sizeof(sUserParameters));

	m_params.version = FRAM_SETTINGS_VERSION;
	m_params.FTP = USER_FTP;
	m_params.weight = USER_WEIGHT;
	m_params.hrm_devid = HRM_DEVICE_NUMBER;
	m_params.bsc_devid = BSC_DEVICE_NUMBER;
	m_params.fec_devid = TACX_DEVICE_NUMBER;
	m_params.gla_devid = GLASSES_DEVICE_NUMBER;

	m_params.crc = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

	LOG_WARNING("User params factory reset");

	return fram_write_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters));
}

void UserSettings::checkConfigVersion(void) {

	if (fram_read_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters))) {

		uint8_t crc = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

		if (crc != m_params.crc) {
			this->resetConfig();

			LOG_WARNING("Basic user params set");
		}
	}
}
