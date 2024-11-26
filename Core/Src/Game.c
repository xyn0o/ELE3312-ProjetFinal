/**
* file: Game.c
* @brief This file implements the maze portion of the game. 
*/

#include "Game.h"
#include "sprite_meta.h"
#include "maze.h"
#include "usart.h"   // Make sure this includes the declaration of huart2
#include <stdio.h>   // For FILE definition

extern UART_HandleTypeDef huart2; // Declare the UART handle

int fputc(int ch, FILE *f);


/**
* @brief This function searches in an array of sprite metadata objects for the sprite with the specified ID.
* @param id [in] unsigend integer id value of the sprite
* @return pointer to a sprite_metadata_t opject that can be used to display bmp sprites on the screen
*/
sprite_metadata_t *getSprite(uint16_t id) {
	for(int i=0; i<16; ++i){
		if(sprites[i]->id == id){
			return sprites[i];
		}
	}
	return NULL;
}

/**
* @brief This function returns the sprite metadata for the specified character.
*        The returned sprite depends on the direction of movement of the character. 
*				 Currently, only the Pac-Man character has different sprites that depend on 
*				 its direction of movement.
* @param delta [in] a position_t object containing the spacial differences
* @param type [in] a character_type_t structure containing character information (like its' type)
* @return a sprite_metadata_t object containing sprite metadat information 
*/
sprite_metadata_t * getFigureSprite(position_t delta, character_type_t type) {
	if(type == PACMAN) {
		int16_t delta_x_abs = delta.x > 0 ? delta.x : -1*delta.x;
		int16_t delta_y_abs = delta.y > 0 ? delta.y : -1*delta.y;
		if (delta_x_abs > delta_y_abs) {
			// Figure faces in x direction
			if(delta.x < 0){
				return getSprite(12);
			}else {
				return getSprite(13);
			}
		} else {
			if(delta.y < 0){
				return getSprite(14);
			} else {
				return getSprite(15);
			}
		}
	} else {
		return getSprite(16);
	}
	return NULL;
}

/**
* @brief Function that draws a bitmap to the screen. 
* @param lcd [in]  pointer to the ili9341 handle structure used to access the tft screen
* @param data [in] unsigned integer pointer to the memory location of the bitmap data
* @param pos [in] position_t structure indicating the lower left position of the rectangular image
* @param width [in] unsigned integer value indicating the width in pixels
* @param height [in] unsigned integer value indicating the height in pixels
*/
void drawBitmap(ili9341_t *lcd, uint16_t *data, position_t pos, uint16_t width, uint16_t height) {
	  uint16_t size = width*height;
	  // Set address region
	  ili9341_spi_tft_set_address_rect(lcd, pos.x, pos.y, pos.x + width-1, pos.y + height);
	  ili9341_spi_tft_select(lcd);
	  HAL_GPIO_WritePin(lcd->data_command_port, lcd->data_command_pin, __GPIO_PIN_SET__);
	  ili9341_transmit_color(lcd, size*2, data, ibTrue);
	  ili9341_spi_tft_release(lcd);
}

