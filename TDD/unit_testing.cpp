/*
 * unit_testing.cpp
 *
 *  Created on: 3 nov. 2018
 *      Author: Vincent
 */

#include <cmath>
#include <stdint.h>
#include <stdbool.h>
#include "Model_tdd.h"
#include "Screenutils.h"
#include "utils.h"

#include "order1_filter.h"
#include "bp_dis_filter.h"

#define TEST_FILTRE_NB    15

bool test_lsq(void) {

	LOG_INFO("Testing LSQ...");

	float _x[TEST_FILTRE_NB] = {1.47,	1.50,	1.52,	1.55,	1.57,	1.60,	1.63,	1.65,	1.68,	1.70,	1.73,	1.75,	1.78,	1.80,	1.83};
	float _y[TEST_FILTRE_NB] = {52.21,	53.12,	54.48,	55.84,	57.20,	58.57,	59.93,	61.29,	63.11,	64.47,	66.28,	68.10,	69.92,	72.19,	74.46};
	float _lrCoef[2];

	_lrCoef[1] = _lrCoef[0] = 0.;

	// regression lineaire
	float corrsq = simpLinReg(_x, _y, _lrCoef, TEST_FILTRE_NB);

	LOG_INFO("LSQ: %f %f (r=%f)", _lrCoef[0], _lrCoef[1], corrsq);

	if (fabsf(_lrCoef[0] - 61.27) > 0.1) return false;

	LOG_INFO("LSQ OK");

	return true;
}

bool test_functions(void) {

	LOG_INFO("Testing functions...");

	float pi = 3.040516;

	String pi_str = _fmkstr(pi, 2);

	if (pi_str.length() != 4) return false;

	pi_str = _fmkstr(pi, 4);

	if (pi_str.length() != 6) return false;

	pi_str = _fmkstr(pi, 0);

	if (pi_str.length() != 1) return false;

	LOG_INFO("Functions OK");

	return true;
}

bool test_filtering(void) {

	LOG_INFO("Testing filters...");

	float lp1_filter_coefficients[5] =
	{
	// Scaled for floating point: 0.008.fs

	    0.024521609249465722, 0.024521609249465722, 0, 0.9509567815010685, 0// b0, b1, b2, a1, a2

	};

	float lp2_filter_coefficients[5] =
	{
	// Scaled for floating point: 0.25.fs

	    0.5010471990823734, 0.5010471990823734, 0, -0.0020943981647468628, 0// b0, b1, b2, a1, a2

	};

	order1_filterType m_bp_filt;
	order1_filterType m_lp_filt;

	bp_dis_filter_executionState m_bp_dis_filt;

	order1_filter_init(&m_lp_filt, lp1_filter_coefficients);
	order1_filter_init(&m_bp_filt, lp2_filter_coefficients);

	bp_dis_filter_init(&m_bp_dis_filt);

	float sim_ele = 350.;


	for (int i=0; i<700; i++) {

		int alt_noise_cm_p = (rand() % 1250) - 625;
		int alt_sin_cm_p = 10.*sinf(i*2*3.1415/60.);
		float alt_noise =  (((float)alt_noise_cm_p)) / 100.;

		float gps_ele = sim_ele;

		// moving average the barometer measurements
		float baro_ele = sim_ele + alt_noise + alt_sin_cm_p;
		for (int j=0; j< 4; j++) {
			alt_noise_cm_p = (rand() % 1250) - 625;
			alt_noise =  (((float)alt_noise_cm_p)) / 100.;

			float tmp_ele = gps_ele + alt_noise + alt_sin_cm_p + ((float)i) * 0.1;

			order1_filter_writeInput(&m_bp_filt, &tmp_ele);

			baro_ele += tmp_ele;
		}
		baro_ele /= 5;

		float bp_out = order1_filter_readOutput(&m_bp_filt);

		float diff_ele = gps_ele - bp_out;

		order1_filter_writeInput(&m_lp_filt, &diff_ele);
		//		bp_dis_filter_writeInput(&m_bp_dis_filt, baro_ele);

		float lp_out = order1_filter_readOutput(&m_lp_filt);
//		float bp_dis_out = bp_dis_filter_readOutput(&m_bp_dis_filt);

		LOG_INFO("#%f %f %f %f %f", sim_ele + alt_sin_cm_p,
				baro_ele, lp_out, bp_out, bp_out + lp_out);
	}

	LOG_INFO("Finished");

	// TODO remove
	exit(0);

	return true;
}


