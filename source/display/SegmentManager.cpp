/*
 * SegmentManager.cpp
 *
 *  Created on: 19 févr. 2018
 *      Author: Vincent
 */

#include <SegmentManager.h>

SegmentManager::SegmentManager() {

	memset(&m_segs, 0, sizeof(m_segs));

}


void SegmentManager::addSegment(Segment *p_seg) {

	if (m_segs.nb_segs < NB_SEG_ON_DISPLAY) {
		m_segs.s_segs[m_segs.nb_segs].p_seg   = p_seg;
		m_segs.s_segs[m_segs.nb_segs].is_prio = 0;
		m_segs.nb_segs++;
	}

}


void SegmentManager::addSegmentPrio(Segment *p_seg) {

	if (m_segs.nb_segs < NB_SEG_ON_DISPLAY) {
		m_segs.s_segs[m_segs.nb_segs].p_seg   = p_seg;
		m_segs.s_segs[m_segs.nb_segs].is_prio = 1;
		m_segs.nb_segs++;
	} else {

		for (uint8_t i=0; i < NB_SEG_ON_DISPLAY; i++) {
			if (!m_segs.s_segs[i].is_prio) {
				m_segs.s_segs[i].p_seg   = p_seg;
				m_segs.s_segs[i].is_prio = 1;
				return;
			}
		}

	}
}
