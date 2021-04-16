/*
 * Global.h
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_H_
#define SOURCE_MODEL_H_

#include <stdbool.h>
#include "notifications.h"
#include "parameters.h"
#include "g_structs.h"
#include "task_manager_wrapper.h"

typedef struct {
	task_id_t peripherals_id;
	task_id_t boucle_id;
	task_id_t ls027_id;
	task_id_t uart_id;
	task_id_t usb_id;
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
#include "PowerZone.h"
#include "RRZone.h"
#include "UserSettings.h"
#include "mk64f_parser.h"

extern SAtt          att;

extern Attitude      attitude;

extern ListeSegments mes_segments;

extern ListePoints   mes_points;

extern ListeParcours mes_parcours;

extern Locator       locator;

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

extern PowerZone     zPower;

extern RRZone        rrZones;

extern sPowerVector  powerVector;

extern UserSettings   u_settings;

extern "C" {
#endif // defined C++

void model_add_notification(const char *title_, const char *msg_, uint8_t persist_, eNotificationType type_);

void model_input_virtual_uart(char c);

void model_go_to_msc_mode(void);

void model_dispatch_sensors_update(void);

void model_dispatch_lns_update(sLnsInfo *lns_info);

void model_get_navigation(sKomootNavigation *nav);

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


#endif /* SOURCE_MODEL_H_ */
