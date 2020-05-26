/*
 * kalman_ext.h
 *
 *  Created on: 26 mrt. 2019
 *      Author: v.golle
 */

#ifndef LIBRARIES_KALMAN_KALMAN_EXT_H_
#define LIBRARIES_KALMAN_KALMAN_EXT_H_

#include <stdint.h>
#include "UDMatrix.h"

/**
 * Input used to estimate the rotation rate only
 */
typedef struct {
	float dt;
	UDMatrix matZ;
	UDMatrix matU;
} sKalmanExtFeed;

typedef struct {
	UDMatrix matA;
	UDMatrix matC;
	UDMatrix matB;
	UDMatrix matE;
	UDMatrix matK;
	UDMatrix matX;
	UDMatrix matKI;
	UDMatrix matXmi;
	UDMatrix matPmi;
	UDMatrix matP;
	UDMatrix matR;
	UDMatrix matQ;
	uint16_t obs_dim;
	uint16_t ker_dim;
} sKalmanLinKernel;

typedef struct {
	UDMatrix matH;
} sKalmanExtKernel;

typedef struct {
	sKalmanLinKernel ker;
	sKalmanExtKernel ker_ext;
	uint8_t is_init;
} sKalmanDescr;


#ifdef	__cplusplus
extern "C" {
#endif


void kalman_lin_init(sKalmanDescr *descr);

void kalman_ext_init(sKalmanDescr *descr);

void kalman_lin_feed(sKalmanDescr *descr, sKalmanExtFeed *feed);

void kalman_ext_feed(sKalmanDescr *descr, sKalmanExtFeed *feed);


#ifdef	__cplusplus
}
#endif


#endif /* LIBRARIES_KALMAN_KALMAN_EXT_H_ */
