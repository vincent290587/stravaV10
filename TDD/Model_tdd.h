/*
 * Global.h
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_TDD_H_
#define SOURCE_MODEL_TDD_H_


#include "notifications.h"
#include "parameters.h"

#if defined(__cplusplus)

#include "ListePoints.h"
#include "Segment.h"
#include "Parcours.h"
#include "Locator.h"
#include "SegmentManager.h"
#include "Boucle.h"
#include "STC3100.h"
#include "GPSMGMT.h"
#include "Attitude.h"
#include "mk64f_parser.h"
#include "Vue.h"


extern SAtt          att;

extern Attitude      attitude;

extern ListeSegments mes_segments;

extern ListePoints   mes_points;

extern ListeParcours mes_parcours;

extern Locator       locator;

extern Boucle        boucle;

extern Vue           vue;

extern SegmentManager     segMngr;

extern GPS_MGMT      gps_mgmt;

extern sFecControl   fec_control;

extern sBacklightOrders     backlight;

extern sNeopixelOrders      neopixel;

extern STC3100              stc;

extern sHrmInfo hrm_info;
extern sBscInfo bsc_info;

extern "C" {
#endif // defined C++

void model_go_to_msc_mode(void);

void model_dispatch_sensors_update(void);

void model_dispatch_lns_update(sLnsInfo *lns_info);

void perform_system_tasks(void);

void perform_system_tasks_light(void);

bool check_memory_exception(void);

void wdt_reload(void);

#if defined(__cplusplus)
}
#endif // defined C++

#endif /* SOURCE_MODEL_H_ */
