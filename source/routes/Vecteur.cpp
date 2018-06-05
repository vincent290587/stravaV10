/* 
 * File:   Vecteur.cpp
 * Author: vincent
 * 
 * Created on October 27, 2015, 3:07 PM
 */

#include "Vecteur.h"


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


Vecteur::Vecteur(Point point1, Point point2) {

    _x = distance_between(point1._lat, point1._lon, point1._lat, point2._lon);

    _y = distance_between(point1._lat, point1._lon, point2._lat, point1._lon);

    if (point2._lat < point1._lat) {
        _y = -_y;
    }
    if (point2._lon < point1._lon) {
        _x = -_x;
    }

    _z = 0;
    _t = 0;
}



Vecteur Vecteur::operator=(const Point *point) {
    if (point != 0) {
        return Vecteur(point->_lon, point->_lat, point->_alt, point->_rtime);
    } else {
        return Vecteur();
    }
}

