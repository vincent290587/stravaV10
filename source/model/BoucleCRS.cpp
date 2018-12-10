/*
 * Boucle.cpp
 *
 *  Created on: 19 oct. 2017
 *      Author: Vincent
 */


#include "BoucleCRS.h"
#include "Segment.h"
#include "Model.h"
#include "uart.h"
#include "sd_functions.h"
#include "segger_wrapper.h"

/**
 *
 */
BoucleCRS::BoucleCRS() : BoucleInterface() {
	m_dist_next_seg = 5000;

	m_s_parcours = nullptr;

	memset(&att, 0, sizeof(SAtt));
}

/**
 *
 * @return true if we are ready to run
 */
bool BoucleCRS::isTime() {

	if (locator.isUpdated()) {
		LOG_INFO("Locator updated");
		return true;
	}

	if (m_needs_init ||
			m_last_refresh.getAge() > 1200) {
		LOG_INFO("Auto refresh %u", millis());
		return true;
	}

	return false;
}

/**
 *
 */
void BoucleCRS::init() {

	// turn GPS ON
	gps_mgmt.awake();
 	gps_mgmt.startEpoUpdate();

	m_last_refresh = 0;

	m_dist_next_seg = 9999;

	if (m_app_error.special == 0xDB) {

		m_app_error.special = 0x00;
		// restoring position and stuff
		memcpy(&att, &m_app_error.saved_att, sizeof(SAtt));

		LOG_WARNING("Last stored date: %u", m_app_error.saved_att.date.date);
	}

	if (m_s_parcours) this->loadPRC();

	m_needs_init = false;
}

/**
 *
 */
void BoucleCRS::run() {

	m_dist_next_seg = 9999;
	float tmp_dist;

	if (m_needs_init) this->init();

	// wait for location to be updated
	(void)events_wait(TASK_EVENT_LOCATION);

	LOG_INFO("Locator is updated (%u)\r\n", millis());

	// reset the segment manager
	segMngr.clearSegs();

	att.nbact = 0;

	// update position
	SLoc loc;
	SDate dat;
	memset(&loc, 0, sizeof(loc));
	memset(&dat, 0, sizeof(dat));

	eLocationSource loc_source = locator.getPosition(loc, dat);

	if (eLocationSourceNone == loc_source) return;

	attitude.addNewLocation(loc, dat, loc_source);

	// update sements
	for (auto& seg : mes_segments._segs) {

		if (seg.isValid()) {

			tmp_dist = segment_allocator(seg, att.loc.lat, att.loc.lon);

			// calculate distance to closest segment
			if (tmp_dist > 0. &&
					tmp_dist < m_dist_next_seg) m_dist_next_seg = tmp_dist;

			// we don't possess enough points to continue calculating...
			if (mes_points.size() < 2) continue;

			if (seg.getStatus() != SEG_OFF) {

				W_SYSVIEW_OnTaskStartExec(SEG_PERF_TASK);
				seg.majPerformance(mes_points);
				W_SYSVIEW_OnTaskStopExec(SEG_PERF_TASK);
				att.nbact += 1;

				if (seg.getStatus() < SEG_OFF) {
					segMngr.addSegmentPrio(&seg);
				} else if (seg.getStatus() > SEG_OFF) {
					segMngr.addSegment(&seg);
				}

				if (seg.getStatus() == SEG_FIN) {

					LOG_INFO("Segment FIN %s\r\n", seg.getName());

					// show some light !
					if (seg.getAvance() > 0.) {
						SET_NEO_EVENT_BLUE(neopixel, eNeoEventWeakNotify, 2);
					} else {
						SET_NEO_EVENT_RED(neopixel, eNeoEventWeakNotify, 2);
					}

				}

			} else if (tmp_dist < 250) {

				W_SYSVIEW_OnTaskStartExec(SEG_PERF_TASK);
				seg.majPerformance(mes_points);
				W_SYSVIEW_OnTaskStopExec(SEG_PERF_TASK);

				segMngr.addSegment(&seg);

			}

		} // fin isValid

	} // fin for

	att.next = m_dist_next_seg;

	LOG_INFO("Next segment: %u", att.next);

	notifications_setNotify(&neopixel);

	vue.refresh();

	m_last_refresh.setUpdateTime();

}

/**
 *
 */
void BoucleCRS::loadPRC() {

	if (!m_s_parcours) return;

	if (load_parcours(m_s_parcours[0]) > 0) {
		vue.addNotif("PRC: ", "Success !", 4, eNotificationTypeComplete);
	} else {
		vue.addNotif("PRC: ", "Loading failed", 4, eNotificationTypeComplete);
	}

}

/**
 *
 */
void BoucleCRS::parcoursSelect(int prc_ind) {

	LOG_INFO("Selection PRC %d", prc_ind);
	m_s_parcours = mes_parcours.getParcoursAt(prc_ind-1);
}

/**
 *
 */
void BoucleCRS::invalidate(void) {
	BoucleInterface::invalidate();
	if (m_s_parcours) m_s_parcours->desallouerPoints();
	m_s_parcours = nullptr;
}
