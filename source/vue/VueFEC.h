/*
 * VueFEC.h
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_VUEFEC_H_
#define SOURCE_VUE_VUEFEC_H_

#include <Adafruit_GFX.h>
#include <vue/VueCommon.h>
#include "BinnedData.h"
#include "RRZone.h"

typedef enum {
	eVueFECScreenInit,
	eVueFECScreenDataFull,
} eVueFECScreenModes;

class VueFEC: virtual public Adafruit_GFX {
public:
	VueFEC();

	virtual void cadran(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite)=0;
	virtual void cadranH(uint8_t p_lig, uint8_t nb_lig, const char *champ, String  affi, const char *p_unite)=0;

	virtual void cadranRR(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, RRZone &zone)=0;

	virtual void Histo(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, sVueHistoConfiguration& h_config_)=0;
	virtual void HistoH (uint8_t p_lig, uint8_t nb_lig, sVueHistoConfiguration& h_config_)=0;

	void cadranZones(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, BinnedData &data);

	void cadranPowerVector(uint8_t p_lig, uint8_t nb_lig, const char *champ, sPowerVector &vector);

protected:
	eVueFECScreenModes m_fec_screen_mode;

	eVueFECScreenModes tasksFEC();

};

#endif /* SOURCE_VUE_VUEFEC_H_ */
