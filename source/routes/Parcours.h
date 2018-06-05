/*
   File:   Parcours.h
   Author: vincent

   Created on March 28, 2016, 14:32 PM
 */

#ifndef PARCOURS_H
#define	PARCOURS_H

#include <list>
#include "ListePoints.h"
#include "Vecteur.h"
#include "WString.h"

using namespace std;


class Parcours {
public:
	Parcours(void);
	Parcours(const char *nom_seg);

	int isValid();

	void setParcoursName(const char *name_);
	const char* getName();
	void emptyName();

	static bool nomCorrect(const char *name_);

	void desallouerPoints(void);
	int  longueur() {
		return _lpts.size();
	}

	void ajouterPointFin(float lat, float lon, float alt=0);
	//void ajouterPointDebutIso(float lat, float lon);

	void toString();

	float getDistanceP1(float lat_, float lon_);

	void updatePosAuParcours(Point p);
	void updateDelta();

	ListePoints2D *getListePoints() {
		return &_lpts;
	}

	Point2D *getFirstPoint();
	float computeDistance(Point *p) {
		return _lpts.dist(p);
	}

private:
	String _nomFichier; // contient la pos du 1er point et le nom
	ListePoints2D _lpts;
};

/**
 *
 */
 class ListeParcours {
 public:
	ListeParcours(void);
	void push_back(Parcours prc) {
		_parcs.push_back(prc);
	};
	Parcours *getParcoursAt(int i);
	Parcours *getLastParcours(void) {
		return &_parcs.back();
	}
	int size() {
		return _parcs.size();
	}

	std::list<Parcours> _parcs;
 private:
 };



#endif	/* PARCOURS_H */

