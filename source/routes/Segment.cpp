
#include "math.h"
#include "Segment.h"
#include "segger_wrapper.h"



static bool _nomCorrect(String nom) {

	int i;

	if (nom.length() == 0)
		return false;

	if (!strstr(nom.c_str(), "#") || !strstr(nom.c_str(), ".")) {
		return false;
	}

	if (strlen(nom.c_str()) != 12) {
		return false;
	}

	for (i = 0; i < 12; i++) {

		if (i == 5) {
			if (nom.c_str()[i] != '#')
				return false;
		} else if (i == 8) {
			if (nom.c_str()[i] != '.')
				return false;
		} else {
			if (nom.c_str()[i] < '0' || nom.c_str()[i] > 'Z' || (nom.c_str()[i] > '9' && nom.c_str()[i] < 'A')) {
				return false;
			}
		}
	}

	return true;
}


bool Segment::nomCorrect(const char *chaine) {

	String nom = chaine;
	return _nomCorrect(nom);
}


Segment::Segment(void) {
	_actif = 0;
	_elevTot = 0.;
	_monStart = 0.;
	_monCur = 0.;
	_monAvance = 0.;
	_monElev0 = 0.;
	_monPElev = 0.;
}

Segment::Segment(const char *nom_seg) : Segment() {
	_actif = 0;
	if (nom_seg)
		_nomFichier = nom_seg;
}

/**
 * Init the list delta and center
 */
void Segment::init(void) {

	_lpts.updateDelta();

}

Point *Segment::getFirstPoint() {
	return _lpts.getFirstPoint();
}

void Segment::setSegmentName(const char *name_) {

	if (name_)
		_nomFichier = name_;

	return;
}

void Segment::emptyName() {
	_nomFichier = "";
}


const char* Segment::getName() {
	return _nomFichier.c_str();
}

int Segment::isValid() {
	if (_nomFichier.length() > 0) {
		return 1;
	} else {
		return 0;
	}

}

void Segment::desallouerPoints() {

	_lpts.removeAll();

	return;
}


void Segment::ajouterPointFin(float lat, float lon, float alt, float msec) {

	_lpts.ajouteFin(lat, lon, alt, msec);

	return;
}

void Segment::ajouterPointDebutIso(float lat, float lon, float alt, float msec) {

	_lpts.ajouteDebut(lat, lon, alt, msec);
	_lpts.removeLast();

}



void Segment::toString() {

	//printf("Segment:\nName: %s\nSize: %d\n", _nomFichier.c_str(), _lpts.longueur());
	_lpts.toString();

}

Vecteur Segment::posAuSegment(Point point) {
	return _lpts.computePosRelative(point);
}

int Segment::testActivation(ListePoints& liste) {

	float distP1P2, distP1, distP2, p_scal, distQuad;
	Point P1, P2, PPc, PPp;
	Vecteur PC, PS;

	if (_lpts.size() <= 3 || liste.size() <= 2) {
		return 0;
	}

	// position courante / premier point
	Point *test = liste.getFirstPoint();
	PPc = test;
	P1 = _lpts.getFirstPoint();

	distQuad = P1.dist(&PPc);
	if (distQuad > DIST_ACT) return 0;

	// second point
	PPp = liste.getPointAt(1);
	P2 = _lpts.getPointAt(1);

	distP1 = P1.dist(&PPc);
	distP2 = P2.dist(&PPc);
	distP1P2 = P1.dist(&P2);

	PC = Vecteur(PPp, PPc);
	PS = Vecteur(P1, P2);

	p_scal = PC._x * PS._x + PC._y * PS._y;

	if (sqrt(PC._x * PC._x + PC._y * PC._y) * sqrt(PS._x * PS._x + PS._y * PS._y) > 0.001) {
		p_scal /= sqrt(PC._x * PC._x + PC._y * PC._y);
		p_scal /= sqrt(PS._x * PS._x + PS._y * PS._y);
	} else {
		p_scal = -10.;
	}

	if (distP2 * distP2 < distP1 * distP1 + distP1P2 * distP1P2 && p_scal > PSCAL_LIM) {
		return 1;
	} else {
		return 0;
	}

}

