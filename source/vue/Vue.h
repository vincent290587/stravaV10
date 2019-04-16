/*
 * Vue.h
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_VUE_H_
#define SOURCE_VUE_VUE_H_

#include <vue/VueCommon.h>
#include <vue/Notif.h>
#include <vue/VueCRS.h>
#include <vue/VueFEC.h>
#include <vue/VuePRC.h>
#include <vue/VueDebug.h>
#include <vue/Menuable.h>
#include <button.h>
#include "ls027.h"

typedef enum {
	eVueGlobalScreenCRS,
	eVueGlobalScreenFEC,
	eVueGlobalScreenPRC,
	eVueGlobalScreenDEBUG,
} eVueGlobalScreenModes;

class Vue: public VueCRS, public VueFEC, public VuePRC, public VueDebug, public NotifiableDevice, public Menuable {
public:
	Vue();

	void init(void);

	void tasks(eButtonsEvent event);

	void setCurrentMode(eVueGlobalScreenModes mode_);

	void refresh(void);

	void drawPixel(int16_t x, int16_t y, uint16_t color);

	void cadran (uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite);
	void cadranH(uint8_t p_lig, uint8_t nb_lig, const char *champ, String  affi, const char *p_unite);

	void cadranRR(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, RRZone &zone);

	void Histo(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, sVueHistoConfiguration& h_config_);
	void HistoH (uint8_t p_lig, uint8_t nb_lig, sVueHistoConfiguration& h_config_);

	void clearDisplay(void) {
		LS027_Clear();
	}
	void invertDisplay(void) {
		LS027_InvertColors();
	}
	void writeWhole(void)
	{
		LS027_UpdateFull();
	}

	uint32_t getLastRefreshed() const {
		return m_last_refreshed;
	}

	// overrides of the GFX library
	void fillRoundRect(int16_t x, int16_t y, int16_t w,
			int16_t h, int16_t r, uint16_t color);

private:
	eVueGlobalScreenModes m_global_mode;
	uint32_t m_last_refreshed;

};

#endif /* SOURCE_VUE_VUE_H_ */
