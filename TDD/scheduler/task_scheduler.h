
#ifndef TDD_SCHEDULER_TASK_SCHEDULER_H_
#define TDD_SCHEDULER_TASK_SCHEDULER_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/**@brief Task ID */
typedef uint8_t task_id_t;

/**@brief Invalid task ID */
#define TASK_ID_INVALID ((task_id_t)(-1))

/**
 * Function prototype (task setup and loop functions).
 */
typedef void (*tasked_func_t)(void *p_context);

/**
 *
 * @param stackSize
 * @return
 */
bool task_begin(size_t stackSize);

/**
 *
 * @return 0 if task manager is not started
 */
uint32_t task_manager_is_started(void);

/**
 *
 * @param taskLoop
 * @param stackSize
 * @return
 */
int task_create(tasked_func_t taskLoop, const char *name, size_t stackSize, void *p_context);

/**
 * This function does not return
 *
 * @param idle_task The background task to be run
 * @param p_context Context
 */
void task_start(tasked_func_t idle_task, void *p_context);

task_id_t task_id_get(void);

/**
 * Yields to the next task
 */
void task_yield();

/**
 * Yields to the next task
 */
void task_wait_event(uint32_t event);

/**
 *
 * @param task_id
 * @param event
 */
void task_feed_event(task_id_t task_id, uint32_t event);

/**@brief Delays the current task
 *
 * @param del_ The amount to delay the task by
 * @return 0 if the timeout was cancelled elsewhere
 */
uint32_t task_delay(uint32_t del_);

/**
 *
 * @param task_id ID of the task for which to cancel delay
 */
void task_delay_cancel(task_id_t task_id);

/**
 *
 * @param tick_dur_ The period at which the function runs
 */
void task_tick_manage(uint32_t tick_dur_);


#ifdef __cplusplus
}
#endif

#endif /* TDD_SCHEDULER_TASK_SCHEDULER_H_ */
