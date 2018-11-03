/*
 * unit_testing.cpp
 *
 *  Created on: 3 nov. 2018
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdbool.h>
#include "Model_tdd.h"


bool test_projection (void) {

	Point P1(0., 0., 0., 0.);
	Point P2(0., 0.00015, 0., 0.);
	Point P(0.00015, -0.0001, 0., 0.);

	Vecteur P1P2 = Vecteur(P1, P2);
	Vecteur P1P = Vecteur(P1, P);

	Vecteur projete = Project(P1P2, P1P);
	Vecteur orthoP1P2 ;
	orthoP1P2._x = P1P2._y;
	orthoP1P2._y = -P1P2._x;

	LOG_INFO("P1P2 %f %f", P1P2._x, P1P2._y);
	LOG_INFO("P1P  %f %f", P1P._x, P1P._y);

	LOG_INFO("dX %f", projete._x / P1P2.getNorm());
	projete = Project(orthoP1P2, P1P);
	LOG_INFO("dY %f", projete._y / orthoP1P2.getNorm());

	return true;
}

bool test_liste (void) {

	Point P1(0., 0., 0., 0.);
	Point P2(0., 0.00015, 0., 0.);
	Point P(0.00015, -0.0001, 0., 0.);

	Vecteur P1P2 = Vecteur(P1, P2);
	Vecteur P1P = Vecteur(P1, P);

	Vecteur projete = Project(P1P2, P1P);
	Vecteur orthoP1P2 ;
	orthoP1P2._x = P1P2._y;
	orthoP1P2._y = -P1P2._x;

	LOG_INFO("P1P2 %f %f", P1P2._x, P1P2._y);
	LOG_INFO("P1P  %f %f", P1P._x, P1P._y);

	LOG_INFO("dX %f", projete._x / P1P2.getNorm());
	projete = Project(orthoP1P2, P1P);
	LOG_INFO("dY %f", projete._y / orthoP1P2.getNorm());

	return true;
}
