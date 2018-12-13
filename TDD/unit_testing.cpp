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
