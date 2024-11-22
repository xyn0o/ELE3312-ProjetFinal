/*
 * Combat.h
 *
 *  Created on: Oct 17, 2024
 *      Author: joerg
 */

#ifndef INC_BATTLE_H_
#define INC_BATTLE_H_

#include "Character.h"
#include "Graphics.h"
#include "Game.h"

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "ili9341.h"
#include "ili9341_gfx.h"

#define INDICATOR_WIDTH 10
#define INDICATOR_HEIGHT 10
#define POWER_BAR_WIDTH 10
#define MAX_POWER 240

#define PLAYER_LEFT_X 30
#define PLAYER_RIGHT_X 160
#define PLAYER_Y 160

void drawBattleScreen(ili9341_t *lcd, player_t *local_player);
void drawRandomTarget(ili9341_t *lcd, ili9341_color_t *target_color,  uint16_t *target_pos_y);

void incrementPowerBar(ili9341_t *lcd, ili9341_color_t *bar_color, uint16_t *power, uint8_t target_matched);
void updateEnemyPowerBar(ili9341_t *lcd, ili9341_color_t *bar_color, uint16_t *enemy_power);

uint8_t checkWinner(ili9341_t *lcd, player_t *local_player, uint16_t *local_power, uint16_t *enemy_power);
void battle(ili9341_t *lcd, player_t *players);

#endif /* INC_BATTLE_H_ */
