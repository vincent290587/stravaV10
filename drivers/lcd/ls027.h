/*
 * ls027.h
 *
 *  Created on: 29 sept. 2017
 *      Author: Vincent
 */

#ifndef SRC_LCD_LS027_H_
#define SRC_LCD_LS027_H_

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#if !defined(WIN32)
#include <stdbool.h>
#endif


typedef bool LS027_PixelColor; /* one bit is enough to describe the color as we have a monochrome display */

#define LS027_PIXEL_BLACK          1 /* 0 is a black pixel */
#define LS027_PIXEL_WHITE          0 /* 1 is a white pixel */

#define LS027_PIXEL_GROUP_BLACK          0xFF /* 0 is a black pixel */
#define LS027_PIXEL_GROUP_WHITE          0x00 /* 1 is a white pixel */

#define LS027_COLOR_PIXEL_SET      LS027_PIXEL_WHITE /* color for a pixel set */
#define LS027_COLOR_PIXEL_CLR      LS027_PIXEL_BLACK /* color for a pixel cleared */

#define LS027_COLOR_BLACK          LS027_PIXEL_BLACK
#define LS027_COLOR_WHITE          LS027_PIXEL_WHITE

typedef enum {
  LS027_ORIENTATION_PORTRAIT    = 0,
  LS027_ORIENTATION_PORTRAIT180 = 1,
  LS027_ORIENTATION_LANDSCAPE   = 2,
  LS027_ORIENTATION_LANDSCAPE180= 3
} LS027_DisplayOrientation;

#define LS027_HW_WIDTH         400u /* width of display in pixels */
#define LS027_HW_HEIGHT        240u /* height of display in pixels */

#define LS027_HW_SHORTER_SIDE  240u /* size of shorter display side in pixels */
#define LS027_HW_LONGER_SIDE   400u /* size of longer display side in pixels */

#define LS027_DISPLAY_HW_NOF_COLUMNS  LS027_HW_WIDTH
#define LS027_DISPLAY_HW_NOF_ROWS     LS027_HW_HEIGHT

#define LS027_BUFFER_SIZE          (LS027_DISPLAY_HW_NOF_ROWS*(((LS027_DISPLAY_HW_NOF_COLUMNS-1)/8)+1))

/////////    FUNCTIONS

#if defined(__cplusplus)
extern "C" {
#endif /* _cplusplus */

void LS027_Clear(void);

void LS027_Init(void);

void LS027_InvertColors(void);

void LS027_UpdateFull(void);

void LS027_ToggleVCOM(void);

void LS027_drawPixel(uint16_t x, uint16_t y, uint16_t color);

#if defined(__cplusplus)
}
#endif /* _cplusplus */


#endif /* SRC_LCD_LS027_H_ */
