/*
 * task_manager_wrapper.h
 *
 *  Created on: 30 oct. 2018
 *      Author: Vincent
 */

#ifndef TASK_MANAGER_WRAPPER_TDD_H_
#define TASK_MANAGER_WRAPPER_TDD_H_

#include <stdint.h>
#include "task_scheduler.h"


#ifdef __cplusplus
extern "C" {
#endif

/**@brief Returns ID of currently running task.
 *
 * @return ID of active task.
 */
static inline task_id_t w_task_id_get(void) {
	return 0;
}

/**@brief Yield CPU to other tasks.
 */
static inline void w_task_yield(void) {
	task_yield();
}

/**@brief Wait for events. Set events are cleared after this function returns.
 *
 * @param[in] evt_mask Mask of events to wait
 *
 * @return Mask with set events (can be a subset of evt_mask).
 */
static inline uint32_t w_task_events_wait(uint32_t evt_mask) {
	task_wait_event(evt_mask);
	return 0;
}

/**@brief Set events for given task.
 *
 * @param[in]  task_id  Id of the task which shall receive events.
 * @param[in]  evt_mask Events for the task.
 *
 */
static inline void w_task_events_set(task_id_t task_id, uint32_t evt_mask) {
	task_feed_event(task_id, evt_mask);
}

/**
 *
 * @param del_ Delay to apply to the task
 */
static inline uint32_t w_task_delay(uint32_t del_) {

	uint32_t ret = task_delay(del_);

	return ret;
}

/**@brief Cancels the delay of a task
 *
 * @param task_id ID of the task for which to cancel delay
 */
static inline void w_task_delay_cancel(task_id_t task_id) {
	task_delay_cancel(task_id);
}

#ifdef __cplusplus
}
#endif

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

#endif /* TASK_MANAGER_WRAPPER_TDD_H_ */
