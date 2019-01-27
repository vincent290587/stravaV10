/*
 * Arduino.c
 *
 *  Created on: 25 févr. 2017
 *      Author: Vincent
 */

#include "math_wrapper.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "app_timer.h"
#include "app_error.h"
#include "nordic_common.h"
#include "nrf_pwr_mgmt.h"
#include "Model.h"
#include "sdk_config.h"
#include "task_manager.h"
#include "segger_wrapper.h"

#include "helper.h"


#define TICKS_TO_MS(ticks)       (ticks * ( (APP_TIMER_PRESCALER + 1 ) * 1000 ) / APP_TIMER_CLOCK_FREQ)

#define TICKS_TO_US(ticks)       (ticks * ( (APP_TIMER_PRESCALER + 1 ) * 1000 ) / APP_TIMER_CLOCK_FREQ)

#define FPU_EXCEPTION_MASK               0x0000009F                      //!< FPU exception mask used to clear exceptions in FPSCR register.
#define FPU_FPSCR_REG_STACK_OFF          0x40                            //!< Offset of FPSCR register stacked during interrupt handling in FPU part stack.


#ifdef FPU_INTERRUPT_MODE

//register int *r0 __asm("r0");

/**
 * @brief FPU Interrupt handler. Clearing exception flag at the stack.
 *
 * Function clears exception flag in FPSCR register and at the stack. During interrupt handler
 * execution FPU registers might be copied to the stack (see lazy stacking option) and
 * it is necessary to clear data at the stack which will be recovered in the return from
 * interrupt handling.
 */
void FPU_IRQHandler(void)
{
    // Prepare pointer to stack address with pushed FPSCR register.
    uint32_t * fpscr = (uint32_t * )(FPU->FPCAR + FPU_FPSCR_REG_STACK_OFF);
    // Execute FPU instruction to activate lazy stacking.
    (void)__get_FPSCR();

    /*
     * The last chance to indicate an error in FPU to the user
     * as the FPSCR is now cleared
     *
     * This assert is related to previous FPU operations
     * and not power management.
     *
     * Critical FPU exceptions signaled:
     * - IOC - Invalid Operation cumulative exception bit.
     * - DZC - Division by Zero cumulative exception bit.
     * - OFC - Overflow cumulative exception bit.
     */
    if (*fpscr & 0x7) {
    	W_SYSVIEW_RecordEnterISR();
    	LOG_ERROR("FPU exception detected 0x%02X", *fpscr);
    	// https://stackoverflow.com/questions/38724658/find-where-the-interrupt-happened-on-cortex-m4
//    	__asm(  "TST lr, #4\n"
//    			"ITE EQ\n"
//    			"MRSEQ r0, MSP\n"
//    			"MRSNE r0, PSP\n" // stack pointer now in r0
//    			"ldr r0, [r0, #0x18]\n" // stored pc now in r0
//    			//"add r0, r0, #6\n" // address to stored pc now in r0
//    	);
        W_SYSVIEW_RecordExitISR();
    }

    // Clear flags in stacked FPSCR register.
    *fpscr = *fpscr & ~(FPU_EXCEPTION_MASK);
}

#endif

void pwr_mgmt_run(void) {
#ifndef FPU_INTERRUPT_MODE
	uint32_t original_fpscr;

	CRITICAL_REGION_ENTER();
	original_fpscr = __get_FPSCR();
	/*
	 * Clear FPU exceptions.
	 * Without this step, the FPU interrupt is marked as pending,
	 * preventing system from sleeping. Exceptions cleared:
	 * - IOC - Invalid Operation cumulative exception bit.
	 * - DZC - Division by Zero cumulative exception bit.
	 * - OFC - Overflow cumulative exception bit.
	 * - UFC - Underflow cumulative exception bit.
	 * - IXC - Inexact cumulative exception bit.
	 * - IDC - Input Denormal cumulative exception bit.
	 */
	__set_FPSCR(original_fpscr & ~0x9Fu);
	__DMB();
	NVIC_ClearPendingIRQ(FPU_IRQn);
	CRITICAL_REGION_EXIT();

	/*
	 * The last chance to indicate an error in FPU to the user
	 * as the FPSCR is now cleared
	 *
	 * This assert is related to previous FPU operations
	 * and not power management.
	 *
	 * Critical FPU exceptions signaled:
	 * - IOC - Invalid Operation cumulative exception bit.
	 * - DZC - Division by Zero cumulative exception bit.
	 * - OFC - Overflow cumulative exception bit.
	 */
#ifdef DEBUG
	ASSERT((original_fpscr & 0x7) == 0);
#else
	APP_ERROR_CHECK(0x1);
#endif
#endif

	nrf_pwr_mgmt_run();

}


