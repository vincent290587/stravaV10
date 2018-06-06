/*
 * Boucle.cpp
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#include <Boucle.h>
#include "segger_wrapper.h"
#include <sd/sd_functions.h>

BoucleCRS boucle_crs;

BoucleFEC boucle_fec;

Boucle::Boucle() {

	m_global_mode = eBoucleGlobalModesInit;

}

void Boucle::init(void) {

	LOG_INFO("Boucle init\r\n");

	init_liste_segments();

	m_global_mode = BOUCLE_DEFAULT_MODE;
}

bool Boucle::isTime(void) {

	switch (m_global_mode) {
	case eBoucleGlobalModesInit:
	{
		return true;
	}
	break;

	case eBoucleGlobalModesCRS:
	{
		return boucle_crs.isTime();
	}
	break;

	case eBoucleGlobalModesFEC:
	{
		return boucle_fec.isTime();
	}
	break;

	case eBoucleGlobalModesPRC:
	{
		return boucle_crs.isTime();
	}
	break;

	default:
		break;
	}

	return false;
}

void Boucle::run(void) {

	switch (m_global_mode) {
	case eBoucleGlobalModesInit:
	{
		this->init();
	}
	break;
	case eBoucleGlobalModesCRS:
	{
		boucle_crs.run();
	}
	break;
	case eBoucleGlobalModesFEC:
	{
		boucle_fec.run();
	}
	break;
	case eBoucleGlobalModesPRC:
	{
		boucle_crs.run();
	}
	break;

	default:
		break;
	}

	return;
}

void Boucle::changeMode(eBoucleGlobalModes new_mode) {

	if (new_mode == m_global_mode) return;

	// finish old operations
	switch (m_global_mode) {
	case eBoucleGlobalModesInit:
	break;
	case eBoucleGlobalModesCRS:
	{
		boucle_crs.invalidate();
	}
	break;
	case eBoucleGlobalModesFEC:
	{
		boucle_fec.invalidate();
	}
	break;
	case eBoucleGlobalModesPRC:
	{
		boucle_crs.invalidate();
	}
	break;

	default:
		break;
	}

	// prepare new operations
	switch (new_mode) {
	case eBoucleGlobalModesCRS:
	{
		boucle_crs.invalidate();
	}
	break;
	case eBoucleGlobalModesFEC:
	{
		boucle_fec.invalidate();
	}
	break;
	case eBoucleGlobalModesPRC:
	{
		boucle_crs.invalidate();
	}
	break;

	case eBoucleGlobalModesInit:
	default:
		break;
	}

	m_global_mode = new_mode;
}