/**
* @brief Draws the maze on the TFT screen. The function also extracts the initial player positions
*        from the maze map that is accessed via the global pointer variable maze.
* @param lcd [in]  pointer to the ili9341 handle structure used to access the tft screen
* @param players [in/out] a player_t array of player_t structures containing player information
*/
void drawMaze(ili9341_t *lcd, player_t *players) {
	ili9341_fill_screen(lcd, ILI9341_BLACK);
	sprite_metadata_t *sprite = NULL;
	
	player_t* pacman = players[0].character.type == PACMAN ? &players[0] : &players[1];
	player_t* ghost = players[0].character.type == GHOST ? &players[0] : &players[1];
	
	for(int y=0; y<SCREEN_Y/10; ++y) {
		for(int x=0; x<SCREEN_X/10; ++x) {
			uint16_t *maze_p = maze + y*32 + x;
			uint16_t tile = *maze_p;
			if((sprite = getSprite(tile)) != NULL) {
				switch(tile){
				case 12: // Pacman sprite that looks to the right
				case 13: // Pacman
				case 14: // Pacman
				case 15: // Pacman
					pacman->current_pos = (position_t){ .x=x*10, .y=y*10 };
					pacman->start_pos = pacman->current_pos;
					*maze_p = 0;
					break;
				case 16: // Ghost
					ghost->current_pos = (position_t){ .x=x*10, .y=y*10 };
					ghost->start_pos = ghost->current_pos;
					*maze_p = 0;
					break;
				}
				drawBitmap(lcd, sprite->data, (position_t){x*10, y*10}, sprite->width, sprite->height);
			} else {
				// Draw Coin
				ili9341_fill_circle(lcd,ILI9341_WHITE,x*10 + 5, y*10 + 5,1);
			}
		}
	}
	
	// Draw mobile characters that might not be on the map anymore
	for(uint16_t i=0; i<NUM_PLAYERS; ++i) {
		player_t *c = &players[i];
		uint16_t tile = c->character.type == PACMAN ? 13 : 16;
		sprite = getSprite(tile);
		drawBitmap(lcd, sprite->data, c->current_pos, sprite->width, sprite->height);
	}
}

/** 
* @brief Helper function that returns the tile value based on the specified 
*        screen x and y coordinates.
* @param x [in] unsigned integer x value (in pixels)
* @param y [in] unsigned integer y value (in pixels)
* @return the value of the tile (possible values are ids of walls or special objects)
*/
uint16_t getTileCoord(int16_t x, int16_t y) {
	return *(maze + y*MAX_TILE_X + x);
}

/**
* @brief This function checks whether an intended move is valid or invalid becaus of a collision with 
*        another object (like the wall).
* @param x [in] unsigned integer of the intended x position
* @param y [in] unsigned integer of the intended y position
* @param players [in] array of player_t structures
* @param maze [in] pointer to the unsigned integer array containing the maze information
* @param  bc [out] boundary_check_t structure that encapsulates the different boundary check results
*/
void checkBoundary(int16_t x, int16_t y, player_t *players, uint16_t *maze, boundary_check_t *bc) {
	player_t * pos = &players[LOCAL_PLAYER_ID];
	player_t * enemy = &players[ENEMY_PLAYER_ID];
	
	
	uint16_t enemy_tile_id = enemy->character.type == GHOST ? 16 : 13;
	position_t enemy_pos = {0};

	bc->is_x_ok = 0;
	bc->is_y_ok = 0;
	bc->is_enemy = 0;

	uint16_t x_col_1, x_col_2, y_col_1, y_col_2;
	// Put enemy temporarily on the maze
	if(enemy != NULL){
		enemy_pos.x = enemy->current_pos.x/10;
		enemy_pos.y = enemy->current_pos.y/10;
		*(maze+enemy_pos.y*MAX_TILE_X+enemy_pos.x) = enemy_tile_id;
	}

	if(x > pos->current_pos.x){ // We are going in positive x direction
		uint16_t x_pos = (x+10)/10;
		x_col_1 = getTileCoord(x_pos, (y+2)/10);
		x_col_2 = getTileCoord(x_pos,(y+10-2)/10);
		bc->is_x_ok = x_col_1 == 0 && x_col_2 == 0;
	} else if (x < pos->current_pos.x) { // We are going in negative x direction
		x_col_1 = getTileCoord((x)/10, (y+1)/10);
		x_col_2 = getTileCoord((x)/10,(y+10-1)/10);
		bc->is_x_ok = x_col_1 == 0 && x_col_2 == 0;
	}
	if(y > pos->current_pos.y){ // We are going in positive y direction
		y_col_1 = getTileCoord((x+2)/10, (y+10)/10);
		y_col_2 = getTileCoord((x+10-2)/10,(y+10)/10);
		bc->is_y_ok = y_col_1 == 0 && y_col_2 == 0;
	} else if (y < pos->current_pos.y) { // We are going in negative y direction
		y_col_1 = getTileCoord((x+2)/10, (y)/10);
		y_col_2 = getTileCoord((x+10-2)/10,(y)/10);
		bc->is_y_ok = y_col_1 == 0 && y_col_2 == 0;
	}
	// Remove enemy on the maze
	if(enemy != NULL){
		enemy_pos.x = enemy->current_pos.x/10;
		enemy_pos.y = enemy->current_pos.y/10;
		*(maze+enemy_pos.y*MAX_TILE_X+enemy_pos.x) = 0;
	}

	if(bc->is_x_ok == 0) {
		bc->is_enemy |= x_col_1 == enemy_tile_id || x_col_2 == enemy_tile_id;
	}
	if(bc->is_y_ok == 0) {
		bc->is_enemy |= y_col_1 == enemy_tile_id || y_col_2 == enemy_tile_id;
	}

}

