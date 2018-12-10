/*
 * Points.h
 *
 *  Created on: 15 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_ROUTES_POINTS_H_
#define SOURCE_ROUTES_POINTS_H_

#include "utils.h"


class Location {
public:
	Location();
	Location(const Location&);
    Location(const Location*);
    Location(float lat, float lon);
    ~Location();

    int isValid();

    float dist(Location *autre_p)  {
        return distance_between(_lat, _lon, autre_p->_lat, autre_p->_lon);
    }
    float dist(Location &autre_p)  {
        return distance_between(_lat, _lon, autre_p._lat, autre_p._lon);
    }
    float dist(float lat_, float lon_)  {
    	return distance_between(_lat, _lon, lat_, lon_);
    }

    float _lat;
    float _lon;
};


class Point2D : public Location {
public:
	Point2D();
	Point2D(const Location&);
	Point2D(const Location*);
	Point2D(const Point2D&);
	Point2D(const Point2D*);
	Point2D(float lat, float lon);
    ~Point2D();

    Point2D & operator=(const Location &point);
    Point2D & operator=(const Location *point);

    virtual void increaseCount2D(void);
    virtual void decreaseCount2D(void);

	static int getObjectCount() {
		return objectCount2D;
	}

private:
    static int objectCount2D;
};

class Point : public Location {
public:
    Point();
    Point(const Location&);
    Point(const Location*);
	Point(const Point&);
	Point(const Point*);
    Point(float lat, float lon, float alt, float rtime);
    ~Point();

    Point & operator=(const Point &point);
    Point & operator=(const Point *point);
    Point & operator=(const Location &point);
    Point & operator=(const Location *point);

    virtual void increaseCount(void);
    virtual void decreaseCount(void);

    void toString();

	static int getObjectCount() {
		return objectCount;
	}

    float _alt;
    float _rtime;

private:
    static int objectCount;
};


#endif /* SOURCE_ROUTES_POINTS_H_ */
