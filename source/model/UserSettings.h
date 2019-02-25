/*
 * UserSettings.h
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#ifndef SOURCE_MODEL_USERSETTINGS_H_
#define SOURCE_MODEL_USERSETTINGS_H_

#include <stdint.h>
#include <stdbool.h>
#include "g_structs.h"


#define FRAM_SETTINGS_ADDRESS        0x0000
#define FRAM_SETTINGS_VERSION        0x0001


class UserSettings {
public:
	UserSettings();

	bool isConfigValid(void);
	bool resetConfig(void);

	uint16_t getHRMdevID(void) const {
		return m_params.hrm_devid;
	}

	uint16_t getBSCdevID(void) const {
		return m_params.bsc_devid;
	}

	uint16_t getFECdevID(void) const {
		return m_params.fec_devid;
	}

private:
	bool m_is_init = false;
	sUserParameters m_params;
};


#endif /* SOURCE_MODEL_USERSETTINGS_H_ */
