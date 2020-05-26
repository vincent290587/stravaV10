/* 
 * File:   ListePoints.cpp
 * Author: vincent
 * 
 * Created on October 15, 2015, 3:30 PM
 */

#include "utils.h"
#include "math_wrapper.h"
#include "Vecteur.h"
#include "ListePoints.h"
#include "assert_wrapper.h"
#include "segger_wrapper.h"

/**
 *
 */
ListePoints::ListePoints() {
	ind_P1 = 0;
	m_dist = 0;
    m_cur_dist = 0;
}

/** @brief Adds a point at the beginning
 *
 * @param lat
 * @param lon
 * @param alt
 * @param msec
 */
void ListePoints::ajouteDebut(float lat, float lon, float alt, float msec) {
	m_lpoints.push_front(Point(lat, lon, alt, msec));
}

/** @brief Adds a point at the end
 *
 * @param lat
 * @param lon
 * @param alt
 * @param msec
 */
void ListePoints::ajouteFin(float lat, float lon, float alt, float msec) {
	m_lpoints.push_back(Point(lat, lon, alt, msec));
}

/**  @brief Adds a point at the end, keeps the number of points under s_max
 *
 * @param lat
 * @param lon
 * @param alt
 * @param msec
 * @param s_max
 */
void ListePoints::ajouteFinIso(float lat, float lon, float alt, float msec, uint16_t s_max) {
	m_lpoints.push_front(Point(lat, lon, alt, msec));
	while (m_lpoints.size() > s_max) {
		m_lpoints.pop_back();
	}
}

/** @brief Adds a point at the end, keeps the number of points under NB_RECORDING
 *
 * @param lat
 * @param lon
 * @param alt
 * @param msec
 */
void ListePoints::enregistrePos(float lat, float lon, float alt, float msec) {
	ajouteFinIso(lat, lon, alt, msec, NB_RECORDING);
}

int ListePoints::size() {
	return m_lpoints.size();
}

void ListePoints::removeAll() {
	m_lpoints.clear();
	return;
}

Point *ListePoints::getFirstPoint() {

	if (!m_lpoints.size()) return NULL;

    return &m_lpoints.front();
}

Point *ListePoints::getLastPoint() {
	return &m_lpoints.back();
}

Point *ListePoints::getPointAt(int i) {
	std::list<Point>::iterator _iter;
	int j;

	if (i >= this->size() || (i < 0 && 1 - i < 0)) {
		return 0;
	}

	if (i < 0) {
		// -1 est le dernier point
		_iter = m_lpoints.end();
		_iter--;
		for (j=-1; j > i; j--) {
			_iter--;
		}
	} else {
		_iter = m_lpoints.begin();
		for (j=1; j <= i; j++) {
			_iter++;
		}
	}

	return _iter.operator->();
}

void ListePoints::removeLast() {
	m_lpoints.pop_back();
}

void ListePoints::toString() {

	if (this->size() > 0) {

		for (auto& point : m_lpoints) {
			point.toString();
		}

	} else {
		//printf("Aucun point\n");
	}

}

/** @brief list distance with a point
 *
 * @param p_
 * @return
 */
float ListePoints::dist(Point *p_) {

	float maDist = 100000.;

	if (!p_) return maDist;

	// on cherche la distance min
	for (auto& point : m_lpoints) {
		if (maDist > point.dist(p_)) {
			maDist = point.dist(p_);
		}
	}

	return maDist;
}

/** @brief list distance with a point
 *
 * @param lat_
 * @param lon_
 * @return
 */
float ListePoints::dist(float lat_, float lon_) {

	Point p_;

	p_._lat = lat_;
	p_._lon = lon_;

	return this->dist(&p_);
}

/** @brief Distance with the stored closest point
 *
 * @param lat_
 * @param lon_
 * @return
 */
float ListePoints::distP1(float lat_, float lon_) {
	return m_P1.dist(lat_, lon_);
}

/**
 * Updates the list delta
 */
