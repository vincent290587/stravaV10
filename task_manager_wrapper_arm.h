/*
 * task_manager_wrapper_arm.h
 *
 *  Created on: 13 déc. 2018
 *      Author: Vincent
 */

#ifndef TASK_MANAGER_WRAPPER_ARM_H_
#define TASK_MANAGER_WRAPPER_ARM_H_

#include "task_manager.h"


/**@brief Yield CPU to other tasks.
 */
inline void w_task_yield(void) {

	task_yield();

}

inline task_id_t w_task_id_get(void) {
	return task_id_get();
}

/**@brief Wait for events. Set events are cleared after this function returns.
 *
 * @param[in] evt_mask Mask of events to wait
 *
 * @return Mask with set events (can be a subset of evt_mask).
 */
inline uint32_t w_task_events_wait(uint32_t evt_mask) {

	uint32_t mask = task_events_wait(evt_mask);

	return mask;
}

/**@brief Set events for given task.
 *
 * @param[in]  task_id  Id of the task which shall receive events.
 * @param[in]  evt_mask Events for the task.
 *
 */
inline void w_task_events_set(task_id_t task_id, uint32_t evt_mask) {
	if (task_id == TASK_ID_INVALID) return;

	task_events_set(task_id, evt_mask);
}

/**
 *
 * @param del_ Delay to apply to the task
 */
inline uint32_t w_task_delay(uint32_t del_) {

	uint32_t ret = task_delay(del_);

	return ret;
}

/**@brief Cancels the delay of a task
 *
 * @param task_id ID of the task for which to cancel delay
 */
inline void w_task_delay_cancel(task_id_t task_id) {
	if (task_id == TASK_ID_INVALID) return;

	task_delay_cancel(task_id);
}



#if USE_SVIEW
#include "SEGGER_SYSVIEW.h"

#define W_SEGGER_SYSVIEW_OnTaskCreate(X)     SEGGER_SYSVIEW_OnTaskCreate(X)
#define W_SEGGER_SYSVIEW_SendTaskInfo(...)   segger_sendTaskInfo(__VA_ARGS__)

#define W_SYSVIEW_OnIdle(...)            SEGGER_SYSVIEW_OnIdle()

#define W_SYSVIEW_OnTaskStartExec(X)     SEGGER_SYSVIEW_OnTaskStartExec(X)
#define W_SYSVIEW_OnTaskStopExec(X)      EMPTY_MACRO
#define W_SYSVIEW_OnTaskStartReady(X)    SEGGER_SYSVIEW_OnTaskStartReady(X)
#define W_SYSVIEW_OnTaskStopReady(X, M)  SEGGER_SYSVIEW_OnTaskStopReady(X, M)

#define W_SYSVIEW_RecordVoid(X)          SEGGER_SYSVIEW_RecordVoid(X)
#define W_SYSVIEW_RecordEndCall(X)       SEGGER_SYSVIEW_RecordEndCall(X)

#define W_SYSVIEW_RecordU32(...)         SEGGER_SYSVIEW_RecordU32(__VA_ARGS__)
#define W_SYSVIEW_RecordU32x2(...)       SEGGER_SYSVIEW_RecordU32x2(__VA_ARGS__)

#define W_SYSVIEW_RecordEnterISR(...)    SEGGER_SYSVIEW_RecordEnterISR()
#define W_SYSVIEW_RecordExitISR(...)     SEGGER_SYSVIEW_RecordExitISR()
#define W_SYSVIEW_OnTaskCreate(X)        SEGGER_SYSVIEW_OnTaskCreate(X)

#else

#define W_SEGGER_SYSVIEW_OnTaskCreate(X)     EMPTY_MACRO
#define W_SEGGER_SYSVIEW_SendTaskInfo(...)   EMPTY_MACRO

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

#define W_SYSVIEW_RecordEnterISR(...)  EMPTY_MACRO
#define W_SYSVIEW_RecordExitISR(...)   EMPTY_MACRO
#define W_SYSVIEW_OnTaskCreate(X)      EMPTY_MACRO

#endif


#endif /* TASK_MANAGER_WRAPPER_ARM_H_ */
