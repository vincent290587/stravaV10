/*
 * BoucleZwift.h
 *
 *  Created on: 4 mars 2020
 *      Author: vgol
 */

#ifndef SOURCE_MODEL_BOUCLEZWIFT_H_
#define SOURCE_MODEL_BOUCLEZWIFT_H_

#include "BoucleCRS.h"
#include "Attitude.h"
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

		extern Attitude attitude;
		attitude.reset();

		m_dist_next_seg = 9999;
	};

	void run_internal(void) {
		BoucleCRS::run_internal();
	};

	void invalidate_internal(void) {
	};

};


#endif /* SOURCE_MODEL_BOUCLEZWIFT_H_ */
