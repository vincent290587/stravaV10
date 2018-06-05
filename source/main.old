/**
 * This is template for main module created by MCUXpresso Project Generator. Enjoy!
 **/

#include "board.h"
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "fsl_fxos.h"
#include "clock_config.h"
#include "Segment.h"
#include "segger_wrapper.h"

#include "composite.h"
#include "sdcard_fatfs.h"
#include "power_manager.h"
#include "Model.h"
#include "UData.h"
#include "spi_scheduler.h"
#include "i2c_scheduler.h"
#include "nrf52.h"

#include "int_i2c0.h"
#include "dma_spi0.h"
#include "uart0.h"
#include "uart2.h"


/*!
 * @brief Application entry point.
 */
int main(void) {

	UData<bool> led_state;

	pwManager.init();

	/* Init board hardware. */
	BOARD_InitBootPins();
	BOARD_InitBootClocks();
	BOARD_InitHardware();

    //
	// High level code
	//
	dma_spi0_mngr_init();
	uaparser.init();

	// LCD driver
	vue.init();

	nrf52_init();
	gps_mgmt.init();

	i2c_scheduling_init();

	boucle.init();

	for(;;) { /* Infinite loop to avoid leaving the main function */

#ifdef DEBUG_CONFIG
		// debug LED
		if (led_state.getAge() >= 1000) {

			if (led_state == true) {
				led_state = false;
//				power_manager_run(kAPP_PowerModeRun120);
			} else {
				led_state = true;
//				power_manager_run(kAPP_PowerModeRun24);
			}

			LED_RED_TOGGLE();
			LED_BLUE_TOGGLE();
			LOG_INFO("LED toggling\r\n");

//			if (millis() > 15000) {
//				power_manager_run(kAPP_PowerModeVlpr);
//				LOG_INFO("VLPR mode\r\n");
//			}

			// update the segger screen
//			sdisplay.clear();
//			sdisplay.setCursor(3,3);
//			sdisplay.setTextSize(2);
//			sdisplay.print(millis());
//			sdisplay.displayRTT();

//			lcd.clearDisplay();
//
//			dma_spi0_mngr_tasks_start();
//			dma_spi0_mngr_finish();
//
//			lcd.setCursor(10,10);
//			lcd.setTextSize(4);
//			lcd.print(millis());
//
//			LS027_InvertColors();
//
//			lcd.writeWhole();
//
//			dma_spi0_mngr_tasks_start();

//          // test rapidite math
//			W_SYSVIEW_OnTaskStartExec(SEG_PERF_TASK);
//			for (int i = 0; i < 1000; i++) {
//				distance_between(46., ((float)i)/1000, 46., ((float)i)/1000);
//			}
//			W_SYSVIEW_OnTaskStopExec(SEG_PERF_TASK);

//			locator.sec_jour = millis() / 1000;
//			LOG_INFO("Locator manual update\r\n");

//			uint8_t array[10] = "Hello\r\n";
//			uart2_send(array, 7);

//			uint8_t reg = 24U;
//			uint8_t i2c_buff[8] = {0};
//
//			i2c0_write(0x70, &reg, 1);
//			i2c0_read(0x70, i2c_buff, sizeof(i2c_buff));
//
//			i2c0_read_reg(0x70, reg, i2c_buff, sizeof(i2c_buff));
//
//			LOG_INFO("Read %02X %02X %02X %02X \r\n",
//					i2c_buff[0], i2c_buff[1],
//					i2c_buff[2], i2c_buff[3]);

			W_SYSVIEW_OnIdle();
		}
#else

		boucle.tasks();

#endif

		// tasks
		perform_system_tasks();

	}

	return 0;
}


