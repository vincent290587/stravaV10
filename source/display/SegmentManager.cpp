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

	if (seg.getScore() >= 0) {
		sVueCRSPSeg p_seg;
		p_seg.score = seg.getScore();
		p_seg.p_seg = &seg;

		seg_list.push_back(p_seg);
	}
}

void SegmentManager::computeOrder(void) {

	if (seg_list.size() > 1) sort(seg_list.begin(), seg_list.end(), _compareScores);

}
