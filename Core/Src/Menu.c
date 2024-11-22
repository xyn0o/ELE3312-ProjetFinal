/**
 * @file Menu.c
 * @brief This file implements the game manu. The menu is used to choose a character (Pac-Man or Ghost)
 */

#include "Menu.h"

/**
* @brief The function draws the two possible players (Pac-Man or Ghost) on the screen. 
*        Additionally, it renders two hollow squares under each character and a filled 
*        square in the middle that serves as a cursor. The function is supposed to 
*        read the accelerometer value and use its values to move the cursor block
*        until it coincides with one of the hollow squares. At this point, the player 
*        is chosen and the function returns. 
* @param lcd  [in] pointer to the ili9341 handle structure used to access the tft screen
* @param players [out] array of player_t structures which contains the identities of the players when the
*        function returns
*/
void choosePlayer(ili9341_t *lcd, player_t* players){
	const uint16_t checkbox_size = 30;
	const uint16_t checkfill_size = 20;
	
	const uint8_t char_y_pos = 140;
	const character_t pacman 					= { .type = PACMAN, .color = PACMAN_COLOR };
	const position_t pacman_icon_pos 	= { .x = 180, .y = char_y_pos };
	const character_t ghost 					= { .type = GHOST,	.color = GHOST_COLOR };
	const position_t ghost_icon_pos 	= { .x = 30, .y = char_y_pos };
	
	position_t cursor_pos = { .x=(((pacman_icon_pos.x+35)-(ghost_icon_pos.x+55))/2)+15+(ghost_icon_pos.x+35), .y=185 };
	position_t old_cursor_pos = cursor_pos;
	
	character_t chosen_character = {0};
	
	ili9341_fill_screen(lcd, ILI9341_BLACK);
	
	drawGhost(lcd, ghost_icon_pos.x, ghost_icon_pos.y, ghost.color);
	drawPacMan(lcd, pacman_icon_pos.x, pacman_icon_pos.y, pacman.color);
	
	ili9341_draw_rect(lcd, ILI9341_WHITE, ghost_icon_pos.x + 35, 180, checkbox_size, checkbox_size);
	ili9341_draw_rect(lcd, ILI9341_WHITE, pacman_icon_pos.x + 35, 180, checkbox_size, checkbox_size);
	
	while(chosen_character.type == NONE) {
		HAL_Delay(20); // � remplacer avec un timer
		
		cursor_pos.x += 0.5 * STEP_SIZE; // Mise � jour de la position du curseur
		
		ili9341_fill_rect(lcd, ILI9341_BLACK, old_cursor_pos.x, old_cursor_pos.y, checkfill_size, checkfill_size);
		ili9341_fill_rect(lcd, ILI9341_WHITE, cursor_pos.x, cursor_pos.y, checkfill_size, checkfill_size);
		old_cursor_pos = cursor_pos;
		
		uint8_t pacman_chosen = cursor_pos.x >= pacman_icon_pos.x + checkbox_size + 10;
		if (pacman_chosen) {
			chosen_character = pacman;
			ili9341_fill_rect(lcd, ILI9341_BLACK, pacman_icon_pos.x + 35, 180, checkbox_size, checkbox_size);
			ili9341_draw_rect(lcd, pacman.color, pacman_icon_pos.x + 35, 180, checkbox_size, checkbox_size);
			ili9341_fill_rect(lcd, pacman.color, pacman_icon_pos.x + 40, cursor_pos.y, checkfill_size, checkfill_size);
		}
		uint8_t ghost_chosen = cursor_pos.x <= ghost_icon_pos.x + checkbox_size + 10;
		if (ghost_chosen) {
			chosen_character = ghost;
			ili9341_fill_rect(lcd, ILI9341_BLACK, ghost_icon_pos.x + 35, 180, checkbox_size, checkbox_size);
			ili9341_draw_rect(lcd, ghost.color, ghost_icon_pos.x + 35, 180, checkbox_size, checkbox_size);
			ili9341_fill_rect(lcd, ghost.color, ghost_icon_pos.x + 40, cursor_pos.y, checkfill_size, checkfill_size);
		}
	}
	
	players[LOCAL_PLAYER_ID].character = chosen_character;
	players[ENEMY_PLAYER_ID].character = chosen_character.type == PACMAN ? ghost : pacman;
	
	HAL_Delay(1000);
	ili9341_fill_screen(lcd, ILI9341_BLACK);
}
