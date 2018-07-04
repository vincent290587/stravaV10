/*
 * Arduino.c
 *
 *  Created on: 25 févr. 2017
 *      Author: Vincent
 */

#include "math.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "app_util_platform.h"
#include "app_timer.h"
#include "app_error.h"
#include "nordic_common.h"
#include "sdk_config.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "helper.h"


#define TICKS_TO_MS(ticks)       (ticks * ( (APP_TIMER_PRESCALER + 1 ) * 1000 ) / APP_TIMER_CLOCK_FREQ)

#define TICKS_TO_US(ticks)       (ticks * ( (APP_TIMER_PRESCALER + 1 ) * 1000 ) / APP_TIMER_CLOCK_FREQ)




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
