/*
 * usb_cdc.c
 *
 *  Created on: 9 janv. 2020
 *      Author: vgol
 */

#include "sdk_config.h"
#include "sd_hal.h"
#include "usb_cdc.h"
#include "Model.h"



void usb_cdc_start_msc(void) {

	ret_code_t ret;

	fatfs_uninit();

	// remove segments and go to the proper mode
	model_go_to_msc_mode();

}
