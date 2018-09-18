/*
 * Vue.h
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_TDD_H_
#define SOURCE_VUE_TDD_H_

#include <vue/VueCommon.h>
#include <vue/Notif.h>
#include <button.h>

typedef enum {
	eVueGlobalScreenCRS,
	eVueGlobalScreenFEC,
	eVueGlobalScreenPRC,
} eVueGlobalScreenModes;

class Vue_TDD: public NotifiableDevice {
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

	void clearDisplay(void) {
		//LS027_Clear();
	}
	void invertDisplay(void) {
		//LS027_InvertColors();
	}
	void writeWhole(void)
	{
		//LS027_UpdateFull();
	}
private:
	eVueGlobalScreenModes m_global_mode;


};

#endif /* SOURCE_VUE_VUE_H_ */