void ListePoints::updateDelta() {

	int indice = 0;
	Point tmpPT1;
	float tmp_dist = 0.;
	float min_lon = 100;
	float max_lon = -100;
	float min_lat = 200;
	float max_lat = -200;

	if (m_lpoints.size() < 2) return;

	for (auto& tmpPT2 : m_lpoints) {

		if (indice != 0) {

			// distance
			tmp_dist += tmpPT1.dist(tmpPT2);

			// minimaux
			if (tmpPT2._lon < min_lon) min_lon = tmpPT2._lon;
			if (tmpPT2._lat < min_lat) min_lat = tmpPT2._lat;

			// maximaux
			if (max_lon < tmpPT2._lon) max_lon = tmpPT2._lon;
			if (max_lat < tmpPT2._lat) max_lat = tmpPT2._lat;

		} else {
			min_lat = tmpPT2._lat;
			max_lat = tmpPT2._lat;
			min_lon = tmpPT2._lon;
			max_lon = tmpPT2._lon;
		}

		tmpPT1 = tmpPT2;
		indice++;
	}

	m_delta_l._x = max_lon - min_lon;
	m_delta_l._y = max_lat - min_lat;
	m_delta_l._z = getElevTot();
	m_delta_l._t = getTempsTot();

	m_center_l._lat = 0.5*(max_lat + min_lat);
	m_center_l._lon = 0.5*(max_lon + min_lon);

	m_dist = tmp_dist;

}

/** @brief Updates P1/P2 and the stored relative position
 *
 * @param point
 */
void ListePoints::updateRelativePosition(Point& point) {

	Point P1, P2;
	float p1p2_dist, tmp_dist, distP1_=0., distP2_=0.;
	int init = 0;
	uint16_t tmp_ind = 0;

	if (m_lpoints.size() < 5) {
		return;
	}

	// on cherche les deux plus proches points
	for (auto& tmpPT : m_lpoints) {

		tmp_ind++;
		tmp_dist = tmpPT.dist(&point);

		if (init == 0) {
			P1 = tmpPT;
			distP1_ = P1.dist(&point);
			init++;
		} else if (init == 1) {
			P2 = tmpPT;
			distP2_ = P2.dist(&point);
			init++;

			if (distP1_ > distP2_) {
				// invert them
				P2 = P1;
				distP2_ = distP1_;

				P1 = tmpPT;
				distP1_ = P1.dist(&point);
			}
		} else {

			if (tmp_dist < distP1_) {
				P2 = P1;
				distP2_ = P2.dist(&point);

				P1 = tmpPT;
				distP1_ = P1.dist(&point);

				ind_P1 = tmp_ind;
			} else if (tmp_dist < distP2_) {
				P2 = tmpPT;
				distP2_ = P2.dist(&point);
			}
		}
	}

	m_P1 = P1;
	m_P2 = P2;

	p1p2_dist = P1.dist(&P2);

	if (distP1_ > 50. ||
			(distP2_*distP2_ >= p1p2_dist*p1p2_dist + distP1_*distP1_)) {
		/* out of triangle, P1 is always closest
		 *                              P
		 *
		 *      P2---------------P1
		 *
		 */
		m_pos_r._x = 0.;
		m_pos_r._y = 0.;
		m_pos_r._z = P1._alt;
		m_pos_r._t = P1._rtime;
	} else {
		// inside the triangle
		Vecteur P1P, P1P2;

		// construction des vecteurs
		P1P = Vecteur(P1, point);
		P1P2 = Vecteur(P1, P2);

		// project on P1P2
		m_pos_r._x = ScalarProduct(P1P, P1P2) / P1P2.getNorm();

		Vecteur orthoP1P2;
		orthoP1P2._x = P1P2._y;
		orthoP1P2._y = -P1P2._x;

		// project on P1P2 orthogonal vector
		m_pos_r._y = ScalarProduct(P1P, orthoP1P2) / orthoP1P2.getNorm();

		m_pos_r._z = P1._alt + (P2._alt - P1._alt) * m_pos_r._x / P1P2.getNorm();
		m_pos_r._t = P1._rtime + (P2._rtime - P1._rtime) * m_pos_r._x / P1P2.getNorm();

	}


}

