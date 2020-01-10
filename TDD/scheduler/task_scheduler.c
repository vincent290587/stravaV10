#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>
#include "task_scheduler.h"
#include "segger_wrapper.h"

#undef setjmp
#define setjmp      __builtin_setjmp

#undef longjmp
#define longjmp     __builtin_longjmp

/**
 * Task run-time structure.
 */
typedef struct task_t {
	struct task_t* next;		//!< Next task.
	struct task_t* prev;		//!< Previous task.
	jmp_buf context;		//!< Task context.
	task_id_t task_id;
	uint32_t events_mask;
	size_t stacksize;
	volatile uint32_t timeout;
	const char *name;
	tasked_func_t exec_func;
	void *p_context;
	const uint8_t* stack;	//!< Task stack top.
} task_t;

/** Default stack size and stack max. */
#define DEFAULT_STACK_SIZE       2048
#define DEFAULT_STACK_SPACING    512

#define STACKDIR         - // set to + for upwards and - for downwards

// Stack magic pattern
const uint8_t MAGIC = 0xa5;

// Main task and run queue
static task_t s_main;

#define MAX_TASKS_NB       10

static task_t m_tasks[MAX_TASKS_NB];
static uint32_t m_tasks_nb = 0;

// Reference running task
static task_t* s_running = NULL;

static char *s_lframe = NULL; // top of stack
static size_t stack_used = 0;

bool task_begin(size_t stackSize)
{
	m_tasks_nb = 0;

	s_lframe = NULL;

	s_main.prev = &s_main;
	s_main.next = &s_main;
	s_main.stack = NULL;

	s_running = &s_main;

	stack_used = DEFAULT_STACK_SIZE + DEFAULT_STACK_SPACING;

	memset(m_tasks, 0, sizeof(m_tasks));

	return (true);
}

static int _task_init(tasked_func_t loop, const char *name, size_t stackSize, uint8_t *stack_top, void *_p_context)
{
	// Add task last in run queue (main task)
	uint8_t task_id = ++m_tasks_nb;

	uint8_t frame[1];

	m_tasks[task_id].task_id = task_id;
	m_tasks[task_id].events_mask = 0;
	m_tasks[task_id].stack = stack_top - stackSize;
	m_tasks[task_id].stacksize = stackSize;
	m_tasks[task_id].name = name;
	m_tasks[task_id].p_context = _p_context;
	m_tasks[task_id].exec_func = loop;
	m_tasks[task_id].timeout = 0;

	m_tasks[task_id].next = &s_main;
	m_tasks[task_id].prev = s_main.prev;

	s_main.prev->next = &m_tasks[task_id];
	s_main.prev = &m_tasks[task_id];

	memset((void*)m_tasks[task_id].stack, MAGIC, stackSize);

	LOG_INFO("Task %s[%u] start stack 0x%X -- 0x%X (%lu)", m_tasks[task_id].name, m_tasks[task_id].task_id, (size_t)frame, stack_top, ((size_t)stack_top - (size_t)&frame));
	LOG_INFO("Stack size: %lu", stackSize);

	s_running = &m_tasks[task_id];

	do {
		// Create context for new task, caller will return
		if (setjmp(m_tasks[task_id].context) == 0) {

			// init code
			longjmp(s_main.context, 1);

		}

		// never returns
		m_tasks[task_id].exec_func(_p_context);

		assert(0);

	} while (1);

	return (int)m_tasks[task_id].task_id;
}

/**
 * https://fanf.livejournal.com/105413.html
 *
 * @param taskLoop
 * @param name
 * @param stackSize
 * @param p_context
 * @return
 */
int task_create(tasked_func_t taskLoop, const char *name, size_t stackSize, void *p_context)
{
	char frame=0;
	if (!s_lframe) s_lframe = &frame;

	size_t addit = (s_lframe - &frame);
	stack_used += stackSize + DEFAULT_STACK_SPACING + addit;
	uint8_t stack[stack_used];

	s_lframe = &frame;

	// Create context for new task, caller will return
	if (setjmp(s_main.context) == 0) {
		_task_init(taskLoop, name, stackSize, &stack[stackSize-1], p_context);
	}

	return m_tasks_nb;
}

