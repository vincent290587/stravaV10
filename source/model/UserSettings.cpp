/*
 * UserSettings.cpp
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "utils.h"
#include "fram.h"
#include "UserSettings.h"
#include "segger_wrapper.h"

UserSettings::UserSettings() {
	m_is_init = false;
}

bool UserSettings::isConfigValid(void) {

	if (!m_is_init) {
		m_is_init = true;

		if (!fram_read_block(FRAM_SETTINGS_ADDRESS, m_params.flat_user_params, sizeof(sUserParameters)))
			return false;
	}

	uint8_t crc = calculate_crc(m_params.flat_user_params, sizeof(sUserParameters) - 1);

	if (crc != m_params.crc) {
		LOG_ERROR("User parameters: wrong CRC");
		return false;
	}

	LOG_INFO("User parameters read correctly !");

	if (FRAM_SETTINGS_VERSION != m_params.version) {
		LOG_ERROR("User parameters: wrong version");
		return false;
	}

	LOG_INFO("User parameters set");

	return true;
}

bool UserSettings::resetConfig(void) {

	memset(&m_params, 0, sizeof(sUserParameters));

	m_params.version = FRAM_SETTINGS_VERSION;

	m_params.crc = calculate_crc(m_params.flat_user_params, sizeof(sUserParameters) - 1);

	return fram_write_block(FRAM_SETTINGS_ADDRESS, m_params.flat_user_params, sizeof(sUserParameters));
}
