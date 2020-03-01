/*
 * power_scheduler.h
 *
 *  Created on: 10 févr. 2020
 *      Author: Vincent
 */

#ifndef SOURCE_SCHEDULING_POWER_SCHEDULER_H_
#define SOURCE_SCHEDULING_POWER_SCHEDULER_H_


typedef enum {
	ePowerSchedulerPingCRS,
	ePowerSchedulerPingFEC,
} ePowerSchedulerPing;


//////////////////////////     FUNCTIONS

#ifdef __cplusplus
extern "C" {
#endif


void power_scheduler__run(void);

void power_scheduler__ping(ePowerSchedulerPing ping_type);


#ifdef __cplusplus
}
#endif


#endif /* SOURCE_SCHEDULING_POWER_SCHEDULER_H_ */
