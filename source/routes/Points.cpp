/*
 * Points.cpp
 *
 *  Created on: 15 oct. 2017
 *      Author: Vincent
 */

#include <math.h>
#include "Points.h"


Point2D::Point2D() {
	_lat = 0;
	_lon = 0;

	objectCount2D++;
}

Point2D::Point2D(float lat, float lon) {
	_lat = lat;
	_lon = lon;

	objectCount2D++;
}

Point2D::Point2D(const Point2D& pt) {
	_lat = pt._lat;
	_lon = pt._lon;

	objectCount2D++;
}

Point2D::~Point2D() {
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

Point::Point():Point2D() {
	_lat = 0;
	_lon = 0;
	_alt = 0;
	_rtime = 0;

	objectCount++;
}

Point::Point(float lat, float lon, float alt, float rtime) {
	_lat = lat;
	_lon = lon;
	_alt = alt;
	_rtime = rtime;

	objectCount++;
}

Point::Point(const Point& pt) {
	_lat = pt._lat;
	_lon = pt._lon;
	_alt = pt._alt;
	_rtime = pt._rtime;

	objectCount++;
}

Point::~Point() {
	objectCount--;
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
	if (_lat == 0. || _lon == 0. || fabs(_lat) > 89. || fabs(_lon) > 189.) {
		return 0;
	} else {
		return 1;
	}
}
