
#include "assert_wrapper.h"
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



sSegmentData::sSegmentData(void) {
}

sSegmentData::~sSegmentData(void) {
	_lpts.removeAll();
	LOG_DEBUG("List emptied");
}

bool Segment::nomCorrect(const char *chaine) {

	String nom = chaine;
	return _nomCorrect(nom);
}

Segment::Segment(void) {

	_actif = SEG_OFF;
	m_p_data = nullptr;

}

Segment::~Segment(void) {

	if (m_p_data) delete m_p_data;
	m_p_data = nullptr;

}

Segment::Segment(const char *nom_seg) : Segment() {
	if (nom_seg)
		_nomFichier = nom_seg;
}

/**
 * Init the list delta and center
 *
 * @return False if memory failure, true if OK
 */
bool Segment::init(void) {

	if (!m_p_data) m_p_data = new sSegmentData();

	if (m_p_data) m_p_data->_lpts.updateDelta();
	else return false;

	return true;
}

void Segment::uninit(void) {

    if (m_p_data) delete m_p_data;
    m_p_data = nullptr;

}

/**
 *
 * @return
 */
int Segment::longueur() {

	int ret = 0;

	if (m_p_data) ret = m_p_data->_lpts.size();

	return ret;
}

float Segment::getAvance() {

	float ret = 0;

	if (m_p_data) ret = m_p_data->_monAvance;

	return ret;
}

float Segment::getCur() {

	float ret = 0;

	if (m_p_data) ret = m_p_data->_monCur;

	return ret;
}

float Segment::getTempsTot() {

	float ret = 0;

	if (m_p_data) ret = m_p_data->_lpts.getTempsTot();

	return ret;
}

float Segment::dist(Point* p) {

	float ret = 0;

	if (m_p_data) ret = m_p_data->_lpts.dist(p);

	return ret;
}

ListePoints* Segment::getListePoints() {

	ListePoints* ret = nullptr;

	if (m_p_data) ret = &m_p_data->_lpts;

	return ret;
}

Point *Segment::getFirstPoint() {

	if (!m_p_data) return nullptr;

	return m_p_data->_lpts.getFirstPoint();
}

Vecteur Segment::deltaListe() {

	Vecteur ret;

	if (m_p_data) ret = m_p_data->_lpts.getDeltaListe();

	return m_p_data->_lpts.getDeltaListe();
}

Point2D Segment::centerListe() {

	Point2D ret;

	if (m_p_data) ret = m_p_data->_lpts.getCenterListe();

	return ret;
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

	if (m_p_data) m_p_data->_lpts.removeAll();

	return;
}


void Segment::ajouterPointFin(float lat, float lon, float alt, float msec) {

	if (m_p_data) m_p_data->_lpts.ajouteFin(lat, lon, alt, msec);

	return;
}

void Segment::ajouterPointDebutIso(float lat, float lon, float alt, float msec) {

	if (m_p_data) m_p_data->_lpts.ajouteDebut(lat, lon, alt, msec);
	if (m_p_data) m_p_data->_lpts.removeLast();

}



void Segment::toString() {

	//printf("Segment:\nName: %s\nSize: %d\n", _nomFichier.c_str(), data->_lpts.longueur());
	if (m_p_data) m_p_data->_lpts.toString();

}

Vecteur Segment::posAuSegment(Point point) {

	ASSERT(m_p_data);

	return m_p_data->_lpts.computePosRelative(point);
}

