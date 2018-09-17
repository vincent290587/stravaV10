/*
 * s_assert.h
 *
 *  Created on: 17 sept. 2018
 *      Author: Vincent
 */

#ifndef ASSERT_WRAPPER_H_
#define ASSERT_WRAPPER_H_

#ifdef TDD
#include "assert.h"
#define ASSERT(X)     assert(X)
#else
#include "nrf_assert.h"
#endif

#endif /* ASSERT_WRAPPER_H_ */
