/*
 * SegmentManager.h
 *
 *  Created on: 19 févr. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_DISPLAY_SEGMENTMANAGER_H_
#define SOURCE_DISPLAY_SEGMENTMANAGER_H_

#include "parameters.h"
#include "Segment.h"

typedef struct {
	uint8_t  is_prio;
	Segment* p_seg;
} sVueCRSPSeg;

typedef struct {
	uint8_t     nb_segs;
	sVueCRSPSeg s_segs[NB_SEG_ON_DISPLAY];
} sVueCRSSegArray;

class SegmentManager {
public:
	SegmentManager();

	void addSegment(Segment*);
	void addSegmentPrio(Segment*);

	uint8_t getNbSegs() {
		return m_segs.nb_segs;
	}

	sVueCRSPSeg* getSeg(uint8_t i) {
		return &m_segs.s_segs[i % NB_SEG_ON_DISPLAY];
	}

	void clearSegs() {
		memset(&m_segs, 0, sizeof(m_segs));
	}

private:
	sVueCRSSegArray    m_segs;
};

#endif /* SOURCE_DISPLAY_SEGMENTMANAGER_H_ */
