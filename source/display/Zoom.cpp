/*
 * Zoom.cpp
 *
 *  Created on: 14 oct. 2017
 *      Author: Vincent
 */

#include "Zoom.h"
#include "utils.h"
#include "Vecteur.h"
#include "segger_wrapper.h"

Zoom::Zoom() {
	m_h_size = 80;
	m_v_size = 30;
	m_last_zoom = BASE_ZOOM_METERS;
	m_zoom_level = BASE_ZOOM_LEVEL;
}

void Zoom::decreaseZoom() {
	if (m_zoom_level < 100) m_zoom_level += 1;
	LOG_INFO("Zoom--: %u\r\n", m_zoom_level);
}

void Zoom::increaseZoom() {
	if (m_zoom_level > 0) m_zoom_level -= 1;
	else m_zoom_level = BASE_ZOOM_LEVEL * 2.f;
	LOG_INFO("Zoom++: %u\r\n", m_zoom_level);
}

void Zoom::resetZoom() {
	m_zoom_level = BASE_ZOOM_LEVEL;
}

void Zoom::setSpan(uint16_t h_space, uint16_t v_space) {
	m_h_size = h_space;
	m_v_size = v_space;
}

/**
 *  * Zoom horizontal level=5   : m_h_size/2 pixels = 60m
 *                    level=6   : m_h_size/2 pixels = 72m
 *
 *                                |
 *              *------h_zoom-----|
 *                                |
 *
 * @param lat Current latitude
 * @param distance Distance to the closest point
 * @param h_zoom Longitude half-span to be displayed
 * @param v_zoom Latitude half-span to be displayed
 */
void Zoom::computeZoom(float lat, float distance, float& h_zoom, float& v_zoom) {

	// compute convertion between mdeg and meters
	const float deglon_to_m = 1000.f * distance_between(lat, 0.f, lat, 0.001f);
	const float deglat_to_m = 1000.f * distance_between(lat, 0.f, lat + 0.001f, 0.f);

	LOG_INFO("distance PRC: %d m\r\n", (int32_t)distance);

	// worst case of horizontal distance
//	float distance_h = distance + BASE_ZOOM_METERS;

	float h_zoom_m = m_zoom_level * m_zoom_level * BASE_ZOOM_METERS / (BASE_ZOOM_LEVEL * BASE_ZOOM_LEVEL);
	float v_zoom_m = h_zoom_m * m_v_size / m_h_size;

	LOG_INFO("h_zoom_m1: %d m\r\n", (int32_t)h_zoom_m);
	LOG_INFO("v_zoom_m1: %d m\r\n", (int32_t)v_zoom_m);

	// make sure the PRC is in the screen
//	if (v_zoom_m < distance_h) {
//
//		v_zoom_m = 1.3 * distance_h;
//		h_zoom_m = v_zoom_m * m_h_size / m_v_size;
//
//	}

	// convert to degrees
	h_zoom = h_zoom_m / deglon_to_m;

	// rule of 3 for the vertical
//	v_zoom_m = h_zoom_m * m_v_size / m_h_size;
	v_zoom = v_zoom_m / deglat_to_m;

//	LOG_INFO("h_zoom_m2: %d m\r\n", (int32_t)h_zoom_m);
//	LOG_INFO("v_zoom_m2: %d m\r\n", (int32_t)v_zoom_m);

	m_last_zoom = h_zoom_m;
}

enum eCohenSutherlandCode {
	eCohenSutherlandCodeInside = 0,
	eCohenSutherlandCodeLeft   = 1,
	eCohenSutherlandCodeRight  = 2,
	eCohenSutherlandCodeBottom = 4,
	eCohenSutherlandCodeTop    = 8,
};

static inline uint8_t get_code(const float h_zoom_m, const float v_zoom_m, const Vecteur& pt) {

	uint8_t code = eCohenSutherlandCodeInside;

	if (pt._x < -h_zoom_m/2.f)          // to the left of clip window
		code |= eCohenSutherlandCodeLeft;
	else if (pt._x > h_zoom_m/2.f)      // to the right of clip window
		code |= eCohenSutherlandCodeRight;
	if (pt._y < -v_zoom_m/2.f)          // below the clip window
		code |= eCohenSutherlandCodeBottom;
	else if (pt._y > v_zoom_m/2.f)      // above the clip window
		code |= eCohenSutherlandCodeTop;

	return code;
}