void delay(uint32_t p_time) {

	nrf_delay_ms(p_time);

}

void pinMode(uint8_t p_pin, uint8_t p_mode) {

	if (p_mode == OUTPUT) {
		nrf_gpio_cfg_output(p_pin);
	} else {
		switch (p_mode) {
		case INPUT:
			nrf_gpio_cfg_input(p_pin, NRF_GPIO_PIN_NOPULL);
			break;
		case INPUT_PULLDOWN:
			nrf_gpio_cfg_input(p_pin, NRF_GPIO_PIN_PULLDOWN);
			break;
		case INPUT_PULLUP:
			nrf_gpio_cfg_input(p_pin, NRF_GPIO_PIN_PULLUP);
			break;
		default:
			NRF_LOG_ERROR("Wrong pin configuration");
			break;
		}
	}
}


void digitalWrite(uint8_t p_pin, uint8_t p_mode) {
	if (p_mode == LOW) {
		nrf_gpio_pin_clear(p_pin);
	} else {
		nrf_gpio_pin_set(p_pin);
	}
}


uint32_t digitalRead(uint8_t p_pin) {
	return nrf_gpio_pin_read(p_pin);
}


void attachInterrupt(nrf_drv_gpiote_pin_t pin_, nrf_drv_gpiote_evt_handler_t evt_handler) {

	ret_code_t err_code;

	if (!nrf_drv_gpiote_is_init()) {
		err_code = nrf_drv_gpiote_init();
		APP_ERROR_CHECK(err_code);
	}

	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
	in_config.hi_accuracy = false;
	in_config.is_watcher = false;
	in_config.sense = NRF_GPIOTE_POLARITY_HITOLO;
	in_config.pull = NRF_GPIO_PIN_PULLUP;

	err_code = nrf_drv_gpiote_in_init(pin_, &in_config, evt_handler);
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_event_enable(pin_, true);
}


void detachInterrupt(nrf_drv_gpiote_pin_t pin_) {

	nrf_drv_gpiote_in_uninit(pin_);

}


float compute2Complement16_t(uint8_t msb, uint8_t lsb) {
	uint16_t t;
	uint16_t val;
	uint8_t tl=lsb, th=msb;
	float ret;

	if (th & 0b10000000) {
		t = th << 8;
		val = (t & 0xFF00) | (tl & 0x00FF);
		val -= 1;
		val = ~(val | 0b1000000000000000);
		//LOG_INFO("Raw 2c1: %u\r\n", val);
		ret = (float)val;
	} else {
		t = (th & 0xFF) << 8;
		val = (t & 0xFF00) | (tl & 0x00FF);
		//LOG_INFO("Raw 2c2: %u\r\n", val);
		ret = (float)-val;
	}

	return ret;
}

float compute2Complement24_t(uint8_t msb, uint8_t csb, uint8_t lsb) {
	uint16_t val;
	uint8_t tl=lsb, tc=csb, th=msb;
	float ret;

	if (msb & 0b10000000) {
		val = (th << 16) | (tc << 8) | tl;
		val -= 1;
		val = ~(val | 0b100000000000000000000000);
		//LOG_INFO("Raw 2c1: %u\r\n", val);
		ret = (float)val;
	} else {
		val = (th << 16) | (tc << 8) | tl;
		//LOG_INFO("Raw 2c2: %u\r\n", val);
		ret = (float)-val;
	}

	return ret;
}

bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

int from_hex(char a)
{
  if (a >= 'A' && a <= 'F')
    return a - 'A' + 10;
  else if (a >= 'a' && a <= 'f')
    return a - 'a' + 10;
  else
    return a - '0';
}
