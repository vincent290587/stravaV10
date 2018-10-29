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

	m_needs_init = false;

	m_last_refresh = 0;

	m_dist_next_seg = 9999;
}

/**
 *
 */
void BoucleCRS::run() {

	m_dist_next_seg = 9999;
	float tmp_dist;

	if (m_needs_init) this->init();

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

	attitude.addNewLocation(loc, dat, loc_source);

	// update sements
	for (auto& seg : mes_segments._segs) {

		if (seg.isValid() && mes_points.size() > 2) {

			tmp_dist = segment_allocator(seg, att.loc.lat, att.loc.lon);

			// calculate distance to closest segment
			if (tmp_dist < m_dist_next_seg) m_dist_next_seg = tmp_dist;

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

	// wait for location to be updated
	(void)task_events_wait(TASK_EVENT_LOCATION);

}
