/*
 * Global.h
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_TDD_H_
#define SOURCE_MODEL_TDD_H_

#include <stdbool.h>
#include "notifications.h"
#include "parameters.h"
#include "task_manager_wrapper_tdd.h"

typedef struct {
	task_id_t peripherals_id;
	task_id_t boucle_id;
	task_id_t system_id;
	task_id_t ls027_id;
} sTasksIDs;

extern sTasksIDs m_tasks_id;

#if defined(__cplusplus)

#include "ListePoints.h"
#include "Segment.h"
#include "Parcours.h"
#include "Locator.h"
#include "SegmentManager.h"
#include "Boucle.h"
#include "STC3100.h"
#include "AltiBaro.h"
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

extern sAppErrorDescr m_app_error;

extern AltiBaro      baro;

extern sHrmInfo hrm_info;
extern sBscInfo bsc_info;
extern sFecInfo fec_info;

extern "C" {
#endif // defined C++

void model_go_to_msc_mode(void);

void model_dispatch_sensors_update(void);

void model_dispatch_lns_update(sLnsInfo *lns_info);

void perform_system_tasks(void);

void perform_system_tasks_light(void);

bool check_memory_exception(void);

void wdt_reload(void);

void print_mem_state(void);

void backlighting_tasks(void);

void idle_task(void * p_context);

void boucle_task(void * p_context);

void peripherals_task(void * p_context);

void system_task(void * p_context);

void ls027_task(void * p_context);


#if defined(__cplusplus)
}
#endif // defined C++

#endif /* SOURCE_MODEL_H_ */
