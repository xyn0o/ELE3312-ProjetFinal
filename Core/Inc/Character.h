/*
 * character.h
 *
 *  Created on: Oct 21, 2024
 *      Author: joerg
 */

#ifndef INC_CHARACTER_H_
#define INC_CHARACTER_H_

#include <stdint.h>
#include "ili9341_gfx.h"

#define GHOST_COLOR ILI9341_RED
#define PACMAN_COLOR ILI9341_YELLOW

typedef enum {
	NONE, PACMAN, GHOST
}character_type_t;

typedef struct {
	character_type_t type;
	ili9341_color_t color;
}character_t;

typedef struct {
	int16_t x;
	int16_t y;
}position_t;

typedef struct {
	character_t character;

	position_t start_pos;
	position_t current_pos;
	position_t previous_pos;
}player_t;

#endif /* INC_CHARACTER_H_ */
