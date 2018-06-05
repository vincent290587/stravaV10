/*
 * File:   Segment.h
 * Author: vincent
 *
 * Created on October 7, 2015, 3:04 PM
 */

#ifndef SEGMENT_H
#define	SEGMENT_H

#include "ListePoints.h"
#include "Vecteur.h"

#include <stdint.h>
#include <list>
#include "WString.h"

using namespace std;

#define MARGE_DESACT  2.
#define MARGE_ACT     1.5
#define DIST_ACT     50.

#define PSCAL_LIM    0.

#define DIST_ALLOC 300.

#define MIN_POINTS 12

#define CAD_SPD_PW_LIM 4.

#define FACTOR 100000.

#define SEG_FIN    -5
#define SEG_OFF     0
#define SEG_START   1
#define SEG_ON      2



class Segment {
  public:
    Segment(void);
    Segment(const char *nom_seg);

    void init(void);

    const char* getName();
    void setSegmentName(const char *name_);
    void emptyName();

    void desallouerPoints(void);
    int  longueur() {return _lpts.size();}
    int isValid();
    static bool nomCorrect(const char *name);

    int8_t getStatus() {return _actif;}
    void setStatus(int8_t act) {_actif = act; return;}

    float getAvance() {return _monAvance;}
    float getCur() {return _monCur;}
    float getTempsTot() {return _lpts.getTempsTot();}

    void ajouterPointFin(float lat, float lon, float alt, float msec);
    void ajouterPointDebutIso(float lat, float lon, float alt, float msec);

    void toString();

    Vecteur posAuSegment(Point point);
    int testActivation(ListePoints& liste);
    int testDesactivation(ListePoints& liste);

    ListePoints *getListePoints() {return &_lpts;}

    void majPerformance(ListePoints& mes_points);

    Point *getFirstPoint();
    float dist(Point *p) {return _lpts.dist(p);}
    Vecteur deltaListe() {return _lpts.getDeltaListe();}
    Point2D centerListe() {return _lpts.getCenterListe();}

  private:
    String _nomFichier; // contient la pos du 1er point et le nom
    int8_t _actif;
    float _elevTot;
    float _monStart;
    float _monCur;
    float _monAvance;
    float _monElev0;
    float _monPElev;
    float _monPDist;
    ListePoints _lpts;
};

/**
 *
 */
class ListeSegments {
  public:
    ListeSegments(void){};

    void push_back(Segment seg) {_segs.push_back(seg);};

    int size() {return _segs.size();}

    std::list<Segment> _segs;
  private:
};



#endif	/* SEGMENT_H */

