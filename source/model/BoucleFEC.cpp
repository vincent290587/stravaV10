/*
 * BoucleFEC.cpp
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdbool.h>
#include <BoucleFEC.h>
#include "Model.h"
#include "segger_wrapper.h"
#include "parameters.h"
#include "Locator.h"


static tHistoValue m_st_buffer[FEC_PW_BUFFER_NB_ELEM];


/**
 *
 */
BoucleFEC::BoucleFEC() : m_pw_buffer(FEC_PW_BUFFER_NB_ELEM, m_st_buffer) {
	m_last_run_time = 0;
}

/**
 *
 * @return true if we are ready to run
 */
bool BoucleFEC::isTime() {

	if (millis() - m_last_run_time >= BOUCLE_FEC_UPDATE_RATE_MS) {
		m_last_run_time = millis();
		return true;
	}

	return false;
}

/**
 *
 */
void BoucleFEC::init() {

	LOG_INFO("Boucle FEC init\r\n");

	// turn GPS OFF
	gps_mgmt.standby();

	m_needs_init = false;
}

/**
 *
 */
void BoucleFEC::run() {

	if (m_needs_init) this->init();

	LOG_INFO("Boucle FEC run\r\n");

	// TODO add FEC point to record
//	attitude.addNewFECPoint();

	if (m_pw_buffer.isFull()) {
		m_pw_buffer.popLast();
	}
	m_pw_buffer.add(&fec_info.data.power);

	vue.refresh();

}
