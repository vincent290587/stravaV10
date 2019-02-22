/*
 * UserSettings.h
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#ifndef SOURCE_MODEL_USERSETTINGS_H_
#define SOURCE_MODEL_USERSETTINGS_H_

#include <stdint.h>


typedef union {
	struct {
		uint8_t crc;
	};
	uint8_t flat_user_params[];
} sUserParameters;


class UserSettings {
public:
	UserSettings();


private:
	sUserParameters m_params;
};


#endif /* SOURCE_MODEL_USERSETTINGS_H_ */
