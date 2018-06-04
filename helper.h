/*
 * Arduino.h
 *
 *  Created on: 25 f�vr. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_ARDUINO_H_
#define LIBRARIES_ARDUINO_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "nrf_drv_spi.h"
#include "nrf_drv_gpiote.h"
#include "nrf.h"
#include "nrf_delay.h"


#define boolean uint8_t
#define byte    uint8_t

#define OUTPUT         0
#define INPUT          1
#define INPUT_PULLUP   2
#define INPUT_PULLDOWN 3

#define LOW 0
#define HIGH 1

#define _BV(bit) (1 << (bit))

//////////    STRUCTURES



//////////    FUNCTIONS


#ifdef __cplusplus
extern "C" {
#endif

void init_timers_millis();

/** @brief Minimum of two uint32_t
 *
 * @param val1
 * @param val2
 * @return
 */
uint32_t min(uint32_t val1, uint32_t val2);

/** @brief Maximum of two uint32_t
 *
 * @param val1
 * @param val2
 * @return
 */
uint32_t max(uint32_t val1, uint32_t val2);

/**
 * Delay of X milliseconds. Blocking function
 * @param p_time Number of ms to wait for
 */
void delay(uint32_t p_time);

/** @brief configures a pin either as an input (see p_mode) or as an output (OUTPUT)
 *
 * @param p_pin Pin number.
 * @param p_mode Either INPUT (NRF_GPIO_PIN_NOPULL), INPUT_PULLDOWN (NRF_GPIO_PIN_PULLDOWN), NRF_GPIO_PIN_PULLUP (NRF_GPIO_PIN_PULLUP)
 */
void pinMode(uint8_t p_pin, uint8_t p_mode);

/**
 * @brief Reads a digital pin
 * @param p_pin Pin number
 * @return
 */
uint32_t digitalRead(uint8_t p_pin);

/** @brief Sets an output pin value
 *
 * @param p_pin Pin number
 * @param p_mode Mode: either LOW or HIGH
 */
void digitalWrite(uint8_t p_pin, uint8_t p_mode);

/**
 * @brief Attaches an interrupt to a pin.
 *
 * Polarity is NRF_GPIOTE_POLARITY_HITOLO and pin is configured with NRF_GPIO_PIN_PULLUP
 *
 * @param pin_ Pin number
 * @param evt_handler Event function to be called on the interupt
 */
void attachInterrupt(nrf_drv_gpiote_pin_t pin_, nrf_drv_gpiote_evt_handler_t evt_handler);

/**
 * Removes the interrupt from a pin
 * @param pin_
 */
void detachInterrupt(nrf_drv_gpiote_pin_t pin_);

/**
 * @brief Computes the 2 complement of a uint16_t value split in two uint8_t values
 * @param msb MSB of the variable
 * @param lsb LSB of the variable
 * @return
 */
float compute2Complement16_t(uint8_t msb, uint8_t lsb);

/**
 * @brief Computes the 2 complement of a uint24_t value split in three uint8_t values
 * @param msb MSB of the variable
 * @param csb Central byte of the variable
 * @param lsb LSB of the variable
 * @return
 */
float compute2Complement24_t(uint8_t msb, uint8_t csb, uint8_t lsb);

/** @brief Tests if a char is a digit (ie belongs to '0' to '9')
 *
 * @param c The input char to test
 * @return True if it is, false otherwise
 */
bool is_digit(char c);

/** @brief Parses a char that describes part of a hex. to a decimal value
 *  exemple: 'F' will be parsed as 15
 *
 * @param a The input char
 * @return An int containing the parsed value
 */
int from_hex(char a);

/** @brief Linear interpolation function
 *
 * Interpolates a value x1 from one interval B1 to another interval B2:
 * b1_i-------------x1----b1_f
 * becomes
 * b2_i-------------x2----b2_f
 * If x1 is out of B1, x2 will be out of B2
 *
 * @param val_ Value to interpolate
 * @param b1_i Initial interval start
 * @param b1_f Initial interval end
 * @param b2_i Initial interval start
 * @param b2_f Initial interval end
 * @return x2
 */
float regFen(float val_, float b1_i, float b1_f, float b2_i, float b2_f);

/** @brief Linear interpolation function
 *
 * Interpolates a value x1 from one interval B1 to another interval B2:
 * b1_i-------------x1----b1_f
 * becomes
 * b2_i-------------x2----b2_f
 * The value of x2 i limited to B2:
 * If x1 is out of B1, x2 will be at the limit of B2
 *
 * @param val_ Value to interpolate
 * @param b1_i Initial interval start
 * @param b1_f Initial interval end
 * @param b2_i Initial interval start
 * @param b2_f Initial interval end
 * @return x2
 */
float regFenLim(float val_, float b1_i, float b1_f, float b2_i, float b2_f);

/**
 *
 * @param dest
 * @param input
 */
void encode_uint16 (uint8_t* dest, uint16_t input);

/**
 *
 * @param dest
 * @param input
 */
void encode_uint32 (uint8_t* dest, uint32_t input);

/**
 *
 * @param dest
 * @return
 */
uint16_t decode_uint16 (uint8_t* dest);

/**
 *
 * @param dest
 * @return
 */
uint32_t decode_uint32 (uint8_t* dest);

#ifdef __cplusplus
}
#endif



#endif /* LIBRARIES_ARDUINO_H_ */
