/*
 * task_manager_wrapper.h
 *
 *  Created on: 30 oct. 2018
 *      Author: Vincent
 */

#ifndef TASK_MANAGER_WRAPPER_H_
#define TASK_MANAGER_WRAPPER_H_

#include "task_manager.h"

/**@brief Returns ID of currently running task.
 *
 * @return ID of active task.
 */
inline task_id_t id_get(void) {
	return (task_id_get() + TASK_BASE_NRF);
}

/**@brief Yield CPU to other tasks.
 */
inline void yield(void) {
	if (task_id_get() < 0) return;
	W_SYSVIEW_OnTaskStopExec(id_get());
	task_yield();
	W_SYSVIEW_OnTaskStartExec(id_get());
}

/**@brief Wait for events. Set events are cleared after this function returns.
 *
 * @param[in] evt_mask Mask of events to wait
 *
 * @return Mask with set events (can be a subset of evt_mask).
 */
inline uint32_t events_wait(uint32_t evt_mask) {
	if (task_id_get() < 0) return 0;
	W_SEGGER_SYSVIEW_OnTaskStopReady(id_get(), evt_mask);
	W_SEGGER_SYSVIEW_OnTaskStop(id_get());
	uint32_t mask = task_events_wait(evt_mask);
	W_SYSVIEW_OnTaskStartExec(id_get());
	return mask;
}

/**@brief Set events for given task.
 *
 * @param[in]  task_id  Id of the task which shall receive events.
 * @param[in]  evt_mask Events for the task.
 *
 */
inline void events_set(task_id_t task_id, uint32_t evt_mask) {
	if (task_id < 0) return;
	W_SEGGER_SYSVIEW_OnTaskStartReady(task_id);
	task_events_set(task_id, evt_mask);
}


#endif /* TASK_MANAGER_WRAPPER_H_ */
