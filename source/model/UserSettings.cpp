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
		m_is_init = true;

		if (!fram_read_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters)))
			return false;
	}

	uint8_t crc = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

	if (crc != m_params.crc) {
		LOG_ERROR("User parameters: wrong CRC, found 0x%02X", crc);
		return false;
	}

	LOG_INFO("User parameters read correctly !");

	if (FRAM_SETTINGS_VERSION != m_params.version) {
		LOG_ERROR("User parameters: wrong version, found %u", m_params.version);
		return false;
	}

	LOG_INFO("User parameters set");

	return true;
}

bool UserSettings::resetConfig(void) {

	m_is_init = false;

	memset(&m_params, 0, sizeof(sUserParameters));

	m_params.version = FRAM_SETTINGS_VERSION;
	m_params.FTP = USER_FTP;
	m_params.weight = USER_WEIGHT;

	m_params.crc = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - sizeof(m_params.crc));

	return fram_write_block(FRAM_SETTINGS_ADDRESS, &m_params.flat_user_params, sizeof(sUserParameters));
}
