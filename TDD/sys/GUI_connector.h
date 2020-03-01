/*
 * GUI_connector.H
 *
 *  Created on: 6 déc. 2018
 *      Author: Vincent
 */

#ifndef TDD_GUI_CONNECTOR_H_
#define TDD_GUI_CONNECTOR_H_

#include <stdint.h>


void GUI_connector_init(void);

void GUI_UpdateLS027(void);

#ifdef __cplusplus
extern "C" {
#endif

void neopixel_update(uint8_t red, uint8_t green, uint8_t blue);

#ifdef __cplusplus
}
#endif

#endif /* TDD_GUI_CONNECTOR_H_ */