/** @gives the relative position as a vector in the referential P1/P2 as x
 *
 * @param point
 * @return
 */
Vecteur ListePoints::computePosRelative(Point point) {

	Point P1, P2;
	float p1p2_dist, tmp_dist, distP1_=0., distP2_=0.;
	int init = 0;
	Vecteur res;

	if (m_lpoints.size() < 5) {
		return res;
	}


	// on cherche les deux plus proches points
	for (auto& tmpPT : m_lpoints) {

		tmp_dist = tmpPT.dist(&point);

		if (init == 0) {
			P1 = tmpPT;
			distP1_ = P1.dist(&point);
			init++;
		} else if (init == 1) {
			P2 = tmpPT;
			distP2_ = P2.dist(&point);
			init++;

			if (distP1_ > distP2_) {
				// invert them
				P2 = P1;
				distP2_ = distP1_;

				P1 = tmpPT;
				distP1_ = P1.dist(&point);
			}
		} else {

			if (tmp_dist < distP1_) {
				P2 = P1;
				distP2_ = P2.dist(&point);

				P1 = tmpPT;
				distP1_ = P1.dist(&point);
			} else if (tmp_dist < distP2_) {
				P2 = tmpPT;
				distP2_ = P2.dist(&point);
			}
		}
	}

	m_P1 = P1;
	m_P2 = P2;

	p1p2_dist = P1.dist(&P2);

	if (distP1_ > 50. ||
			(distP2_*distP2_ >= p1p2_dist*p1p2_dist + distP1_*distP1_)) {
		/* out of triangle, P1 is always closest
		 *                              P
		 *
		 *      P2---------------P1
		 *
		 */
		res._x = 0.;
		res._y = 0.;
		res._z = P1._alt;
		res._t = P1._rtime;
	} else {
		// inside the triangle
		Vecteur P1P, P1P2;

		// construction des vecteurs
		P1P = Vecteur(P1, point);
		P1P2 = Vecteur(P1, P2);

		// project on P1P2
		res._x = ScalarProduct(P1P, P1P2) / P1P2.getNorm();

		Vecteur orthoP1P2;
		orthoP1P2._x = P1P2._y;
		orthoP1P2._y = -P1P2._x;

		// project on P1P2 orthogonal vector
		res._y = ScalarProduct(P1P, orthoP1P2) / orthoP1P2.getNorm();

		res._z = P1._alt + (P2._alt - P1._alt) * res._x / P1P2.getNorm();
		res._t = P1._rtime + (P2._rtime - P1._rtime) * res._x / P1P2.getNorm();

	}

	return res;
}

/**
 *
 * @return The stored list delta
 */
Vecteur& ListePoints::getDeltaListe() {
	return m_delta_l;
}

/**
 *
 * @return The stored list center
 */
Point2D& ListePoints::getCenterListe() {
    return m_center_l;
}

/**
 *
 * @return The stored list relative position
 */
Vecteur ListePoints::getPosRelative() {
	return m_pos_r;
}

/**
 *
 * @return the list dt
 */
float ListePoints::getTempsTot() {
	return m_lpoints.back()._rtime - m_lpoints.front()._rtime;
}

/**
 *
 * @return The list dh
 */
float ListePoints::getElevTot() {
	return m_lpoints.back()._alt - m_lpoints.front()._alt;
}

ListePoints2D::ListePoints2D() {
    m_dist = 0.;
    m_cur_dist = 0.;
}

void ListePoints2D::ajouteFin(float lat, float lon) {
	m_lpoints.push_back(Point2D(lat, lon));
}

int ListePoints2D::size() {
	return m_lpoints.size();
}

void ListePoints2D::removeAll() {
	m_lpoints.clear();
}

Point2D* ListePoints2D::getFirstPoint() {

	if (!m_lpoints.size()) return NULL;

	return &m_lpoints.front();
}