int Segment::testActivation(ListePoints& liste) {

	float distP1P2, distP1, distP2, p_scal, distQuad;
	Point P1, P2, PPc, PPp;
	Vecteur PC, PS;

	if (!m_p_data) {
		if (!this->init()) {
			LOG_ERROR("!! Segment no mem !!");
			return 0;
		}
		LOG_ERROR("Segment allocated last minute");
	}

	if (m_p_data->_lpts.size() <= 3 || liste.size() <= 2) {
		return 0;
	}

	// position courante / premier point
	Point *test = liste.getFirstPoint();
	PPc = test;
	P1 = m_p_data->_lpts.getFirstPoint();

	distQuad = P1.dist(&PPc);
	if (distQuad > DIST_ACT) return 0;

	// second point
	PPp = liste.getPointAt(1);
	P2 = m_p_data->_lpts.getPointAt(1);

	distP1 = P1.dist(&PPc);
	distP2 = P2.dist(&PPc);
	distP1P2 = P1.dist(&P2);

	PC = Vecteur(PPp, PPc);
	PS = Vecteur(P1, P2);

	p_scal = PC._x * PS._x + PC._y * PS._y;

	if (sqrtf(PC._x * PC._x + PC._y * PC._y) * sqrtf(PS._x * PS._x + PS._y * PS._y) > 0.001) {
		p_scal /= sqrtf(PC._x * PC._x + PC._y * PC._y);
		p_scal /= sqrtf(PS._x * PS._x + PS._y * PS._y);
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

	ASSERT(m_p_data);

	if (m_p_data->_lpts.size() <= 3 || liste.size() <= 2) {
		return 0;
	}

	// position courante
	PPc = liste.getFirstPoint();

	// avant-dernier point
	P1 = m_p_data->_lpts.getPointAt(-2);
	// dernier point
	P2 = m_p_data->_lpts.getPointAt(-1);

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
		return;
	}

	pc = mes_points.getFirstPoint();

	if (!pc.isValid()) {
		return;
	}

	activable = testActivation(mes_points);

	ASSERT(m_p_data);

	m_p_data->_lpts.updateDelta();
	Vecteur& delta = m_p_data->_lpts.getDeltaListe();

	// update the relative position of our last point on the segment
	m_p_data->_lpts.updateRelativePosition(pc);

	if (_actif == SEG_OFF) {
		if (activable > 0) {
			// premiere interpolation avec notre historique de points
			pc = this->getFirstPoint();
			vect = mes_points.computePosRelative(pc);

			m_p_data->_monStart = vect._t;
			m_p_data->_monCur = vect._t;

			m_p_data->_monElev0 = vect._z;
			m_p_data->_monPElev = 0.;
			m_p_data->_elevTot = delta._z;
			m_p_data->_monAvance = 0.;
			_actif = SEG_START;

		}
	} else if (_actif > SEG_OFF) {
		// deja actif
		if (_actif == SEG_START) {
			_actif = SEG_ON;
		}

		desactivable = testDesactivation(mes_points);

		if (desactivable == 0) {

			vect = m_p_data->_lpts.getPosRelative();

			if (fabsf(vect._y) < MARGE_ACT * DIST_ACT) {

				Point *pp = m_p_data->_lpts.getFirstPoint();

				m_p_data->_monCur = pc._rtime - m_p_data->_monStart;
				m_p_data->_monAvance = (vect._t - pp->_rtime) - m_p_data->_monCur;

				m_p_data->_monElev0 = m_p_data->_lpts.getFirstPoint()->_alt;

				if (m_p_data->_elevTot > 5.) {
					m_p_data->_monPElev = vect._z;
					m_p_data->_monPElev -= pc._alt;
					m_p_data->_monPElev /= m_p_data->_elevTot;
				}

			} else {
				_actif = SEG_OFF;
			}

		} else {
			// on doit desactiver
			Point lp = *m_p_data->_lpts.getLastPoint();

			if (!lp.isValid()) {
				this->desallouerPoints();
				_actif = SEG_OFF;
				return;
			}

			// position relative du dernier point segment / mes points
			vect = mes_points.computePosRelative(lp);

			m_p_data->_monCur = vect._t - m_p_data->_monStart;
			m_p_data->_monAvance = delta._t - m_p_data->_monCur;

			_actif = SEG_FIN;

		}

	} else if (_actif < SEG_OFF) {
		_actif += 1;
	}
}
