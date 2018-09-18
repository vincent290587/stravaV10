/*
 * main_tdd.cpp
 *
 *  Created on: 17 sept. 2018
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sd_hal.h"
#include "Model.h"
#include "segger_wrapper.h"



/**
 *
 * @return 0
 */
int main(void)
{
	LOG_INFO("Program init");

	millis_init();

	fatfs_init();

	boucle.init();

	delay_ms(1);

	for (;;)
	{
		boucle.tasks();

		// tasks
		perform_system_tasks();

	}
}