Point2D* ListePoints2D::getLastPoint() {
	return &m_lpoints.back();
}

Point2D* ListePoints2D::getPointAt(int i) {
	std::list<Point2D>::iterator _iter;
	int j;

	if (i >= this->size() || (i < 0 && 1 - i < 0)) {
		return 0;
	}

	if (i < 0) {
		// -1 est le dernier point
		_iter = m_lpoints.end();
		_iter--;
		for (j=-1; j > i; j--) {
			_iter--;
		}
	} else {
		_iter = m_lpoints.begin();
		for (j=1; j <= i; j++) {
			_iter++;
		}
	}

	return _iter.operator->();
}

Vecteur ListePoints2D::computePosRelative(Point point) {

	Point P1, P2;
	float p1p2_dist, tmp_dist, distP1_=0., distP2_=0.;
	int init = 0;
	Vecteur res, PP1, P1P2;

	if (m_lpoints.size() < 5) {
		return res;
	}

	// on cherche les deux plus proches points
	for (auto& tmpPT : m_lpoints) {

		tmp_dist = tmpPT.dist(&point);

		if (init == 0) {
			P1 = tmpPT;
			distP1_ = P1.dist(&point);
			init++;
		} else if (init == 1) {
			P2 = tmpPT;
			distP2_ = P2.dist(&point);
			init++;

			if (distP1_ > distP2_) {
				// invert them
				P2 = P1;
				distP2_ = distP1_;

				P1 = tmpPT;
				distP1_ = P1.dist(&point);
			}
		} else {

			if (tmp_dist < distP1_) {
				P2 = P1;
				distP2_ = P2.dist(&point);

				P1 = tmpPT;
				distP1_ = P1.dist(&point);
			} else if (tmp_dist < distP2_) {
				P2 = tmpPT;
				distP2_ = P2.dist(&point);
			}
		}
	}

	p1p2_dist = P1.dist(&P2);

	if (distP1_ > 50. ||
			(distP2_*distP2_ >= p1p2_dist*p1p2_dist + distP1_*distP1_)) {
		/* out of triangle, P1 is always closest
		 *                              P
		 *
		 *      P2---------------P1
		 *
		 */
		res._x = 0.;
		res._y = 0.;
	} else {
		// inside the triangle
		Vecteur P1P, P1P2;

		// construction des vecteurs
		P1P = Vecteur(P1, point);
		P1P2 = Vecteur(P1, P2);

		// project on P1P2
		res._x = ScalarProduct(P1P, P1P2) / P1P2.getNorm();

		Vecteur orthoP1P2;
		orthoP1P2._x = P1P2._y;
		orthoP1P2._y = -P1P2._x;

		// project on P1P2 orthogonal vector
		res._y = ScalarProduct(P1P, orthoP1P2) / orthoP1P2.getNorm();

	}

	return res;
}

void ListePoints2D::updateDelta() {

	int indice = 0;
	Point tmpPT1;
	float tmp_dist = 0.;
	float min_lon = 100;
	float max_lon = -100;
	float min_lat = 200;
	float max_lat = -200;

	if (m_lpoints.size() < 2) return;

	for (auto& tmpPT2 : m_lpoints) {

		if (indice != 0) {

			// distance
			tmp_dist += tmpPT1.dist(tmpPT2);

			// minimaux
			if (tmpPT2._lon < min_lon) min_lon = tmpPT2._lon;
			if (tmpPT2._lat < min_lat) min_lat = tmpPT2._lat;

			// maximaux
			if (max_lon < tmpPT2._lon) max_lon = tmpPT2._lon;
			if (max_lat < tmpPT2._lat) max_lat = tmpPT2._lat;

		} else {
			min_lat = tmpPT2._lat;
			max_lat = tmpPT2._lat;
			min_lon = tmpPT2._lon;
			max_lon = tmpPT2._lon;
		}

		tmpPT1 = tmpPT2;
		indice++;
	}

	m_delta_l._x = max_lon - min_lon;
	m_delta_l._y = max_lat - min_lat;
	m_delta_l._z = 0;
	m_delta_l._t = 0;

	m_center_l._lat = 0.5*(max_lat + min_lat);
	m_center_l._lon = 0.5*(max_lon + min_lon);

	m_dist = tmp_dist;

}

