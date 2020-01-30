/*
 * Simulator.h
 *
 *  Created on: 18 sept. 2018
 *      Author: Vincent
 */

#ifndef TDD_SIMULATOR_H_
#define TDD_SIMULATOR_H_

#include "WString.h"

#ifdef __cplusplus
extern "C" {
#endif

void bme280_set_pressure(float);

void simulator_init(void);

void simulator_tasks(void);

#ifdef __cplusplus
}
#endif

class GPRMC {
public:
	GPRMC(float latitude, float longitude, float vitesse, int sec_jour);

	void coordtoString(char* buffer_, size_t max_size_, uint16_t prec, float value);
	int toString(char *buffer_, size_t max_size_);

private:
	float c_latitude;
	float c_longitude;
	float _vitesse;
	int _date;
};

#endif /* TDD_SIMULATOR_H_ */
