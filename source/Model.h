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
#include "g_structs.h"

typedef struct {
	uint8_t peripherals_id;
	uint8_t boucle_id;
	uint8_t system_id;
	uint8_t ls027_id;
} sTasksIDs;

extern sTasksIDs m_tasks_id;

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
#include "AltiBaro.h"
#include "Attitude.h"
#include "VParser.h"
#include "SufferScore.h"
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

extern AltiBaro      baro;

extern GPS_MGMT      gps_mgmt;

extern VParser       vparser;

extern sFecControl   fec_control;

extern sBacklightOrders     backlight;

extern sNeopixelOrders      neopixel;

extern sAppErrorDescr m_app_error;

extern TinyGPSPlus   gps;

extern SufferScore   suffer_score;

extern "C" {
#endif // defined C++

void __aeabi_idiv0(void);

void model_go_to_msc_mode(void);

void model_dispatch_sensors_update(void);

void model_dispatch_lns_update(sLnsInfo *lns_info);

void model_get_navigation(sKomootNavigation *nav);

void perform_system_tasks(void);

void perform_system_tasks_light(void);

bool check_memory_exception(void);

void wdt_reload(void);

void bsp_tasks(void);

void backlighting_tasks(void);

void idle_task(void * p_context);

void boucle_task(void * p_context);

void peripherals_task(void * p_context);

void system_task(void * p_context);

void ls027_task(void * p_context);

#if defined(__cplusplus)
}
#endif // defined C++

#else /* TDD */

#include "Model_tdd.h"

#endif /* TDD */

#endif /* SOURCE_MODEL_H_ */