void ListePoints2D::updateRelativePosition(Point& point) {

	Point P1, P2;
	float p1p2_dist, tmp_dist, distP1_=0., distP2_=0.;
	uint16_t init = 0;
	uint16_t tmp_idx = 0;

	if (m_lpoints.size() < 5) {
		return;
	}

	// on cherche les deux plus proches points
	idx_P1 = 0;
	for (auto& tmpPT : m_lpoints) {

		tmp_idx++;
		tmp_dist = tmpPT.dist(&point);

		if (init == 0) {
			P1 = tmpPT;
			distP1_ = P1.dist(&point);
			init++;
		} else if (init == 1) {
			P2 = tmpPT;
			distP2_ = P2.dist(&point);
			init++;

			if (distP1_ > distP2_) {
				// invert them
				P2 = P1;
				distP2_ = distP1_;

				P1 = tmpPT;
				distP1_ = P1.dist(&point);
			}
		} else {

			if (tmp_dist < distP1_) {
				P2 = P1;
				distP2_ = P2.dist(&point);

				idx_P1 = tmp_idx;

				P1 = tmpPT;
				distP1_ = P1.dist(&point);
			} else if (tmp_dist < distP2_) {
				P2 = tmpPT;
				distP2_ = P2.dist(&point);
			}
		}
	}

	m_P1 = P1;
	m_P2 = P2;

	p1p2_dist = P1.dist(&P2);

	if (distP1_ > 50. ||
			(distP2_*distP2_ >= p1p2_dist*p1p2_dist + distP1_*distP1_)) {
		/* out of triangle, P1 is always closest
		 *                              P
		 *
		 *      P2---------------P1
		 *
		 */
		m_pos_r._x = 0.;
		m_pos_r._y = 0.;
		m_pos_r._z = P1._alt;
		m_pos_r._t = P1._rtime;
	} else {
		// inside the triangle
		Vecteur P1P, P1P2;

		// construction des vecteurs
		P1P = Vecteur(P1, point);
		P1P2 = Vecteur(P1, P2);

		// project on P1P2
		m_pos_r._x = ScalarProduct(P1P, P1P2) / P1P2.getNorm();

		Vecteur orthoP1P2;
		orthoP1P2._x = P1P2._y;
		orthoP1P2._y = -P1P2._x;

		// project on P1P2 orthogonal vector
		m_pos_r._y = ScalarProduct(P1P, orthoP1P2) / orthoP1P2.getNorm();

		m_pos_r._z = P1._alt + (P2._alt - P1._alt) * m_pos_r._x / P1P2.getNorm();
		m_pos_r._t = P1._rtime + (P2._rtime - P1._rtime) * m_pos_r._x / P1P2.getNorm();

	}

}

Vecteur& ListePoints2D::getDeltaListe() {
	return m_delta_l;
}

Vecteur ListePoints2D::getPosRelative() {
	return m_pos_r;
}

Point2D& ListePoints2D::getCenterListe() {
    return m_center_l;
}

float ListePoints2D::dist(Point2D* p_) {

	ASSERT(p_);

	if (!p_) return 100000.;

	return this->dist(p_->_lat, p_->_lon);
}

float ListePoints2D::dist(Point* p_) {

	ASSERT(p_);

	if (!p_) return 100000.;

	return this->dist(p_->_lat, p_->_lon);
}

float ListePoints2D::dist(float lat_, float lon_) {

	float maDist = 100000.;

	// on cherche la distance min
	for (auto& point : m_lpoints) {
		float tmp_dist = point.dist(lat_, lon_);
		if (maDist > tmp_dist) {
			maDist = tmp_dist;
		}
	}

	return maDist;
}

float ListePoints2D::distP1(float lat_, float lon_) {
	return m_P1.dist(lat_, lon_);
}
