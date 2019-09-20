/*
 * task_manager_wrapper_arm.h
 *
 *  Created on: 13 d�c. 2018
 *      Author: Vincent
 */

#ifndef TASK_MANAGER_WRAPPER_ARM_H_
#define TASK_MANAGER_WRAPPER_ARM_H_

#include "task_manager.h"


#define SYSVIEW_ID_GET()        (task_id_get() + TASK_BASE_NRF)


/**@brief Yield CPU to other tasks.
 */
inline void w_task_yield(void) {
	if (task_id_get() == TASK_ID_INVALID) return;

	task_yield();
	sysview_task_transfer(SYSVIEW_ID_GET());

}

/**@brief Wait for events. Set events are cleared after this function returns.
 *
 * @param[in] evt_mask Mask of events to wait
 *
 * @return Mask with set events (can be a subset of evt_mask).
 */
inline uint32_t w_task_events_wait(uint32_t evt_mask) {
	if (task_id_get() == TASK_ID_INVALID) return 0;

	sysview_task_block(evt_mask);
	uint32_t mask = task_events_wait(evt_mask);
	sysview_task_transfer(SYSVIEW_ID_GET());

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

	sysview_task_event(task_id, evt_mask);
	task_events_set(task_id, evt_mask);
}

/**
 *
 * @param del_ Delay to apply to the task
 */
inline uint32_t w_task_delay(uint32_t del_) {

	sysview_task_block(TASK_EVENT_PERIPH_MS_WAIT);
	uint32_t ret = task_delay(del_);
	sysview_task_transfer(SYSVIEW_ID_GET());

	return ret;
}

/**@brief Cancels the delay of a task
 *
 * @param task_id ID of the task for which to cancel delay
 */
inline void w_task_delay_cancel(task_id_t task_id) {
	sysview_task_event(task_id, TASK_EVENT_PERIPH_MS_WAIT);
	task_delay_cancel(task_id);
}


#endif /* TASK_MANAGER_WRAPPER_ARM_H_ */
