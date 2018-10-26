/*
 * Vue.h
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_TDD_H_
#define SOURCE_VUE_TDD_H_

#include "VueCRS.h"
#include "VueFEC.h"
#include "VuePRC.h"
#include <vue/VueCommon.h>
#include <vue/Notif.h>
#include <button.h>

typedef enum {
	eVueGlobalScreenCRS,
	eVueGlobalScreenFEC,
	eVueGlobalScreenPRC,
	eVueGlobalScreenDEBUG,
} eVueGlobalScreenModes;

class Vue_TDD: public VueCRS, public VueFEC, public VuePRC, public VueDebug, public NotifiableDevice {
public:
	Vue_TDD();

	void init(void);

	void tasks(eButtonsEvent event);

	void setCurrentMode(eVueGlobalScreenModes mode_);

	void refresh(void);

	void drawPixel(int16_t x, int16_t y, uint16_t color);

	void cadran (uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite);
	void cadranH(uint8_t p_lig, uint8_t nb_lig, const char *champ, String  affi, const char *p_unite);

	void Histo(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, sVueHistoConfiguration& h_config_);
	void HistoH (uint8_t p_lig, uint8_t nb_lig, sVueHistoConfiguration& h_config_);

	void clearDisplay(void);
	void invertDisplay(void);
	void writeWhole(void);
private:
	eVueGlobalScreenModes m_global_mode;


};

#endif /* SOURCE_VUE_VUE_H_ */
