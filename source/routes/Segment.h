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
#include <vector>
#include "WString.h"

using namespace std;

#define MARGE_DESACT  1.f
#define MARGE_ACT     1.5f
#define DIST_ACT     50.f

#define PSCAL_LIM    0.f

#define DIST_ALLOC 300.f

#define MIN_POINTS 12

#define CAD_SPD_PW_LIM 4.f

#define FACTOR 100000.f

#define SEG_FIN    -5
#define SEG_OFF     0
#define SEG_START   1
#define SEG_ON      2

class sSegmentData {
public:
	sSegmentData(void);
	~sSegmentData(void);
    float _elevTot;
    float _monStart;
    float _monCur;
    float _monAvance;
    float _monElev0;
    float _monPElev;
    float _monPDist;
    ListePoints _lpts;
};


class Segment {
  public:
    Segment(void);
    ~Segment(void);
    Segment(const char *nom_seg);

    bool init(void);
    void uninit(void);

    const char* getName();
    void setSegmentName(const char *name_);
    void emptyName();

    void desallouerPoints(void);
    int longueur();
    int isValid();
    static bool nomCorrect(const char *name);

    int8_t getStatus() {return _actif;}
    void setStatus(int8_t act) {_actif = act; return;}

    float getAvance();
    float getCur();
    float getTempsTot();

    int getScore(void);

    void ajouterPointFin(float lat, float lon, float alt, float msec);
    void ajouterPointDebutIso(float lat, float lon, float alt, float msec);

    void toString();

    Vecteur posAuSegment(Point point);
    int testActivation(ListePoints& liste);
    int testDesactivation(ListePoints& liste);

    ListePoints *getListePoints();

    void majPerformance(ListePoints& mes_points);

    Point *getFirstPoint();
    float dist(Point *p);
    Vecteur deltaListe();
    Point2D centerListe();

  private:
    String _nomFichier; // contient la pos du 1er point et le nom
    int8_t _actif;
    sSegmentData *m_p_data = nullptr;
};

/**
 *
 */
class ListeSegments {
  public:
    ListeSegments(void){};

    void push_back(Segment seg) {_segs.push_back(seg);};

    int size() {return _segs.size();}

    void clear(void) {_segs.clear();}

    std::vector<Segment> _segs;
  private:
};



#endif	/* SEGMENT_H */

