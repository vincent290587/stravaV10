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
#include "millis.h"
#include "sd_functions.h"
#include "Simulator.h"
#include "Model_tdd.h"
#include "i2c_scheduler.h"
#include "segger_wrapper.h"
#include "GUI_connector.h"
#include "unit_testing.hpp"
#include "task_manager_wrapper.h"


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
		w_task_yield();
	}
}

void task1(void *p_context) {
	for (;;) {
		LOG_INFO("Task1");

		sleep(500);
		w_task_events_set(2, 1);
		w_task_yield();
	}
}

void task2(void *p_context) {
	for (;;) {
		w_task_events_wait(1);
		LOG_INFO("Task2");

		sleep(500);
		w_task_events_set(3, 2);
	}
}

void task3(void *p_context) {
	for (;;) {
		w_task_events_wait(2);
		LOG_INFO("Task3");

		sleep(500);
		w_task_events_set(4, 4);
	}
}

void task4(void *p_context) {
	for (;;) {
		w_task_events_wait(4);
		LOG_INFO("Task4");

		sleep(500);
	}
}

/**
 *
 * @param p_context
 */
void idle_task_tdd(void * p_context)
{

	for(;;)
	{

#ifndef LS027_GUI
		millis_increase_time(5);
#endif

		w_task_yield();
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

	if (!test_fusion()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	if (!test_rollover()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	if (!test_lsq()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	if (!test_fram()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	if (!test_power_zone()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	if (!test_functions()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	if (!test_liste ()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	if (!test_projection ()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	if (!test_nb_points ()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	if (!test_score ()) {
		LOG_ERROR("Unit testing failed !");
		exit(-1);
	}

	LOG_INFO("Program init");

	m_tasks_id.boucle_id = TASK_ID_INVALID;
	m_tasks_id.system_id = TASK_ID_INVALID;
	m_tasks_id.peripherals_id = TASK_ID_INVALID;
	m_tasks_id.ls027_id = TASK_ID_INVALID;

	i2c_scheduling_init();

	simulator_init();

#ifdef LS027_GUI
	// start timer for real time simulation
	millis_init();
#endif

	fatfs_init();

	boucle.init();

	vue.init();

	notifications_init(0);

	// check for errors
	if (m_app_error.hf_desc.crc == SYSTEM_DESCR_POS_CRC) {
		LOG_ERROR("Hard Fault found");
		String message = "Hardfault happened: pc = 0x";
		message += String(m_app_error.hf_desc.stck.pc, HEX);
		message += " in void ";
		message += m_app_error.void_id;
		LOG_ERROR("%s", message.c_str());
	    vue.addNotif("Error", message.c_str(), 8, eNotificationTypeComplete);
		memset(&m_app_error.hf_desc, 0, sizeof(m_app_error.hf_desc));
	}

	sNeopixelOrders neo_order;
	SET_NEO_EVENT_RED(neo_order, eNeoEventNotify, 0);
	notifications_setNotify(&neo_order);

	delay_ms(1);

	task_begin(65536 * 5);

	m_tasks_id.boucle_id = task_create(boucle_task, "boucle_task", 65536, NULL);
	m_tasks_id.system_id = task_create(system_task, "system_task", 65536, NULL);
	m_tasks_id.peripherals_id = task_create(peripherals_task, "peripherals_task", 65536, NULL);
	m_tasks_id.ls027_id = task_create(ls027_task, "ls027_task", 65536, NULL);

	task_start(idle_task_tdd, NULL);

}


