/*
 * segger_wrapper.c
 *
 *  Created on: 5 oct. 2017
 *      Author: Vincent
 */


#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if USE_SVIEW
#include "SEGGER_SYSVIEW.h"
static SEGGER_SYSVIEW_TASKINFO pInfo[SYSVIEW_MAX_NOF_TASKS];
static uint32_t nb_tasks;

SEGGER_SYSVIEW_OS_API os_api;

SEGGER_SYSVIEW_MODULE m_module = {
		"M=stravaV10",
		30,
		0,
};

#endif

#if USE_SVIEW

#define W_SYSVIEW_OnIdle(...)            SEGGER_SYSVIEW_OnIdle()

#define W_SYSVIEW_OnTaskStartExec(X)     SEGGER_SYSVIEW_OnTaskStartExec(X)
#define W_SYSVIEW_OnTaskStopExec(X)      SEGGER_SYSVIEW_OnTaskStopExec()
#define W_SYSVIEW_OnTaskStartReady(X)    SEGGER_SYSVIEW_OnTaskStartReady(X)
#define W_SYSVIEW_OnTaskStopReady(X, M)  SEGGER_SYSVIEW_OnTaskStopReady(X, M)

#define W_SYSVIEW_RecordVoid(X)          SEGGER_SYSVIEW_RecordVoid(X)
#define W_SYSVIEW_RecordEndCall(X)       SEGGER_SYSVIEW_RecordEndCall(X)

#define W_SYSVIEW_RecordU32x2(...)       SEGGER_SYSVIEW_RecordU32x2(__VA_ARGS__)

#else

#define W_SYSVIEW_OnIdle(...)            EMPTY_MACRO
#define W_SYSVIEW_OnTaskStartExec(X)     EMPTY_MACRO
#define W_SYSVIEW_OnTaskStopExec(X)      EMPTY_MACRO
#define W_SYSVIEW_OnTaskCreate(X)        EMPTY_MACRO
#define W_SYSVIEW_OnTaskStartReady(X)    EMPTY_MACRO
#define W_SYSVIEW_OnTaskStopReady(X, M)  EMPTY_MACRO

#define W_SYSVIEW_RecordVoid(X)          EMPTY_MACRO
#define W_SYSVIEW_RecordEndCall(X)       EMPTY_MACRO

#define W_SYSVIEW_RecordU32x2(...)       EMPTY_MACRO

#endif

static uint32_t m_cur_task_id;
static uint32_t m_cur_void_id;

/*******************************************************************************
 * Functions
 ******************************************************************************/
static void cbSendTaskList(void) {

#if USE_SVIEW

	for (int i = 0; i < nb_tasks; i++) {

		SEGGER_SYSVIEW_SendTaskInfo(&pInfo[i]);

	}

#endif

}

void sysview_task_block(uint32_t evt_mask) {
	W_SYSVIEW_OnTaskStopReady(sysview_id_get(), evt_mask);
	W_SYSVIEW_OnTaskStopExec(sysview_id_get());
}

void sysview_task_transfer(uint32_t task_id) {
	W_SYSVIEW_OnTaskStartExec(task_id);
	m_cur_task_id = task_id;
}

void sysview_task_event(uint32_t task_id, uint32_t event_mask) {
	W_SYSVIEW_RecordU32x2(TASK_RECV_EVENT, task_id, event_mask);
}

void sysview_task_void_enter(uint32_t void_id) {
	m_cur_void_id = void_id;
	W_SYSVIEW_RecordVoid(m_cur_void_id);
}

void sysview_task_void_exit(uint32_t void_id) {
	W_SYSVIEW_RecordEndCall(void_id);
	m_cur_void_id = 0;
}

void sysview_task_idle(void) {
	if (m_cur_task_id != 0) {
		m_cur_task_id = 0;
		W_SYSVIEW_OnIdle();
	}
}


void segger_init(void) {

//#if USE_RTT && !USE_SVIEW
//	  // RTT
//	  SEGGER_RTT_Init();
//#endif

#if USE_SVIEW

#warning "SysView is active"

	  // SYSVIEW
	  os_api.pfSendTaskList = cbSendTaskList;
//	  os_api.pfGetTime = (U64)millis();
	  os_api.pfGetTime = 0;

	  nb_tasks = 0;

	  pInfo[nb_tasks].TaskID = BOUCLE_TASK;
	  pInfo[nb_tasks++].sName  = "BOUCLE_TASK";

	  pInfo[nb_tasks].TaskID = PERIPH_TASK;
	  pInfo[nb_tasks++].sName  = "PERIPH_TASK";

	  pInfo[nb_tasks].TaskID = LCD_TASK;
	  pInfo[nb_tasks++].sName  = "LCD_TASK";

	  SEGGER_SYSVIEW_Conf();

	  SEGGER_SYSVIEW_Start();

	  for (int i = 0; i < nb_tasks; i++) {

		  SEGGER_SYSVIEW_OnTaskCreate(pInfo[i].TaskID);

	  }

	  SEGGER_SYSVIEW_RegisterModule(&m_module);

#endif

}
