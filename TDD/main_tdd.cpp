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

//	Point P1(0., 0., 0., 0.);
//	Point P2(0., 0.00015, 0., 0.);
//	Point P(0.00015, -0.0001, 0., 0.);
//
//	Vecteur P1P2 = Vecteur(P1, P2);
//	Vecteur P1P = Vecteur(P1, P);
//
//	Vecteur projete = Project(P1P2, P1P);
//	Vecteur orthoP1P2 ;
//	orthoP1P2._x = P1P2._y;
//	orthoP1P2._y = -P1P2._x;
//
//	LOG_INFO("P1P2 %f %f", P1P2._x, P1P2._y);
//	LOG_INFO("P1P  %f %f", P1P._x, P1P._y);
//
//	LOG_INFO("dX %f", projete._x / P1P2.getNorm());
//	projete = Project(orthoP1P2, P1P);
//	LOG_INFO("dY %f", projete._y / orthoP1P2.getNorm());
//
//	exit(0);

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


