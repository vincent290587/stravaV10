/*
 * BoucleZwift.h
 *
 *  Created on: 4 mars 2020
 *      Author: vgol
 */

#ifndef SOURCE_MODEL_BOUCLEZWIFT_H_
#define SOURCE_MODEL_BOUCLEZWIFT_H_

#include "BoucleCRS.h"
#include "GPSMGMT.h"

class BoucleZwift : private BoucleCRS, virtual public BoucleInterface {
public:
	BoucleZwift() : BoucleCRS() {
	};
	virtual ~BoucleZwift() {

	};

	void init_internal(void) {

		// turn GPS OFF
		extern GPS_MGMT gps_mgmt;
		gps_mgmt.standby();

		m_dist_next_seg = 9999;
	};

	void run_internal(void) {
		BoucleCRS::run_internal();
	};

	void invalidate_internal(void) {
		BoucleCRS::invalidate_internal();
	};

//	void loadPRC() {
//
//	};
//
//	void parcoursSelect(int prc_ind) {
//
//	};

};


#endif /* SOURCE_MODEL_BOUCLEZWIFT_H_ */