/**
 *
 * @param liste Mon historique de positions
 * @return 1 si desactivable, 0 sinon
 */
int Segment::testDesactivation(ListePoints& liste) {

	float distP1P2, distP1, distP2;
	Point *P1, *P2, *PPc;

	if (_lpts.size() <= 3 || liste.size() <= 2) {
		return 0;
	}

	// position courante
	PPc = liste.getFirstPoint();

	// avant-dernier point
	P1 = _lpts.getPointAt(-2);
	// dernier point
	P2 = _lpts.getPointAt(-1);

	distP2 = P2->dist(PPc);

	// trick to make it go faster
	if (distP2 > DIST_ACT) return 0;

	distP1 = P1->dist(PPc);
	distP1P2 = P1->dist(P2);

	// pythagore
	if (distP1 * distP1 > distP2 * distP2 + distP1P2 * distP1P2 && (distP1 < DIST_ACT ||  distP2 < DIST_ACT)) {
		return 1;
	}
	else {
		return 0;
	}

}

/**
 *
 * @param mes_points List of the last user's GPS positions
 */
void Segment::majPerformance(ListePoints& mes_points) {

	int activable, desactivable;
	Point pc;
	Vecteur vect;

	if (mes_points.size() < 2) {
		//    loggerMsg("Historique insuffisant");
		return;
	}

	pc = mes_points.getFirstPoint();

	if (!pc.isValid()) {
		//    loggerMsg("Premier point invalide");
		return;
	}

	activable = testActivation(mes_points);

	Vecteur& delta = _lpts.getDeltaListe();

	// update the relative position of our last point on the segment
	_lpts.updateRelativePosition(pc);

	if (_actif == SEG_OFF) {
		if (activable > 0) {
			// premiere interpolation avec notre historique de points
			pc = this->getFirstPoint();
			vect = mes_points.computePosRelative(pc);

			_monStart = vect._t;
			_monCur = vect._t;

			_monElev0 = vect._z;
			_monPElev = 0.;
			_elevTot = delta._z;
			_monAvance = 0.;
			_actif = SEG_START;

		}
	} else if (_actif > SEG_OFF) {
		// deja actif
		if (_actif == SEG_START) {
			_actif = SEG_ON;
		}

		desactivable = testDesactivation(mes_points);

		if (desactivable == 0) {

			vect = _lpts.getPosRelative();

			if (fabs(vect._y) < MARGE_ACT * DIST_ACT) {

				_monCur = vect._t;
				_monAvance = _monStart + vect._t - pc._rtime;

				_monElev0 = _lpts.getFirstPoint()->_alt;

				if (_elevTot > 5.) {
					_monPElev = vect._z;
					_monPElev -= pc._alt;
					_monPElev /= _elevTot;
				}

			} else {
				//        loggerMsg("Desactivation pendant segment de ");
				//        loggerMsg(_nomFichier.c_str());

				_actif = SEG_OFF;

				//        display.notifyANCS(1, "SEG", "Seg desactive");
			}

		} else {
			// on doit desactiver

			Point lp = *_lpts.getLastPoint();

			if (!lp.isValid()) {
				//        loggerMsg("Dernier point invalide !!!!!");
				//        Serial.print(F("Dernier point invalide !!!!!"));
				desallouerPoints();
				_actif = SEG_OFF;

				//display.notifyANCS(1, "SEG", "Dernier point invalide");
				return;
			}

			// position relative du dernier point segment / mes points
			vect = mes_points.computePosRelative(lp);

			_monCur = vect._t - _monStart;
			_monAvance = delta._t - _monCur;

			_actif = SEG_FIN;

		}

	} else if (_actif < SEG_OFF) {
		_actif += 1;
	}
}


