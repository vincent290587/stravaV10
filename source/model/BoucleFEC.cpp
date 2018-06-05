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
#include "spi_scheduler.h"
#include "parameters.h"
#include "nrf52.h"
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

	// CPU reduced speed
	pwManager.switchToRun24();

	if (m_needs_init) this->init();

	LOG_INFO("Boucle FEC run\r\n");

	nrf52_page0.fec_info.type = eFecControlNone;
	nrf52_page0.fec_info.data.power_control.target_power_w = 150;

	nrf52_refresh();

	dma_spi0_mngr_tasks_start();
	dma_spi0_mngr_finish();

	// TODO add FEC point to record
//	attitude.addNewFECPoint();

	if (m_pw_buffer.isFull()) {
		m_pw_buffer.popLast();
	}
	m_pw_buffer.add(&fec_info.data.power);

	vue.refresh();

	dma_spi0_mngr_tasks_start();
	dma_spi0_mngr_finish();

	// go back to very low power
	pwManager.switchToVlpr();

}
