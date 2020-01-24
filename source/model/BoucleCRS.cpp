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

#ifdef TDD
#include "tdd_logger.h"
#endif

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

	if (m_s_parcours) this->loadPRC();

	m_needs_init = false;
}

/**
 *
 */
void BoucleCRS::run() {

	m_dist_next_seg = 9999;
	float tmp_dist = 0.0f;

	if (m_needs_init) this->init();

	// wait for location to be updated
	(void)w_task_events_wait(TASK_EVENT_LOCATION);

	LOG_INFO("\r\nLocator is updated (%u)", millis());

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

	// update segments

	sysview_task_void_enter(MainSegLoop);
	for (auto& seg : mes_segments._segs) {

		if (seg.isValid()) {

			tmp_dist = segment_allocator(seg, att.loc.lat, att.loc.lon);

			// calculate distance to closest segment
			if (tmp_dist > 0.f &&
					tmp_dist < m_dist_next_seg) m_dist_next_seg = (uint16_t)tmp_dist;

			// we don't possess enough points to continue calculating...
			if (mes_points.size() < 2) {
				LOG_INFO("Not enough points: %d", mes_points.size());
				break;
			}

			if (seg.getStatus() != SEG_OFF) {

				sysview_task_void_enter(ComputeSegmentPerf);
				seg.majPerformance(mes_points);
				sysview_task_void_exit(ComputeSegmentPerf);
				att.nbact += 1;

				if (seg.getStatus() == SEG_FIN) {

					LOG_INFO("Segment FIN %s\r\n", seg.getName());

					// show some light !
					if (seg.getAvance() > 0.f) {
						SET_NEO_EVENT_BLUE(neopixel, eNeoEventWeakNotify, 2);
					} else {
						SET_NEO_EVENT_RED(neopixel, eNeoEventWeakNotify, 2);
					}

				}

			} else if (tmp_dist < 250) {

				sysview_task_void_enter(ComputeSegmentPerf);
				seg.majPerformance(mes_points);
				sysview_task_void_exit(ComputeSegmentPerf);

			}

			if (seg.getStatus() == SEG_OFF && tmp_dist > DIST_ALLOC) {
				// we don't want to save this, it will be deallocated in the next loop,
				// and is of no further interest
				continue;
			}

			segMngr.addSegment(seg);

		} // fin isValid
		else {
			LOG_INFO("Segment not valid...");
		}

	} // fin for
	sysview_task_void_exit(MainSegLoop);

	segMngr.computeOrder();

	segMngr.conveyLightNotification();

	att.next = m_dist_next_seg;

	LOG_INFO("Next segment: %u", att.next);

#ifdef TDD
	tdd_logger_log_int(TDD_LOGGING_SEG_DIST  , att.next);
	tdd_logger_log_int(TDD_LOGGING_NB_SEG_ACT, att.nbact);
#endif

	notifications_setNotify(&neopixel);

	// ready for displaying
	if (m_tasks_id.ls027_id != TASK_ID_INVALID) {
		w_task_delay_cancel(m_tasks_id.ls027_id);
	}

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
