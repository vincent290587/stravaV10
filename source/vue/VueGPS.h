/*
 * VueGPS.h
 *
 *  Created on: 27 févr. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_VUEGPS_H_
#define SOURCE_VUE_VUEGPS_H_

#include <Adafruit_GFX.h>


class VueGPS: virtual public Adafruit_GFX {
public:
	VueGPS();

	virtual void cadranH(uint8_t p_lig, uint8_t nb_lig, const char *champ, String  affi, const char *p_unite)=0;
	virtual void cadran(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite)=0;

	void displayGPS();
};

#endif /* SOURCE_VUE_VUEGPS_H_ */
