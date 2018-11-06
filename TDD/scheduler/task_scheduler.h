
#ifndef TDD_SCHEDULER_TASK_SCHEDULER_H_
#define TDD_SCHEDULER_TASK_SCHEDULER_H_

#include <setjmp.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TASK_ID_INVALID                -1

/**@brief Task ID */
typedef uint8_t task_id_t;

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

/**
 *
 * @return The size of the running task stack
 */
size_t task_stack();


#ifdef __cplusplus
}
#endif

#endif /* TDD_SCHEDULER_TASK_SCHEDULER_H_ */