bool test_nb_points (void) {

	int nb_p2D = Point2D::getObjectCount();
	int nb_p3D = Point::getObjectCount();

	LOG_INFO("Allocated points: %d 2D %d 3D", Point2D::getObjectCount(), Point::getObjectCount());

	{
		Point P1(0., 0., 0., 0.);
		Point P2(P1);
		Point P3 = P1;
		Point P4 = &P1;

		Point2D P2d_1(0.00015, -0.0001);
		LOG_INFO("Allocated points: %d 2D %d 3D", Point2D::getObjectCount(), Point::getObjectCount());
		Point2D P2d_2(P2d_1);
		LOG_INFO("Allocated points: %d 2D %d 3D", Point2D::getObjectCount(), Point::getObjectCount());
		Point2D P2d_3 = P2d_1;
		LOG_INFO("Allocated points: %d 2D %d 3D", Point2D::getObjectCount(), Point::getObjectCount());
		Point2D P2d_4 = &P2d_1;
		LOG_INFO("Allocated points: %d 2D %d 3D", Point2D::getObjectCount(), Point::getObjectCount());

		Point P5 = P2d_1;
		Point P6 = &P2d_1;
	}

	LOG_INFO("Allocated points: %d 2D %d 3D", Point2D::getObjectCount(), Point::getObjectCount());

	if (nb_p2D != Point2D::getObjectCount() || nb_p3D != Point::getObjectCount()) return false;

	return true;
}

bool test_projection (void) {

	Vecteur P1P2(3, 0, 0, 0);
	Vecteur P1P(1, 1.2, 0, 0);

	Vecteur orthoP1P2 ;
	orthoP1P2._x = P1P2._y;
	orthoP1P2._y = -P1P2._x;

	LOG_INFO("P1P2 %f %f", P1P2._x, P1P2._y);
	LOG_INFO("P1P  %f %f", P1P._x, P1P._y);

	Vecteur projete = Project(P1P2, P1P) / P1P2.getNorm();
	LOG_INFO("dX %f", projete._x);
	LOG_INFO("dY %f", projete._y);

	if (fabsf(projete._x - P1P._x) > 0.001) return false;

	projete = Project(orthoP1P2, P1P) / orthoP1P2.getNorm();
	LOG_INFO("dX %f", projete._x);
	LOG_INFO("dY %f", projete._y);

	if (fabsf(projete._y + P1P._y) > 0.001) return false;

	return true;
}

bool test_score(void) {

	SufferScore test_score;
	uint32_t timestamp = 0;

	LOG_INFO("Testing suffer score...");

	for (int i=0; i < 116; i++) test_score.addHrmData(110, (timestamp++)*1000);
	for (int i=0; i < 158; i++) test_score.addHrmData(130, (timestamp++)*1000);
	for (int i=0; i < 1831; i++) test_score.addHrmData(155, (timestamp++)*1000);
	for (int i=0; i < 812; i++) test_score.addHrmData(170, (timestamp++)*1000);
	for (int i=0; i < 330; i++) test_score.addHrmData(180, (timestamp++)*1000);

	LOG_INFO("Score is %f on %f", test_score.getScore(), 71.);

	if (fabsf(test_score.getScore() - 66.46) > 4) return false;

	return true;
}

#define U_LAT     (45.)
#define D_LAT     (0.0001)
#define D_LON     (0.0005)
#define SEG_LENGTH           (15)
#define SEG_POINTS_DELAY     (1.)

bool test_liste (void) {

	Segment mon_seg;
	mon_seg.init();
	for (int i=0; i < SEG_LENGTH; i++) {
		mon_seg.ajouterPointFin(U_LAT, D_LON/2 + i*D_LON, 0., (1+SEG_POINTS_DELAY)*i + 30);
	}

	ListePoints mes_points;
	float cur_lon = -9.*D_LON;
	for (int i=0; i < 7; i++) {
		cur_lon += D_LON;
		mes_points.ajouteFinIso(U_LAT+D_LAT, cur_lon, 0., 1.*i, 7);
	}

	for (int i=0; i < 25; i++) {
		mon_seg.majPerformance(mes_points);

		LOG_INFO("Status: %d", mon_seg.getStatus());

		cur_lon += D_LON;
		mes_points.ajouteFinIso(U_LAT+D_LAT, cur_lon, 0., 35886.+i, 7);

		if (mon_seg.getStatus() < SEG_OFF) {
			LOG_INFO("Avance: %f", mon_seg.getAvance());
			if (fabs(mon_seg.getAvance() - (SEG_POINTS_DELAY*(SEG_LENGTH-1))) > 0.1) return false;
			break;
		}
	}

	mon_seg.uninit();

	return true;
}
