/*
 * segger_wrapper.c
 *
 *  Created on: 5 oct. 2017
 *      Author: Vincent
 */

#include "Attitude.h"
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

#define W_SYSVIEW_OnIdle(...)            EMPTY_MACRO

#define W_SYSVIEW_OnTaskStartExec(X)     SEGGER_SYSVIEW_OnTaskStartExec(X)
#define W_SYSVIEW_OnTaskStopExec(X)      SEGGER_SYSVIEW_OnTaskStopExec()
#define W_SYSVIEW_OnTaskStartReady(X)    SEGGER_SYSVIEW_OnTaskStartReady(X)
#define W_SYSVIEW_OnTaskStopReady(X, M)  SEGGER_SYSVIEW_OnTaskStopReady(X, M)

#define W_SYSVIEW_RecordVoid(X)          SEGGER_SYSVIEW_RecordVoid(X)
#define W_SYSVIEW_RecordEndCall(X)       SEGGER_SYSVIEW_RecordEndCall(X)

#define W_SYSVIEW_RecordU32(...)         SEGGER_SYSVIEW_RecordU32(__VA_ARGS__)
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

#define W_SYSVIEW_RecordU32(...)         EMPTY_MACRO
#define W_SYSVIEW_RecordU32x2(...)       EMPTY_MACRO

#endif

static uint32_t m_cur_task_id;
static uint32_t m_cur_void_id;

extern sAppErrorDescr m_app_error;

/*******************************************************************************
 * Functions
 ******************************************************************************/

#if USE_SVIEW
static void cbSendTaskList(void) {

	task_manager_get_tasks_desc(pInfo, &nb_tasks);

	for (int i = 0; i < nb_tasks; i++) {

		SEGGER_SYSVIEW_SendTaskInfo(&pInfo[i]);

	}

}
#endif

void sysview_task_block(uint32_t evt_mask) {
	W_SYSVIEW_OnTaskStopReady(SYSVIEW_ID_GET(), evt_mask);
	W_SYSVIEW_OnTaskStopExec(SYSVIEW_ID_GET());
}

void sysview_task_unblock(uint32_t task_id) {
	W_SYSVIEW_OnTaskStartReady(task_id);
}

void sysview_task_transfer(uint32_t task_id) {
	if (task_id) W_SYSVIEW_OnTaskStartExec(task_id);
	else W_SYSVIEW_OnIdle();
	m_cur_task_id = task_id;
	m_app_error.task_id = task_id;
}

void sysview_task_event(uint32_t task_id, uint32_t event_mask) {
	W_SYSVIEW_RecordU32x2(TASK_RECV_EVENT, task_id, event_mask);
}

void sysview_task_void_enter(uint32_t void_id) {
	m_cur_void_id = void_id;
	m_app_error.void_id = void_id;
	W_SYSVIEW_RecordVoid(m_cur_void_id);
}

void sysview_task_u32_enter(uint32_t void_id, uint32_t data) {
	m_cur_void_id = void_id;
	m_app_error.void_id = void_id;
	W_SYSVIEW_RecordU32(m_cur_void_id, data);
}

void sysview_task_void_exit(uint32_t void_id) {
	W_SYSVIEW_RecordEndCall(void_id);
	m_cur_void_id = 0;
	m_app_error.void_id = 0;
}

void sysview_task_idle(void) {
	if (m_cur_task_id != 0) {
		m_cur_task_id = 0;
		m_app_error.task_id = 0;
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

	  task_manager_get_tasks_desc(pInfo, &nb_tasks);

	  for (int i = 0; i < nb_tasks; i++) {

		  SEGGER_SYSVIEW_SendTaskInfo(&pInfo[i]);

	  }

	  SEGGER_SYSVIEW_Conf();

	  SEGGER_SYSVIEW_Start();

	  SEGGER_SYSVIEW_RegisterModule(&m_module);

#endif

}
