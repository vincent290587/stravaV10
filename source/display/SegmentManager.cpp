/*
 * SegmentManager.cpp
 *
 *  Created on: 19 févr. 2018
 *      Author: Vincent
 */

#include <algorithm>
#include <cmath>
#include "utils.h"
#include "notifications.h"
#include "SegmentManager.h"
#include "segger_wrapper.h"

static bool _compareScores(const sVueCRSPSeg& i1, const sVueCRSPSeg& i2)
{
    return (i1.score > i2.score);
}

SegmentManager::SegmentManager() {
}

void SegmentManager::addSegment(Segment &seg) {

	int score = seg.getScore();
	if (score >= 0) {
		sVueCRSPSeg p_seg;
		p_seg.score = score;
		p_seg.p_seg = &seg;

		LOG_DEBUG("Adding seg %s with score %u",seg.getName() , score);

		seg_list.push_back(p_seg);
	}
}

void SegmentManager::computeOrder(void) {

	if (seg_list.size() > 1) std::sort(seg_list.begin(), seg_list.end(), _compareScores);

}

void SegmentManager::conveyLightNotification(void) {

	neo_sb_seg_params orders;

	if (seg_list.size() < 1) {

		// clear it all: no light
		memset(&orders, 0, sizeof(orders));

	} else {
		float off_time;
		float indice_pc;
		float rtime, curtime;
		Segment* p_seg = this->getSeg(0)->p_seg;

		ASSERT(p_seg);

		rtime   = p_seg->getAvance();
		curtime = p_seg->getCur();

		if (curtime < 7.f) {
			indice_pc = 100.f * rtime / 7.f;
		} else {
			indice_pc = 100.f * rtime / curtime;
		}

		orders.active = 1;
		orders.on_time = 1;

		if (indice_pc < 0.f) {
			off_time = regFen(indice_pc, -10.f,  0.f,  12.5f, 1.5f);
			orders.rgb[0] = 0x30;
			orders.rgb[1] = 0x00;
			orders.rgb[2] = 0x00;
		} else {
			off_time = regFen(indice_pc,   0.f, 10.f,   1.5f, 12.5f);
			orders.rgb[0] = 0x00;
			orders.rgb[1] = 0x00;
			orders.rgb[2] = 0x30;
		}

		//only display if less than 10% from record
		if (fabsf(indice_pc) > 10.f || curtime < 7.f) {
			orders.active = 0;
		} else {
			orders.off_time = (uint32_t)((int32_t)off_time & 0xFF);
		}

	}

	// send to light
	notifications_segNotify(&orders);
}
