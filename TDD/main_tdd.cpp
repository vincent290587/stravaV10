/*
 * main_tdd.cpp
 *
 *  Created on: 17 sept. 2018
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "Model.h"
#include "segger_wrapper.h"



/**
 *
 * @return 0
 */
int main(void)
{

	LOG_INFO("Program init");

	boucle.init();

	for (;;)
	{
		boucle.tasks();

		// tasks
		perform_system_tasks();

	}
}


