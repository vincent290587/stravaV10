/* 
 * File:   Vecteur.cpp
 * Author: vincent
 * 
 * Created on October 27, 2015, 3:07 PM
 */

#include "math_wrapper.h"
#include "Vecteur.h"

Vecteur Project(Vecteur const &v1, Vecteur const &v2) {
	Vecteur res = v1;
	res.project(v2);
	return res;
}

float ScalarProduct(Vecteur const &v1, Vecteur const &v2) {
	return (v1._x*v2._x + v1._y*v2._y);
}

Vecteur::Vecteur() {
    _x = 0;
    _y = 0;
    _z = 0;
    _t = 0;
}

Vecteur::Vecteur(float x, float y, float z, float t) {
    _x = x;
    _y = y;
    _z = z;
    _t = t;
}


Vecteur::Vecteur(Point &point1, Point &point2) {

    _x = distance_between(point1._lat, point1._lon, point1._lat, point2._lon);

    _y = distance_between(point1._lat, point1._lon, point2._lat, point1._lon);

    // on repasse de la distance (toujours +) a une coordonnee
    if (point2._lat < point1._lat) {
        _y = -_y;
    }
    if (point2._lon < point1._lon) {
        _x = -_x;
    }

    _z = 0;
    _t = 0;
}

float Vecteur::getNorm(void) {
	float _norm_sq = _x*_x + _y*_y;
	return my_sqrtf(_norm_sq);
}

void Vecteur::project(Vecteur const &vecteur_) {
	_x *= vecteur_._x;
	_y *= vecteur_._y;
}

void Vecteur::norm(void) {
	float _norm_sq = this->getNorm();
	if (_norm_sq < 0.001) return;
	_x /= my_sqrtf(_norm_sq);
	_y /= my_sqrtf(_norm_sq);
}

Vecteur Vecteur::operator=(const Point *point) {
    if (point != 0) {
        return Vecteur(point->_lon, point->_lat, point->_alt, point->_rtime);
    } else {
        return Vecteur();
    }
}

