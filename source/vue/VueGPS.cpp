/*
 * VueGPS.cpp
 *
 *  Created on: 27 févr. 2018
 *      Author: Vincent
 */

#include <VueGPS.h>
#include "Screenutils.h"
#include "Model.h"

#define VUE_GPS_NB_LINES    7

VueGPS::VueGPS() : Adafruit_GFX(0, 0) {

}

void VueGPS::displayGPS() {

	locator.displayGPS2();

	this->cadranH(6, VUE_GPS_NB_LINES, "Time", _timemkstr(att.date, ':'), NULL);

	this->cadran(7, VUE_GPS_NB_LINES, 1, "STC", _imkstr((int)stc.getCurrent()), "mA");
	this->cadran(7, VUE_GPS_NB_LINES, 2, "SOC", _imkstr(percentageBatt(stc.getVoltage(), stc.getCurrent())), "%");

}
