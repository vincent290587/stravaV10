/*
 * Points.cpp
 *
 *  Created on: 15 oct. 2017
 *      Author: Vincent
 */

#include <math.h>
#include "Points.h"


Point2D::Point2D(bool count) : m_count(count) {
	_lat = 0;
	_lon = 0;

	if (m_count) this->increaseCount2D();
}

Point2D::Point2D(float lat, float lon) : Point2D(true) {
	_lat = lat;
	_lon = lon;
}

Point2D::Point2D(const Point2D& pt) : Point2D(true) {
	_lat = pt._lat;
	_lon = pt._lon;
}

Point2D::~Point2D() {
	if (m_count) this->decreaseCount2D();
}

void Point2D::increaseCount2D(void) {
	objectCount2D++;
}

void Point2D::decreaseCount2D(void) {
	objectCount2D--;
}

Point2D & Point2D::operator=(const Point2D *point) {
	if (point != 0) {
		_lat = point->_lat;
		_lon = point->_lon;
		return *this;
	} else {
		return *this;
	}
}

Point2D & Point2D::operator=(const Point2D &point) {
	_lat = point._lat;
	_lon = point._lon;
	return *this;
}

Point::Point() : Point2D(false) {
	_lat = 0;
	_lon = 0;
	_alt = 0;
	_rtime = 0;

	this->increaseCount();
}

Point::Point(float lat, float lon, float alt, float rtime) : Point2D(false) {
	_lat = lat;
	_lon = lon;
	_alt = alt;
	_rtime = rtime;

	this->increaseCount();
}

Point::Point(const Point& pt) : Point2D(false) {
	_lat = pt._lat;
	_lon = pt._lon;
	_alt = pt._alt;
	_rtime = pt._rtime;

	this->increaseCount();
}

Point::~Point() {
	this->decreaseCount();
}

void Point::increaseCount(void) {
	objectCount++;
}

void Point::decreaseCount(void) {
	objectCount--;
}

void Point::increaseCount2D(void) {
}

void Point::decreaseCount2D(void) {
}

Point & Point::operator=(const Point *point) {
	if (point != 0) {
		_lat = point->_lat;
		_lon = point->_lon;
		_alt = point->_alt;
		_rtime = point->_rtime;
		return *this;
	} else {
		return *this;
	}
}

Point & Point::operator=(const Point &point) {
	_lat = point._lat;
	_lon = point._lon;
	_alt = point._alt;
	_rtime = point._rtime;
	return *this;
}

Point& Point::operator =(const Point2D& point) {
	_lat = point._lat;
	_lon = point._lon;
	return *this;
}

Point& Point::operator =(const Point2D* point) {
	_lat = point->_lat;
	_lon = point->_lon;
	return *this;
}

void Point::toString() {
	//printf("Point -> %f %f %f %f\n", _lat, _lon, _alt, _rtime);
}

int Point2D::isValid() {
	if (_lat == 0. || _lon == 0. || fabsf(_lat) > 89. || fabsf(_lon) > 189.) {
		return 0;
	} else {
		return 1;
	}
}
