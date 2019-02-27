/*
 * app_config.h
 *
 *  Created on: 10 juil. 2018
 *      Author: Vincent
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

// <0=> RC
// <1=> XTAL
// <2=> Synth
#define USER_CLOCK_SOURCE      1


#if (USER_CLOCK_SOURCE==0)

#define USER_CLOCK_CTIV        16

#define USER_CLOCK_TTIV        2

#define USER_CLOCK_ACCURACY    1

#else

#define USER_CLOCK_CTIV        0

#define USER_CLOCK_TTIV        0

#define USER_CLOCK_ACCURACY    7

#endif

#endif /* APP_CONFIG_H_ */
