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
static inline task_id_t id_get(void) {
	return 0;
}

/**@brief Yield CPU to other tasks.
 */
static inline void yield(void) {
	task_yield();
}

/**@brief Wait for events. Set events are cleared after this function returns.
 *
 * @param[in] evt_mask Mask of events to wait
 *
 * @return Mask with set events (can be a subset of evt_mask).
 */
static inline uint32_t events_wait(uint32_t evt_mask) {
	task_wait_event(evt_mask);
	return 0;
}

/**@brief Set events for given task.
 *
 * @param[in]  task_id  Id of the task which shall receive events.
 * @param[in]  evt_mask Events for the task.
 *
 */
static inline void events_set(task_id_t task_id, uint32_t evt_mask) {
	task_feed_event(task_id, evt_mask);
}

#ifdef __cplusplus
}
#endif

#endif /* TASK_MANAGER_WRAPPER_TDD_H_ */
