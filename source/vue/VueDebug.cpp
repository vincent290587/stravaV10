/*
 * VueGPS.cpp
 *
 *  Created on: 27 févr. 2018
 *      Author: Vincent
 */

#include <VueDebug.h>
#include "Screenutils.h"
#include "Model.h"

#define VUE_GPS_NB_LINES    7

VueDebug::VueDebug() : Adafruit_GFX(0, 0) {

}

void VueDebug::displayDebug() {

	String info = "";

	vue.setCursor(20,20);
	vue.setTextSize(2);

	locator.displayGPS2();

	info = String("STC: ") + String((int)stc.getCurrent());
	vue.println(info);

	this->cadranH(6, VUE_GPS_NB_LINES, "Time", _timemkstr(att.date, ':'), NULL);

	this->cadran(7, VUE_GPS_NB_LINES, 1, "STC", _imkstr((int)stc.getCurrent()), "mA");
	this->cadran(7, VUE_GPS_NB_LINES, 2, "SOC", _imkstr((int)percentageBatt(stc.getVoltage(), stc.getCurrent())), "%");


}
