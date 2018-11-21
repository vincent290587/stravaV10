/*
 * main_tdd.cpp
 *
 *  Created on: 17 sept. 2018
 *      Author: Vincent
 */

#pragma STDC FENV_ACCESS on
#define _GNU_SOURCE 1

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "sd_hal.h"
#include "sd_functions.h"
#include "Simulator.h"
#include "Model_tdd.h"
#include "segger_wrapper.h"
#include "unit_testing.hpp"
#include "task_scheduler.h"


#include <fenv.h> // For feenableexcept
#include <execinfo.h> // For backtrace and backtrace_symbols_fd
#include <unistd.h> // For STDERR_FILENO
#include <signal.h> // To register the signal handler

void print_backtrace(void)
{
        static const char start[] = "BACKTRACE ------------\n";
        static const char end[] = "----------------------\n";

        void *bt[1024];
        int bt_size;
        char **bt_syms;
        int i;

        bt_size = backtrace(bt, 1024);
        bt_syms = backtrace_symbols(bt, bt_size);
        write(STDERR_FILENO, start, strlen(start));
        for (i = 1; i < bt_size; i++) {
                size_t len = strlen(bt_syms[i]);
                write(STDERR_FILENO, bt_syms[i], len);
                write(STDERR_FILENO, "\n", 1);
        }
        write(STDERR_FILENO, end, strlen(end));
    free(bt_syms);
}

void signalHandler( int signum ) {

    printf("Interrupt signal %u received.\n", signum);

    // Get a back trace
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    backtrace_symbols_fd(array, size, STDERR_FILENO);

    print_backtrace();

    // cleanup and close up stuff here
    // terminate program

    exit(signum);
}

void pipeHandler( int signum ) {
	printf("Screen shutdown detected\n");
    exit(signum);
}

void idle_task_test(void *p_context) {
	for (;;) {
		sleep(500);
		yield();
	}
}

void task1(void *p_context) {
	for (;;) {
		LOG_INFO("Task1");

		sleep(500);
		events_set(2, 1);
		yield();
	}
}

void task2(void *p_context) {
	for (;;) {
		events_wait(1);
		LOG_INFO("Task2");

		sleep(500);
		events_set(3, 2);
	}
}

void task3(void *p_context) {
	for (;;) {
		events_wait(2);
		LOG_INFO("Task3");

		sleep(500);
		events_set(4, 4);
	}
}

void task4(void *p_context) {
	for (;;) {
		events_wait(4);
		LOG_INFO("Task4");

		sleep(500);
	}
}

/**
 *
 * @return 0
 */
int main(void)
{
	// Enable exceptions for certain floating point results
	feenableexcept(FE_INVALID   |
			FE_DIVBYZERO |
			FE_OVERFLOW  |
			FE_UNDERFLOW);

	// Install the trap handler
	// register signal SIGINT and signal handler
	signal(SIGFPE, signalHandler);
	signal(SIGSEGV, signalHandler);
	signal(SIGABRT, signalHandler);

	// pipe handler
	signal(SIGPIPE, pipeHandler);

	LOG_INFO("Unit testing...");

	if (!test_liste ()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	LOG_INFO("Program init");

	m_tasks_id.boucle_id = TASK_ID_INVALID;
	m_tasks_id.system_id = TASK_ID_INVALID;
	m_tasks_id.peripherals_id = TASK_ID_INVALID;
	m_tasks_id.ls027_id = TASK_ID_INVALID;

	simulator_init();

	millis_init();

	fatfs_init();

	boucle.init();

	baro.init();

	vue.init();

	delay_ms(1);

	task_begin(65536 * 5);

//	m_tasks_id.boucle_id = task_create(task1, "task1", 2048, NULL);
//	m_tasks_id.system_id = task_create(task2, "task2", 2048, NULL);
//	m_tasks_id.peripherals_id = task_create(task3, "task3", 2048, NULL);
//	m_tasks_id.ls027_id = task_create(task4, "task4", 2048, NULL);
//
//	task_start(idle_task_test, NULL);

	m_tasks_id.boucle_id = task_create(boucle_task, "boucle_task", 65536, NULL);
	m_tasks_id.system_id = task_create(system_task, "system_task", 65536, NULL);
	m_tasks_id.peripherals_id = task_create(peripherals_task, "peripherals_task", 65536, NULL);
	m_tasks_id.ls027_id = task_create(ls027_task, "ls027_task", 65536, NULL);

	task_start(idle_task, NULL);

}


