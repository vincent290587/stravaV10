
#include "Parcours.h"


ListeParcours::ListeParcours(void) {
	_parcs.clear();
}

Parcours *ListeParcours::getParcoursAt(int i) {

	std::list<Parcours>::iterator _iter;

	if (i >= this->size() || (i < 0 && 1 - i < 0)) {
		return nullptr;
	}

	_iter = _parcs.begin();
	for (int j=1; j <= i; j++) {
		_iter++;
	}

	return _iter.operator->();
}


/**
 *
 * @param nom
 * @return
 */
static bool _nomCorrect(String nom) {

	if (nom.length() == 0) {
		return false;
	}

	if (!nom.endsWith(".PAR")) {
		return false;
	}

	return true;
}

/**
 *
 */
Parcours::Parcours(void) {
	_nomFichier = "";
	_lpts.removeAll();
}

/**
 *
 * @param nom_seg
 */
Parcours::Parcours(const char *nom_seg) {
	if (nom_seg)
		_nomFichier = nom_seg;

	_lpts.removeAll();
}

/**
 *
 * @return
 */
Point2D *Parcours::getFirstPoint() {
	return _lpts.getFirstPoint();
}

/**
 *
 * @param name_
 */
void Parcours::setParcoursName(const char *name_) {

	if (name_)
		_nomFichier = name_;

	return;
}

/**
 *
 */
void Parcours::emptyName() {
	_nomFichier = "";
}

/**
 *
 * @return
 */
const char* Parcours::getName() {
	return _nomFichier.c_str();
}

/**
 *
 * @return
 */
int Parcours::isValid() {
	if (_nomFichier.length() > 0) {
		return 1;
	} else {
		return 0;
	}

}

/**
 *
 */
void Parcours::desallouerPoints() {

	_lpts.removeAll();

	return;
}

/**
 *
 * @param lat
 * @param lon
 * @param alt
 */
void Parcours::ajouterPointFin(float lat, float lon, float alt) {

	_lpts.ajouteFin(lat, lon);

	return;
}

/**
 *
 * @param lat
 * @param lon
 */
//void Parcours::ajouterPointDebutIso(float lat, float lon) {
//
//	_lpts.ajouteDebut(lat, lon);
//	_lpts.removeLast();
//
//}

/**
 *
 */
void Parcours::toString() {

	//printf("Parcours:\nName: %s\nSize: %d\n", _nomFichier.c_str(), _lpts.longueur());

}

/**
 *
 * @param point
 */
void Parcours::updatePosAuParcours(Point point) {
	_lpts.updateRelativePosition(point);
}

/**
 *
 */
void Parcours::updateDelta() {
	_lpts.updateDelta();
}

/**
 *
 * @param lat_
 * @param lon_
 * @return
 */
float Parcours::getDistanceP1(float lat_, float lon_) {
	return _lpts.distP1(lat_, lon_);
}

/**
 *
 * @param name_
 * @return
 */
bool Parcours::nomCorrect(const char *name_) {

	String name = name_;
	return _nomCorrect(name);
}

