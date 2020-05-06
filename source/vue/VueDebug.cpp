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

	vue.println("  ----- SNS -----");

	info = String(" STC: ") + String((int)stc.getCurrent());
	info += "mA @ ";
	info += String((int)(stc.getVoltage()*1000.f));
	info += "mV";
	vue.println(info);

	vue.println("  ----- MEM -----");

	String segs = " ";
	segs += mes_segments.size();
	segs += " segments loaded";
	vue.println(segs);

	this->cadranH(6, VUE_GPS_NB_LINES, "Time", _timemkstr(att.date, ':'), NULL);

	this->cadran(7, VUE_GPS_NB_LINES, 1, "STC", _imkstr((int)stc.getCurrent()), "mA");
	this->cadran(7, VUE_GPS_NB_LINES, 2, "SOC", _imkstr((int)percentageBatt(stc.getVoltage(), stc.getCurrent())), "%");


}
