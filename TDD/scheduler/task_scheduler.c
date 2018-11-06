#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "task_scheduler.h"
#include "segger_wrapper.h"

/**
 * Task run-time structure.
 */
typedef struct task_t {
	struct task_t* next;		//!< Next task.
	struct task_t* prev;		//!< Previous task.
	jmp_buf context;		//!< Task context.
	task_id_t task_id;
	uint32_t events_mask;
	const char *name;
	tasked_func_t exec_func;
	void *p_context;
	const uint8_t* stack;	//!< Task stack top.
} task_t;

/** Default stack size and stack max. */
#define DEFAULT_STACK_SIZE       2048

// Stack magic pattern
const uint8_t MAGIC = 0xa5;

// Main task and run queue
static task_t s_main;

#define MAX_TASKS_NB       5

static task_t m_tasks[MAX_TASKS_NB];
static uint32_t m_tasks_nb = 0;
static uint32_t m_cur_task_id = 0;

// Reference running task
static task_t* s_running = &s_main;

// Initial top stack for task allocation
size_t s_top = DEFAULT_STACK_SIZE;

bool task_begin(size_t stackSize)
{
	// Set main task stack size
	s_top = stackSize;
	s_main.prev = &s_main;
	s_main.next = &s_main;
	s_main.stack = NULL;
	return (true);
}

static int _task_init(tasked_func_t loop, const char *name, const uint8_t* stack, void *_p_context)
{
	// Add task last in run queue (main task)
	uint8_t task_id = m_tasks_nb++;

	m_tasks[task_id].task_id = task_id;
	m_tasks[task_id].events_mask = 0;
	m_tasks[task_id].stack = stack;
	m_tasks[task_id].name = name;
	m_tasks[task_id].p_context = _p_context;
	m_tasks[task_id].exec_func = loop;

	m_tasks[task_id].next = &s_main;
	m_tasks[task_id].prev = s_main.prev;
	s_main.prev->next = &m_tasks[task_id];
	s_main.prev = &m_tasks[task_id];

	LOG_INFO("Task %s[%u] start...", m_tasks[task_id].name, m_tasks[task_id].task_id);

	// Create context for new task, caller will return
	if (setjmp(m_tasks[task_id].context)) {
		m_tasks[task_id].exec_func(_p_context);
	}

	return (int)m_tasks[task_id].task_id;
}

int task_create(tasked_func_t taskLoop, const char *name, size_t stackSize, void *p_context)
{
	// Check called from main task and valid task loop function
	if ((s_running != &s_main) || !taskLoop) return -2;

	// Adjust stack size with size of task context
	stackSize += sizeof(task_t);

	// Allocate stack(s) and check if main stack top should be set
	uint8_t stack[s_top];
	if (s_main.stack == NULL) {
		s_main.stack = stack;
		memset(stack, MAGIC, s_top);
	}

	// Adjust stack top for next task allocation
	s_top += stackSize;

	// Fill stack with magic pattern to allow detect of stack usage
	memset(stack - stackSize, MAGIC, stackSize - sizeof(task_t));

	// Initiate task with given functions and stack top
	return _task_init(taskLoop, name, stack - stackSize, p_context);
}

void task_start(tasked_func_t idle_task, void *p_context)
{
	LOG_INFO("%u tasks recorded and starting", m_tasks_nb);

	while (1) {
		LOG_DEBUG("Main task...");
		idle_task(p_context);
	}
}

void task_yield()
{
	// Caller will continue here on yield
	if (setjmp(s_running->context)) return;

	LOG_DEBUG("Finishing task %u", s_running->task_id);

	// Next task in run queue will continue
	s_running = s_running->next;

	LOG_DEBUG("Starting task %u", s_running->task_id);

	longjmp(s_running->context, true);
}

void task_wait_event(uint32_t event)
{
	LOG_INFO("Task %s[%u] wait event %u",
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
			LOG_INFO("Task %s[%u] recv event %u",
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

size_t task_stack()
{
	const uint8_t* sp = s_running->stack;
	size_t bytes = 0;
	while (*sp++ == MAGIC) bytes++;
	return (bytes);
}