/**
* @brief This function takes a set of new postions on the screen, validates them
*        and draws the player on the new position.
* @param lcd [in]  pointer to the ili9341 handle structure used to access the tft screen
* @param new_pos [in] position_t structure that encapsulates the new position data (mainly x and y position)
* @param players [in] array of player_t structures
* @return unsigned integer that can be interpreted as an boolean value indicating whether the player collided with an opponent.
*/
uint8_t updatePosition(ili9341_t *lcd, position_t new_pos, player_t* players) {
	player_t *pos = &players[LOCAL_PLAYER_ID];
	// Calculate new position
	uint8_t pos_changed = 0;
	boundary_check_t bc = {0};
	// Save old position
	int16_t old_x = pos->current_pos.x;
	int16_t old_y = pos->current_pos.y;

	// Check intended position and update position values
	checkBoundary(new_pos.x, new_pos.y, players, maze, &bc);

	if (bc.is_x_ok){
		pos->current_pos.x = new_pos.x;
		pos_changed = 1;
	}
	if(bc.is_y_ok){
		pos->current_pos.y = new_pos.y;
		pos_changed = 1;
	}
	// Update Screen position of moving entity
	if(pos_changed){ // Only update screen if position has changed
		sprite_metadata_t *sprite = getFigureSprite((position_t){new_pos.x-old_x, new_pos.y-old_y}, pos->character.type);
		if(sprite != NULL){
			ili9341_fill_rect(lcd, ILI9341_BLACK, old_x, old_y, 10, 10); // Delete old sprite
			// Check for screen border (tunnels)
			if(pos->current_pos.x > SCREEN_X - 15) { // Does not work with 10
				pos->current_pos.x = 10;
			}
			if(pos->current_pos.x < 10) {
				pos->current_pos.x = SCREEN_X - 16;
			}
			if(pos->current_pos.y > SCREEN_Y - 14) {
				pos->current_pos.y = 14;
			}
			if(pos->current_pos.y < 10) {
				pos->current_pos.y = SCREEN_Y - 14;
			}

			drawBitmap(lcd, sprite->data, pos->current_pos, sprite->width, sprite->height);
		}
	}
	return bc.is_enemy;
}

/**
* @brief This function is used to draw the remote player on the screen. We assume
*        that the remote player has already checked its new position for collisions
*        and thus don't do the collision check again.
* @param lcd [in] pointer to the ili9341 handle structure used to access the tft screen
* @param player [in] pointer to a player_t structure containing the relevant player information
*/
void drawRemotePlayer(ili9341_t *lcd, player_t* player) {
	position_t delta = {.x = player->current_pos.x-player->previous_pos.x,
											.y = player->current_pos.y-player->previous_pos.y };
	
	sprite_metadata_t *sprite = getFigureSprite(delta, player->character.type);
	ili9341_fill_rect(lcd, ILI9341_BLACK, player->previous_pos.x, player->previous_pos.y, 10, 10);
	
	drawBitmap(lcd, sprite->data, player->current_pos, sprite->width, sprite->height );
}


