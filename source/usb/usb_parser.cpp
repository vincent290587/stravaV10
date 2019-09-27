/*
 * usb_parser.cpp
 *
 *  Created on: 4 juil. 2018
 *      Author: Vincent
 */

#include "segger_wrapper.h"
#include "usb_parser.h"
#include "usb_cdc.h"
#include "sd_hal.h"
#include "Model.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "nrf_pwr_mgmt.h"
#ifdef __cplusplus
}
#endif

/**
 * Decodes chars from the VCOM line
 * @param c
 */
void usb_cdc_decoder(char c) {

	NRF_LOG_RAW_INFO("%c", c);
	NRF_LOG_FLUSH();

	// TODO
	switch (vparser.encode(c)) {
	case _SENTENCE_LOC:

		locator.sim_loc.data.lat = (float)vparser.getLat() / 10000000.f;
		locator.sim_loc.data.lon = (float)vparser.getLon() / 10000000.f;
		locator.sim_loc.data.alt = (float)vparser.getEle();
		locator.sim_loc.data.utc_time = vparser.getSecJ();

		locator.sim_loc.setIsUpdated();

		// notify task
		if (m_tasks_id.boucle_id != TASK_ID_INVALID) {
		    w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_LOCATION);
		}

		break;

	case _SENTENCE_PC:

		if (vparser.getPC() == 17) {
			nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_GOTO_DFU);
		}
		else if (vparser.getPC() == 16) {
			usb_cdc_start_msc();
		}
		else if (vparser.getPC() == 15) {
			format_memory();
		}
		else if (vparser.getPC() == 14) {
			fmkfs_memory();
		}
		else if (vparser.getPC() == 13) {
			test_memory();
		}

		break;
	default:
		break;

	}

}
