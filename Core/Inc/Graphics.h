/*
 * Graphics.h
 *
 *  Created on: Oct 22, 2024
 *      Author: joerg
 */

#ifndef INC_GRAPHICS_H_
#define INC_GRAPHICS_H_

#include "ili9341.h"
#include "ili9341_gfx.h"

#include "math.h"

void drawGrave(ili9341_t *lcd, int16_t x, int16_t y);

void writeFillArcHelper(ili9341_t *lcd, int16_t cx, int16_t cy, int16_t oradius, int16_t iradius, float start, float end, ili9341_color_t color);
void fillArc(ili9341_t *lcd, int16_t x, int16_t y, int16_t r1, int16_t r2, float start, float end, ili9341_color_t color);
void drawPacMan(ili9341_t *lcd, uint16_t x, uint16_t y, ili9341_color_t color);

void drawGhost(ili9341_t *lcd, uint16_t x, uint16_t y, ili9341_color_t color);

#endif /* INC_GRAPHICS_H_ */
