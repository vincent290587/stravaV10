/*
 * Boucle.cpp
 *
 *  Created on: 19 oct. 2017
 *      Author: Vincent
 */


#include "BoucleCRS.h"
#include "Segment.h"
#include "Model.h"
#include "uart0.h"
#include "nrf52.h"
#include "sd_functions.h"
#include "segger_wrapper.h"
#include "spi_scheduler.h"


/**
 *
 */
BoucleCRS::BoucleCRS() : BoucleInterface() {
	m_dist_next_seg = 5000;
}

/**
 *
 * @return true if we are ready to run
 */
bool BoucleCRS::isTime() {

	if (locator.isUpdated()) {
		LOG_INFO("Locator updated\r\n");
		return true;
	}

	if (m_needs_init ||
			m_last_refresh.getAge() > 1200) {
		LOG_INFO("Auto refresh\r\n");
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

	memset(&att, 0, sizeof(SAtt));

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

	pwManager.switchToRun24();

	if (m_needs_init) this->init();

	nrf52_refresh();

	dma_spi0_mngr_tasks_start();
	dma_spi0_mngr_finish();

	pwManager.switchToRun120();

	if (locator.isUpdated()) {

		LOG_INFO("Locator is updated (%u)\r\n", millis());

		att.nbact = 0;

		// update position
		SLoc loc;
		SDate dat;
		memset(&loc, 0, sizeof(loc));
		memset(&dat, 0, sizeof(dat));

		locator.getPosition(loc, dat);

		attitude.addNewLocation(loc, dat);

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
						vue.addSegmentPrio(&seg);
					} else if (seg.getStatus() > SEG_OFF) {
						vue.addSegment(&seg);
					}

					if (seg.getStatus() == SEG_FIN) {

						LOG_INFO("Segment FIN %s\r\n", seg.getName());

						if (seg.getAvance() > 0.) {
							ua_send_notification_blue(2);
						} else {
							ua_send_notification_red (2);
						}

					}

				} else if (tmp_dist < 250) {

					W_SYSVIEW_OnTaskStartExec(SEG_PERF_TASK);
					seg.majPerformance(mes_points);
					W_SYSVIEW_OnTaskStopExec(SEG_PERF_TASK);

					vue.addSegment(&seg);

				}

			} // fin isValid

		} // fin for

		att.next = m_dist_next_seg;

		LOG_INFO("Next segment: %u\r\n", att.next);
	} else {
		// update date
		SDate dat;
		memset(&dat, 0, sizeof(dat));
		locator.getDate(dat);

		attitude.addNewDate(dat);
	}

	pwManager.switchToRun24();

	W_SYSVIEW_OnTaskStartExec(LCD_TASK);
	vue.refresh();
	m_last_refresh.setUpdateTime();
	W_SYSVIEW_OnTaskStopExec(LCD_TASK);

	dma_spi0_mngr_tasks_start();
	dma_spi0_mngr_finish();

	//	sdisplay.displayRTT();

	// go back to low power
	pwManager.switchToVlpr();

	W_SYSVIEW_OnIdle();
}
