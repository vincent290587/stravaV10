/*
 * UDMatrix.h
 *
 *  Created on: 28 mrt. 2019
 *      Author: v.golle
 */

#ifndef LIBRARIES_KALMAN_UDMATRIX_H_
#define LIBRARIES_KALMAN_UDMATRIX_H_

#include <stdint.h>
#include <vector>

using std::vector;


typedef float udm_type_t;

class UDMatrix {
public:
	UDMatrix(void);
	UDMatrix(UDMatrix &mat);
	UDMatrix(unsigned _rowSize, unsigned _colSize);

	udm_type_t get(unsigned x, unsigned y);
	void set(unsigned x, unsigned y, udm_type_t val);
	void unity(udm_type_t res = (udm_type_t)1);
	void ones(udm_type_t res = (udm_type_t)1);
	void zeros(void);
	void print(const char *str = nullptr);
	void div(udm_type_t val);
	void mul(udm_type_t val);
	void resize(unsigned _rowSize, unsigned _colSize);

	bool isEmpty(void) {if (!m_rowSize || !m_colSize) return true; return false;}

	void normalize(void);
	void bound(udm_type_t min_val, udm_type_t max_val);

	UDMatrix operator+(UDMatrix &s_mat);
	UDMatrix operator-(UDMatrix &s_mat);
	UDMatrix operator*(UDMatrix &s_mat);

	UDMatrix transpose();
	UDMatrix invert();

protected:
	unsigned m_rowSize;
	unsigned m_colSize;

	vector< vector<udm_type_t> > m_data;

};



#endif /* LIBRARIES_KALMAN_UDMATRIX_H_ */
