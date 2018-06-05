/*
 * VueCRS.h
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_VUECRS_H_
#define SOURCE_VUE_VUECRS_H_

#include "parameters.h"
#include <display/SegmentManager.h>
#include <Adafruit_GFX.h>
#include <vue/VueGPS.h>


typedef enum {
	eVueCRSScreenInit,
	eVueCRSScreenDataFull,
	eVueCRSScreenDataSS,
	eVueCRSScreenDataDS,
} eVueCRSScreenModes;


class VueCRS: virtual public Adafruit_GFX, virtual public VueGPS, virtual public SegmentManager {
public:
	VueCRS();

	eVueCRSScreenModes tasksCRS();

	virtual void cadranH(uint8_t p_lig, uint8_t nb_lig, const char *champ, String  affi, const char *p_unite)=0;
	virtual void cadran(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite)=0;

protected:
	void partner(uint8_t ligne, Segment *p_seg);

	eVueCRSScreenModes m_crs_screen_mode;

private:
	void afficheSegment(uint8_t ligne, Segment *p_seg);

};

#endif /* SOURCE_VUE_VUECRS_H_ */
