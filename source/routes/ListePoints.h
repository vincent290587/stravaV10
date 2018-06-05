/* 
 * File:   ListePoints.h
 * Author: vincent
 *
 * Created on October 15, 2015, 3:30 PM
 */

#ifndef LISTEPOINTS_H
#define	LISTEPOINTS_H


#define NB_RECORDING 9

#include <list>
#include "utils.h"
#include "Points.h"
#include "Vecteur.h"

using namespace std;


class ListePoints2D {
public:
	ListePoints2D();

    void ajouteFin(float lat, float lon);

    int  size();

    void removeAll();

    Point2D *getFirstPoint();
    Point2D *getLastPoint();
    Point2D *getPointAt(int i);

    Vecteur computePosRelative(Point point);

    void updateDelta();
    void updateRelativePosition(Point& point);

    Vecteur& getDeltaListe();
    Vecteur getPosRelative();
    Point2D& getCenterListe();

    float dist(Point2D *p_);
    float dist(Point *p_);
	float dist(float lat_, float lon_);
	float distP1(float lat_, float lon_);

    std::list<Point2D> *getLPTS() {return &m_lpoints;}
    
    Point m_P1, m_P2;

protected:
    std::list<Point2D> m_lpoints;
    Vecteur m_pos_r;
    Vecteur m_delta_l;
    Point2D m_center_l;
    float m_dist;
    float m_cur_dist;
};



class ListePoints {
public:
    ListePoints();

    void ajouteDebut(float lat, float lon, float alt, float msec);
    void ajouteFin(float lat, float lon, float alt, float msec);
    void ajouteFinIso(float lat, float lon, float alt, float msec, uint16_t s_max);

    void enregistrePos(float lat, float lon, float alt, float msec);

    int  size();

    void removeAll();
    void removeLast();

    Point *getFirstPoint();
    Point *getLastPoint();
    Point *getPointAt(int i);

    void toString();

    Vecteur computePosRelative(Point point);

    void updateDelta();
    void updateRelativePosition(Point& point);
    Vecteur& getDeltaListe();
    Vecteur getPosRelative();
    Point2D& getCenterListe();

    float dist(Point *p_);
	float dist(float lat_, float lon_);
	float distP1(float lat_, float lon_);

    float getTempsTot();
    float getElevTot();

    std::list<Point> *getLPTS() {return &m_lpoints;}

    Point m_P1, m_P2;

protected:
    std::list<Point> m_lpoints;
    Vecteur m_pos_r;
    Vecteur m_delta_l;
    Point2D m_center_l;
    float m_dist;
    float m_cur_dist;
};

#endif	/* LISTEPOINTS_H */

