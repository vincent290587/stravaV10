/*
 * SegmentManager.h
 *
 *  Created on: 19 févr. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_DISPLAY_SEGMENTMANAGER_H_
#define SOURCE_DISPLAY_SEGMENTMANAGER_H_

#include <vector>
#include "parameters.h"
#include "Segment.h"

typedef struct {
	int  score;
	Segment* p_seg;
} sVueCRSPSeg;

class SegmentManager {
public:
	SegmentManager();

	void addSegment(Segment&);

	uint8_t getNbSegs() {
		return seg_list.size();
	}

	void computeOrder(void);

	sVueCRSPSeg* getSeg(size_t i) {
		if (i < 0 ||  i > seg_list.size() + 1) return nullptr;
		return &seg_list[i];
	}

	void clearSegs() {
		seg_list.clear();
	}

	std::vector<sVueCRSPSeg> seg_list;
};

#endif /* SOURCE_DISPLAY_SEGMENTMANAGER_H_ */