int Zoom::includes(Location& center, Location& location1, int16_t h_pixels, int16_t v_pixels, PixelPoint& point) {

	Vecteur pos_to_loc1(center, location1);

	// convert vector from meters to pixels
	pos_to_loc1.operator /(m_last_zoom / h_pixels);

	uint8_t outcode0 = get_code(h_pixels, v_pixels, pos_to_loc1);

	if (outcode0 == eCohenSutherlandCodeInside) {

		point.x =  (int16_t)pos_to_loc1._x;
		point.y = -(int16_t)pos_to_loc1._y;

		return 1;
	}

	return 0;
}

int Zoom::intersects(Location& center, int16_t h_pixels, int16_t v_pixels, Location& location1, Location& location2, PixelLine& line_rep) {

	// we want to know whether the line defined by both locations intersects this zoom rectangle
	Vecteur pos_to_loc1(center, location1);
	Vecteur pos_to_loc2(center, location2);

	const float dx = (float)h_pixels;
	const float dy = (float)v_pixels;

	line_rep.region_width  = h_pixels;
	line_rep.region_length = v_pixels;

	// convert vector from meters to pixels
	pos_to_loc1.operator /(m_last_zoom / dx);
	pos_to_loc2.operator /(m_last_zoom / dx);

	line_rep.x0 = (int16_t)pos_to_loc1._x;
	line_rep.y0 = -(int16_t)pos_to_loc1._y;

	line_rep.x1 = (int16_t)pos_to_loc2._x;
	line_rep.y1 = -(int16_t)pos_to_loc2._y;

	// start of algorithm
	uint8_t outcode0 = get_code(dx, dy, pos_to_loc1);
	uint8_t outcode1 = get_code(dx, dy, pos_to_loc2);

	// trivial cases
	if (!(outcode0 | outcode1)) {
		// bitwise OR is 0: both points inside window; trivially accept and exit loop
		return 2;
	} else if (outcode0 & outcode1) {
		// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
		// or BOTTOM), so both must be outside window; exit loop (accept is false)
		return 0;
	}

	const float xmin = -dx/2.f;
	const float xmax =  dx/2.f;

	const float ymin = -dy/2.f;
	const float ymax =  dy/2.f;

	float &x0 = pos_to_loc1._x;
	float &y0 = pos_to_loc1._y;

	float &x1 = pos_to_loc2._x;
	float &y1 = pos_to_loc2._y;

	// real algorithm, situation is here not trivial and only clipping will help
	// determine if there is an intersection
	while (true) {
		if (!(outcode0 | outcode1)) {
			// bitwise OR is 0: both points inside window; trivially accept and exit loop
			return 1;
		} else if (outcode0 & outcode1) {
			// bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
			// or BOTTOM), so both must be outside window; exit loop (accept is false)
			return 0;
		} else {
			// failed both tests, so calculate the line segment to clip
			// from an outside point to an intersection with clip edge
			float x=x0, y=y0;

			// At least one endpoint is outside the clip rectangle; pick it.
			uint8_t outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

			// Now find the intersection point;
			// use formulas:
			//   slope = (y1 - y0) / (x1 - x0)
			//   x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
			//   y = y0 + slope * (xm - x0), where xm is xmin or xmax
			// No need to worry about divide-by-zero because, in each case, the
			// outcode bit being tested guarantees the denominator is non-zero
			if (outcodeOut & eCohenSutherlandCodeTop) {           // point is above the clip window
				x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
				y = ymax;
			} else if (outcodeOut & eCohenSutherlandCodeBottom) { // point is below the clip window
				x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
				y = ymin;
			} else if (outcodeOut & eCohenSutherlandCodeRight) {  // point is to the right of clip window
				y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
				x = xmax;
			} else if (outcodeOut & eCohenSutherlandCodeLeft) {   // point is to the left of clip window
				y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
				x = xmin;
			}

			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (outcodeOut == outcode0) {
				x0 = x;
				y0 = y;
				outcode0 = get_code(dx, dy, pos_to_loc1);

				line_rep.x0 = (int16_t)x0;
				line_rep.y0 = -(int16_t)y0;
			} else {
				x1 = x;
				y1 = y;
				outcode1 = get_code(dx, dy, pos_to_loc2);

				line_rep.x1 = (int16_t)x1;
				line_rep.y1 = -(int16_t)y1;
			}
		}
	}

}