void task_start(tasked_func_t idle_task, void *p_context)
{
	LOG_INFO("%u tasks recorded and starting", m_tasks_nb);

	uint8_t idle_stack[DEFAULT_STACK_SIZE + DEFAULT_STACK_SPACING];

	uint8_t frame = 0;
	uint8_t *stack_top = &idle_stack[DEFAULT_STACK_SIZE-1];

	s_main.exec_func = idle_task;
	s_main.stack = idle_stack;
	s_main.name = "Idle task";

	LOG_INFO("Task %s[%u] start stack 0x%X -- 0x%X (%lu)", s_main.name, s_main.task_id, (size_t)&frame, stack_top, ((size_t)stack_top - (size_t)&frame));

//	exit(0);

	// Fill stack with magic pattern to allow detect of stack usage
	memset((void*)s_main.stack, MAGIC, DEFAULT_STACK_SIZE);

//	for (int i = 0; i <= m_tasks_nb; i++) {
//		LOG_INFO("Task %s %d", s_running->name, i);
//		s_running = s_running->next;
//	}

	s_running = &s_main;

	// does not return
	idle_task(NULL);

	assert(0);

	while (1) ;

}

void task_yield()
{
	// save stack
	if (setjmp(s_running->context)) {
		// task resumed, unblock by not jumping to scheduler
		return;
	}

	const uint8_t *p_stack_ = s_running->stack + 1;
	assert(*p_stack_ == MAGIC);

	if (s_running != &s_main) {
		size_t ind = 0;
		for (ind = 0; ind < s_running->stacksize - 1; ind++)  {
			const uint8_t *p_stack = s_running->stack + ind;

			if (*p_stack != MAGIC) {

				break;
			}
		}

		LOG_DEBUG("Finishing task %s -- %lu bytes free", s_running->name, ind);
	}


	// Next task in run queue will continue
	do {
		s_running = s_running->next;
	} while (s_running->timeout > 1 || s_running->events_mask);

//	if (s_running != &s_main) {
//		LOG_INFO("Starting task %s 0x%02X", s_running->name, *p_stack);
//	}

	// jump to scheduler
	longjmp(s_running->context, 1);
}

void task_wait_event(uint32_t event)
{
	LOG_DEBUG("Task %s[%u] wait event %u",
			s_running->name, s_running->task_id, event);

	s_running->events_mask |= event;

	while (s_running->events_mask) task_yield();
}

void task_feed_event(task_id_t task_id, uint32_t event)
{
	if (TASK_ID_INVALID == task_id) {
		LOG_ERROR("Invalid task id");
		return;
	}

	task_t* p_task = s_running;

	for (int i=0; i < MAX_TASKS_NB; i++) {
		if (task_id == p_task->task_id) {
			LOG_DEBUG("Task %s[%u] recv event %u",
					p_task->name,
					p_task->task_id,
					event);
			p_task->events_mask &= ~event;
			return;
		}
		p_task = p_task->next;
	}

	LOG_ERROR("Task not found");
}

task_id_t task_id_get(void) {

	return s_running->task_id;
}

size_t task_stack()
{
	const uint8_t* sp = s_running->stack;
	size_t bytes = 0;
	while (*sp++ == MAGIC) bytes++;
	return (bytes);
}

/**@brief Delays the current task
 *
 * @param del_ The amount to delay the task by
 * @return 0 if the timeout was cancelled elsewhere
 */
uint32_t task_delay(uint32_t del_) {

	LOG_DEBUG("Task %s[%u] delay %u ms",
			s_running->name, s_running->task_id, del_);

	s_running->timeout = del_;

	while (s_running->timeout > 1) task_yield();

	uint32_t ret = s_running->timeout;
	s_running->timeout = 0;

	return ret;
}

/**
 *
 * @param task_id ID of the task for which to cancel delay
 */
void task_delay_cancel(task_id_t task_id) {

	task_t* p_task = s_running;

	if (!p_task) return;

	for (int i=0; i < MAX_TASKS_NB; i++) {
		if (task_id == p_task->task_id) {
			LOG_DEBUG("Task %s[%u] delay cancel",
					p_task->name,
					p_task->task_id);
			p_task->timeout = 0;
			return;
		}
		p_task = p_task->next;
	}

	LOG_ERROR("Task not found");
}

/**
 *
 * @param tick_dur_ The period at which the function runs
 */
void task_tick_manage(uint32_t tick_dur_) {

	for (int i=0; i < MAX_TASKS_NB; i++) {
		task_t* p_task = &m_tasks[i];
		if (p_task->timeout > 1) {
			if (p_task->timeout <= tick_dur_) {
				// unblock the task
				p_task->timeout = 1;
			} else {
				// decrement count
				p_task->timeout -= tick_dur_;
			}
		}

	}

}
