/*
 * SegmentManager.cpp
 *
 *  Created on: 19 févr. 2018
 *      Author: Vincent
 */

#include <algorithm>
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
