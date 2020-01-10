/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include <unistd.h>
#include "Model.h"
#include "uart_tdd.h"
#include "Simulator.h"
#include "neopixel.h"
#include "gpio.h"
#include "ble_api_base.h"
#include "segger_wrapper.h"



/**
 *
 */
void perform_system_tasks(void) {

	uart_tasks();

	btn_task();

}

/**
 * System continuous tasks
 *
 * @param p_context
 */
void system_task(void * p_context)
{
	for(;;)
	{
		perform_system_tasks();

		simulator_tasks();

		w_task_delay(25);
	}
}
