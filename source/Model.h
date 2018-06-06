/*
 * Global.h
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_H_
#define SOURCE_MODEL_H_

#if defined(__cplusplus)

#include "Sensor.h"
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
#include "parameters.h"
#include "mk64f_parser.h"


extern SAtt          att;

extern Attitude      attitude;

extern ListeSegments mes_segments;

extern ListePoints   mes_points;

extern ListeParcours mes_parcours;

extern ListePoints   mes_points;

extern Locator       locator;

extern Boucle        boucle;

extern SegmentManager     segMngr;

extern Vue           vue;

extern STC3100       stc;

extern VEML6075      veml;

extern MS5637        ms5637;

extern GPS_MGMT      gps_mgmt;

extern sFecControl   fec_control;

extern Sensor<sCadenceData> cad;

extern Sensor<sHrmInfo>     hrm;

extern Sensor<sFecInfo>     fec_info;

extern "C" void model_dispatch_sensors_update(void);

extern "C" void perform_system_tasks(void);

extern "C" bool check_memory_exception(void);

#endif

#endif /* SOURCE_MODEL_H_ */
