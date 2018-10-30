/*
 * task_manager_wrapper.h
 *
 *  Created on: 30 oct. 2018
 *      Author: Vincent
 */

#ifndef TASK_MANAGER_WRAPPER_TDD_H_
#define TASK_MANAGER_WRAPPER_TDD_H_

#include <stdint.h>

#define TASK_ID_INVALID                -1

/**@brief Task ID */
typedef uint8_t task_id_t;

/**@brief Returns ID of currently running task.
 *
 * @return ID of active task.
 */
inline task_id_t id_get(void) {
	return 0;
}

/**@brief Yield CPU to other tasks.
 */
inline void yield(void) {

}

/**@brief Wait for events. Set events are cleared after this function returns.
 *
 * @param[in] evt_mask Mask of events to wait
 *
 * @return Mask with set events (can be a subset of evt_mask).
 */
inline uint32_t events_wait(uint32_t evt_mask) {
	return 0;
}

/**@brief Set events for given task.
 *
 * @param[in]  task_id  Id of the task which shall receive events.
 * @param[in]  evt_mask Events for the task.
 *
 */
inline void events_set(task_id_t task_id, uint32_t evt_mask) {

}


#endif /* TASK_MANAGER_WRAPPER_TDD_H_ */
