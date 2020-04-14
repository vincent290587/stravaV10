/*
 * kalman_ext.h
 *
 *  Created on: 26 mrt. 2019
 *      Author: v.golle
 *
 *      http://bilgin.esme.org/BitsAndBytes/KalmanFilterforDummies
 *
 *      https://stackoverflow.com/questions/41754085/tracking-a-robot-in-circular-motion-using-kalman-filter
 *
 *      https://dsp.stackexchange.com/questions/36135/tracking-a-sine-wave-with-kalman-filter-how-to-account-for-offset-dc-signal
 */

#include "kalman_ext.h"
#include "math_wrapper.h"
#include "assert_wrapper.h"
#include "nordic_common.h"
#include "segger_wrapper.h"

static void _time_update(sKalmanDescr *descr, sKalmanExtFeed *feed) {

	UDMatrix matXx;
	descr->ker.matXmi = descr->ker.matA * descr->ker.matX;

	if (!feed->matU.isEmpty()) {
		UDMatrix matXu;
		matXu = descr->ker.matB * feed->matU;

		descr->ker.matXmi = descr->ker.matXmi + matXu;
	}
}

/**
 *
 * @param descr
 */
void kalman_lin_init(sKalmanDescr *descr) {

	ASSERT(descr);
	ASSERT(descr->ker.ker_dim);
	ASSERT(descr->ker.obs_dim);

	descr->is_init = 0;

	// size matrixes
	descr->ker.matA.resize(descr->ker.ker_dim, descr->ker.ker_dim);
	descr->ker.matE.resize(descr->ker.ker_dim, descr->ker.ker_dim);
	descr->ker.matP.resize(descr->ker.ker_dim, descr->ker.ker_dim);
	descr->ker.matPmi.resize(descr->ker.ker_dim, descr->ker.ker_dim);
	descr->ker.matQ.resize(descr->ker.ker_dim, descr->ker.ker_dim);
	descr->ker.matR.resize(descr->ker.obs_dim, descr->ker.obs_dim);

	descr->ker.matB.resize(descr->ker.ker_dim, descr->ker.ker_dim);

	// set X
	descr->ker.matX.resize(descr->ker.ker_dim, 1);
	descr->ker.matXmi.resize(descr->ker.ker_dim, 1);

	descr->ker.matKI.resize(descr->ker.ker_dim, 1);

	descr->ker.matK.resize(descr->ker.ker_dim, descr->ker.obs_dim);

	// set C
	descr->ker.matC.resize(descr->ker.obs_dim, descr->ker.ker_dim);

}

/**
 *
 * @param descr
 */
void kalman_ext_init(sKalmanDescr *descr) {

	kalman_lin_init(descr);

	// set H
	descr->ker_ext.matH.resize(descr->ker.obs_dim, descr->ker.ker_dim);
	// it must be the jacobian matrix
	// first line: dZ1 / dState[co]

	// C now maps the state vector to the measurements
	// first val: Z1 = f(State[])
}

/**
 *
 * @param descr
 * @param feed
 */
void kalman_lin_feed(sKalmanDescr *descr, sKalmanExtFeed *feed) {

	ASSERT(descr);
	ASSERT(feed);

	if (!descr->is_init) {

		descr->is_init = 1;

	}

	_time_update(descr, feed);

	// Measurement update
	UDMatrix matAt;
	matAt = descr->ker.matA.transpose();

	// project covariance
	descr->ker.matPmi = descr->ker.matA * descr->ker.matP;
	descr->ker.matPmi = descr->ker.matPmi * matAt;
	descr->ker.matPmi = descr->ker.matPmi + descr->ker.matQ;
	descr->ker.matPmi.bound(1e-15, 1e12);

	// update kalman gain
	UDMatrix matCt;
	matCt = descr->ker.matC.transpose();
	descr->ker.matK = descr->ker.matPmi * matCt;
	UDMatrix matCp;
	matCp = descr->ker.matC * descr->ker.matK;
	matCp = matCp + descr->ker.matR;
	UDMatrix matCpi;
	matCpi = matCp.invert();
	descr->ker.matK = descr->ker.matK * matCpi;

	// z - C*x_est
	UDMatrix matI, innov;
	matI = descr->ker.matC * descr->ker.matXmi;
	innov = feed->matZ - matI;

	// update estimate
	descr->ker.matKI = descr->ker.matK * innov;
	descr->ker.matX = descr->ker.matXmi;
	descr->ker.matX = descr->ker.matX + descr->ker.matKI;

	// update covariance
	UDMatrix matTmp;
	UDMatrix matKI;
	matKI.resize(descr->ker.ker_dim, descr->ker.ker_dim);
	matKI.unity();
	matTmp = descr->ker.matK * descr->ker.matC;
	matKI = matKI - matTmp;
	descr->ker.matP = matKI * descr->ker.matPmi;

}

/**
 * https://towardsdatascience.com/extended-kalman-filter-43e52b16757d
 *
 * @param descr
 * @param feed
 */
void kalman_ext_feed(sKalmanDescr *descr, sKalmanExtFeed *feed) {

	ASSERT(descr);
	ASSERT(feed);

	if (!descr->is_init) {

		// set C
		descr->ker.matC.resize(0, 0);

		descr->is_init = 1;

	}

	_time_update(descr, feed);

	// Measurement update
	UDMatrix matAt;
	matAt = descr->ker.matA.transpose();

	// project covariance
	descr->ker.matPmi = descr->ker.matA * descr->ker.matP;
	descr->ker.matPmi = descr->ker.matPmi * matAt;
	descr->ker.matPmi = descr->ker.matPmi + descr->ker.matQ;
	descr->ker.matPmi.bound(1e-5, 1e3);

	// update extended kalman gain
	UDMatrix matHt;
	matHt = descr->ker_ext.matH.transpose();
	descr->ker.matK = descr->ker.matPmi * matHt;
	UDMatrix matHp;
	matHp = descr->ker_ext.matH * descr->ker.matK;
	matHp = matHp + descr->ker.matR;
	UDMatrix matHpi;
	matHpi = matHp.invert();
	descr->ker.matK = descr->ker.matK * matHpi;

	// z - H * x_est
	UDMatrix matI, innov;
	matI = descr->ker_ext.matH * descr->ker.matXmi;
	innov = feed->matZ - matI;

	// update estimate
	descr->ker.matKI = descr->ker.matK * innov;
	descr->ker.matX = descr->ker.matXmi;
	descr->ker.matX = descr->ker.matX + descr->ker.matKI;

	// update covariance
	UDMatrix matTmp;
	UDMatrix matKI;
	matKI.resize(descr->ker.ker_dim, descr->ker.ker_dim);
	matKI.unity();
	matTmp = descr->ker.matK * descr->ker_ext.matH;
	matKI = matKI - matTmp;
	descr->ker.matP = matKI * descr->ker.matPmi;

}
