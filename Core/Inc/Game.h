/*
 * Game.h
 *
 *  Created on: Oct 17, 2024
 *      Author: joerg
 */

#ifndef INC_GAME_H_
#define INC_GAME_H_

#include "ili9341.h"
#include "ili9341_gfx.h"

#include "Character.h"

#define SCREEN_X 320
#define SCREEN_Y 240
#define SCREEN_CENTER_X 160
#define SCREEN_CENTER_Y 120
#define MAX_TILE_X SCREEN_X/10
#define MAX_TILE_Y SCREEN_Y/10
#define STEP_SIZE 2

#define NUM_PLAYERS 2
#define LOCAL_PLAYER_ID 0
#define ENEMY_PLAYER_ID 1

typedef enum {
	CHOOSE_PLAYER, INIT_MAZE, WANDER_MAZE, BATTLE
} game_state_t;

typedef struct {
	uint8_t is_x_ok;
	uint8_t is_y_ok;
	uint8_t is_enemy;
} boundary_check_t;

typedef struct {
	 uint16_t width;
	 uint16_t height;
	 int id;
	 uint16_t *data;
} sprite_metadata_t;

sprite_metadata_t *getSprite(uint16_t id);
sprite_metadata_t * getFigureSprite(position_t delta, character_type_t type);
void drawBitmap(ili9341_t *lcd, uint16_t *data, position_t pos, uint16_t width, uint16_t height);
void drawMaze(ili9341_t *lcd, player_t *players);

uint16_t getTileCoord(int16_t x, int16_t y);
void checkBoundary(int16_t x, int16_t y, player_t *players, uint16_t *maze, boundary_check_t *bc);
uint8_t updatePosition(ili9341_t *lcd, position_t new_pos, player_t *players);
void drawRemotePlayer(ili9341_t *lcd, player_t *player);

#endif /* INC_GAME_H_ */
