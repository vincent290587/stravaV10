/*
 * Task.hpp
 *
 *  Created on: 29 oct. 2018
 *      Author: Vincent
 */

#ifndef SOURCE_TASK_HPP_
#define SOURCE_TASK_HPP_

#include "task_manager.h"

class Task {
public:
	Task() {
		m_task_id = TASK_ID_INVALID;
	}

	virtual void run(void * p_context) = 0;

	/**@brief Create new task.
	 *
	 * @param[in]   task        Function which become main procedure of new task.
	 * @param[in]   p_task_name Task name.
	 * @param[in]   p_context   Context passed to task procedure.
	 *
	 * @return      ID of the task on success, otherwise TASK_ID_INVALID.
	 */
	void create(char const * p_task_name) {
		m_task_id = task_create(this->run(), p_task_name, NULL);
	}

	/**@brief Yield CPU to other tasks.
	 */
	void yield(void) {
		yield();
	}

	/**@brief Complete current task.
	 *
	 * Task stack returns to the pool of available stacks.
	 */
	void exit(void) {
		task_exit();
	}

	/**@brief Wait for events. Set events are cleared after this function returns.
	 *
	 * @param[in] evt_mask Mask of events to wait
	 *
	 * @return Mask with set events (can be a subset of evt_mask).
	 */
	uint32_t events_wait(uint32_t evt_mask) {
		task_events_wait(evt_mask);
	}

	/**@brief Set events for given task.
	 *
	 * @param[in]  task_id  Id of the task which shall receive events.
	 * @param[in]  evt_mask Events for the task.
	 *
	 */
	void events_set(task_id_t task_id, uint32_t evt_mask) {
		task_events_set(task_id, evt_mask);
	}

	/**@brief Returns maximum depth of task stack.
	 *
	 * @param[in] task_id Id of the task (use @ref TASK_ID_INVALID for current task).
	 * @return Number of bytes ever used on task stack.
	 */
	uint32_t stack_max_usage_get(task_id_t task_id) {
		task_stack_max_usage_get(m_task_id);
	}

	/**@brief Returns ID of currently running task.
	 *
	 * @return ID of active task.
	 */
	task_id_t id_get(void) {
		return m_task_id;
	}

protected:
	uint32_t m_task_id;
};

#endif /* SOURCE_MODEL_BOUCLE_H_ */



#endif /* SOURCE_TASK_HPP_ */
