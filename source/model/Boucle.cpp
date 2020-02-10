/*
 * Boucle.cpp
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#include <Boucle.h>
#include "Model.h"
#include "segger_wrapper.h"
#include <sd/sd_functions.h>

BoucleCRS boucle_crs;

BoucleFEC boucle_fec;

static eBoucleGlobalModes m_global_mode = eBoucleGlobalModesInit;


void boucle__init(void) {

	LOG_INFO("Boucle init...");

	if (m_app_error.special == SYSTEM_DESCR_POS_CRC) {
		String message = "Last void executed: ";
		message += m_app_error.void_id;
		message += " in task: ";
		message += m_app_error.task_id;
		LOG_ERROR(message.c_str());
	    vue.addNotif("System", message.c_str(), 6, eNotificationTypeComplete);
	}
	if (m_app_error.err_desc.crc == SYSTEM_DESCR_POS_CRC) {
		LOG_ERROR("Error identified:");
		String message = m_app_error.err_desc._buffer;
		message += " in void ";
		message += m_app_error.void_id;
		LOG_ERROR(message.c_str());
	    vue.addNotif("Error", message.c_str(), 6, eNotificationTypeComplete);
	    memset(&m_app_error.err_desc, 0, sizeof(m_app_error.err_desc));
	}

	int ret;
	if ((ret = init_liste_segments())) {
		LOG_ERROR("Boucle init fail %d", ret);
	}

	m_global_mode = BOUCLE_DEFAULT_MODE;

	// prepare for next reboot
	m_app_error.special = SYSTEM_DESCR_POS_CRC;
}

void boucle__uninit(void) {

	LOG_INFO("Boucle uninit");

	uninit_liste_segments();

	m_global_mode = eBoucleGlobalModesMSC;
}

void boucle__run(void) {

	switch (m_global_mode) {
	case eBoucleGlobalModesInit:
	{
		boucle__init();
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
	{
		(void)w_task_delay(10000);
	} break;
	}

}

void boucle__change_mode(eBoucleGlobalModes new_mode) {

	if (new_mode == m_global_mode) return;

	// Unblock task
	if (m_tasks_id.boucle_id != TASK_ID_INVALID) {
		w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_BOUCLE_RELEASE);
	}

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
	stc.resetCharge();

	m_global_mode = new_mode;
}

eBoucleGlobalModes boucle__get_mode(void) {

	return m_global_mode;
}

