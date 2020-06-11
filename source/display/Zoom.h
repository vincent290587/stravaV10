/*
 * Zoom.h
 *
 *  Created on: 14 oct. 2017
 *      Author: Vincent
 */


#ifndef ZOOM_H_
#define ZOOM_H_

#include <stdint.h>
#include "Points.h"

#define BASE_ZOOM_LEVEL    10.f
#define BASE_ZOOM_METERS   250.f

class PixelPoint {
public:
	PixelPoint() {
		x = 0; y = 0;
	}

	void shift(int16_t h_pixels, int16_t v_pixels) {
		x += h_pixels;
		y += v_pixels;
	}

	int16_t x, y;
};

class PixelLine {
public:
	PixelLine() {

		region_width  = 0;
		region_length = 0;
		x0 = 0; y0 = 0; x1 = 0; y1 = 0;
	}

	void shift(int16_t h_pixels, int16_t v_pixels) {

		x0 += h_pixels;
		x1 += h_pixels;
		y0 += v_pixels;
		y1 += v_pixels;
	}

	int16_t region_width;
	int16_t region_length;
	int16_t x0, y0, x1, y1;
};

class Zoom {
public:
	Zoom();

	void increaseZoom();
	void decreaseZoom();
	void resetZoom();

	void setSpan(uint16_t h_space, uint16_t v_space);
	void computeZoom(float lat, float distance, float& h_zoom, float& v_zoom);

	uint8_t getZoomLevel() const {
		return m_zoom_level;
	}

	float getLastZoom() const {
		return m_last_zoom;
	}

	void setLastZoom(float zoom_m) {
		m_last_zoom = zoom_m;
	}

	int includes(Location& center, Location& location1, int16_t h_pixels, PixelPoint& point);
	int intersects(Location& center, int16_t h_pixels, int16_t v_pixels, Location& location1, Location& location2, PixelLine& line_rep);

private:
	uint16_t m_h_size;
	uint16_t m_v_size;

	uint8_t m_zoom_level;
protected:
	float m_last_zoom;
};


#endif /* ZOOM_H_ */

