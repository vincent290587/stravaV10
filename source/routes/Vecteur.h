/* 
 * File:   Vecteur.h
 * Author: vincent
 *
 * Created on October 27, 2015, 3:07 PM
 */

#ifndef VECTEUR_H
#define	VECTEUR_H


#include <list>
#include "Points.h"

class Vecteur {
public:
    Vecteur() ;
    Vecteur(float x, float y, float z, float t) ;
    Vecteur(Point& point1, Point& point2);

    Vecteur operator=(const Point *point);
    Vecteur operator/(const float cst);

    float getNorm();
    void project(Vecteur const &vecteur_);
    void norm(void);

public:
    float _x;
    float _y;
    float _z;
    float _t;
};

Vecteur Project(Vecteur const &v1, Vecteur const &v2);

float ScalarProduct(Vecteur const &v1, Vecteur const &v2);


#endif	/* VECTEUR_H */

