/*
 * Global.h
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_H_
#define SOURCE_MODEL_H_

#ifndef TDD

#include <stdbool.h>
#include "notifications.h"
#include "parameters.h"

#if defined(__cplusplus)

#include "fxos.h"
#include "ListePoints.h"
#include "Segment.h"
#include "Parcours.h"
#include "Locator.h"
#include "Cadence.h"
#include "Boucle.h"
#include "Vue.h"
#include "GPSMGMT.h"
#include "TinyGPS++.h"
#include "STC3100.h"
#include "VEML6075.h"
#include "MS5637.h"
#include "Attitude.h"
#include "VParser.h"
#include "mk64f_parser.h"


extern SAtt          att;

extern Attitude      attitude;

extern ListeSegments mes_segments;

extern ListePoints   mes_points;

extern ListeParcours mes_parcours;

extern Locator       locator;

extern Boucle        boucle;

extern SegmentManager     segMngr;

extern Vue           vue;

extern STC3100       stc;

extern VEML6075      veml;

extern MS5637        ms5637;

extern GPS_MGMT      gps_mgmt;

extern VParser       vparser;

extern sFecControl   fec_control;

extern sBacklightOrders     backlight;

extern sNeopixelOrders      neopixel;

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

#else /* TDD */

#include "Model_tdd.h"

#endif /* TDD */

#endif /* SOURCE_MODEL_H_ */
