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


#include <fenv.h> // For feenableexcept
#include <execinfo.h> // For backtrace and backtrace_symbols_fd
#include <unistd.h> // For STDERR_FILENO
#include <signal.h> // To register the signal handler


void signalHandler( int signum ) {

    printf("Interrupt signal %u received.\n", signum);

    // Get a back trace
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    backtrace_symbols_fd(array, size, STDERR_FILENO);

    // cleanup and close up stuff here
    // terminate program

    exit(signum);
}

/**
 *
 * @return 0
 */
#include "Vecteur.h"
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

	LOG_INFO("Unit testing...");

	test_liste ();

	LOG_INFO("Program init");

	simulator_init();

	millis_init();

	fatfs_init();

	boucle.init();

	baro.init();

	vue.init();

	delay_ms(1);

	for (;;)
	{
		boucle.tasks();

		// tasks
		perform_system_tasks();

		simulator_tasks();

		sleep(2);

	}
}


