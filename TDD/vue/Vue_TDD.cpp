/*
 * Vue.cpp
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#include "Vue_TDD.h"
#include "parameters.h"
#include "assert_wrapper.h"
#include "WString.h"
#include "segger_wrapper.h"


Vue_TDD::Vue_TDD() {

	m_global_mode = VUE_DEFAULT_MODE;

}

void Vue_TDD::init(void) {

}

void Vue_TDD::tasks(eButtonsEvent event) {



}

void Vue_TDD::setCurrentMode(eVueGlobalScreenModes mode_) {

	m_global_mode = mode_;

}

void Vue_TDD::refresh(void) {


}

void Vue_TDD::drawPixel(int16_t x, int16_t y, uint16_t color) {


}

void Vue_TDD::cadranH(uint8_t p_lig, uint8_t nb_lig, const char *champ, String  affi, const char *p_unite) {


}


void Vue_TDD::cadran(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite) {


}

void Vue_TDD::HistoH(uint8_t p_lig, uint8_t nb_lig, sVueHistoConfiguration& h_config_) {


}

void Vue_TDD::Histo(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, sVueHistoConfiguration& h_config_) {


}
