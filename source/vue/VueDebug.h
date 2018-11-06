/*
 * VueDebug.h
 *
 *  Created on: 27 févr. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_VUEDEBUG_H_
#define SOURCE_VUE_VUEDEBUG_H_

#include <Adafruit_GFX.h>


class VueDebug: virtual public Adafruit_GFX {
public:
	VueDebug();

	virtual void cadranH(uint8_t p_lig, uint8_t nb_lig, const char *champ, String  affi, const char *p_unite)=0;
	virtual void cadran(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite)=0;

	void displayDebug();
};

#endif /* SOURCE_VUE_VUEDEBUG_H_ */
