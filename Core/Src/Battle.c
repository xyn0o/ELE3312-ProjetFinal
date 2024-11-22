/**
 * @file Battle.c
 * @brief Implements the battle between two players (Pac-Man and Ghost).  
 */

#include "Battle.h"

/**
* @brief This function draws the two enemies (Ghost and Pac-Man) on the screen.
* @param lcd [in] a pointer to the ili9341 configuration structure
* @param local_player [in] a pointer to the player structure that encapsulates information about the local player (mainly its type)
*/
void drawBattleScreen(ili9341_t *lcd, player_t *local_player) {
	char buffer[40] = {0};
	ili9341_fill_screen(lcd, ILI9341_BLACK);
	ili9341_text_attr_t text_attr = {&ili9341_font_11x18,  ILI9341_WHITE, ILI9341_BLACK, 120, 0};
	sprintf(buffer,"Fight !!!!");
	ili9341_draw_string(lcd, text_attr, buffer);
	
	if (local_player->character.type == PACMAN) {
		drawGhost(lcd, PLAYER_LEFT_X, PLAYER_Y, GHOST_COLOR);
		drawPacMan(lcd, PLAYER_RIGHT_X, PLAYER_Y, PACMAN_COLOR);
	} else {
		drawPacMan(lcd, PLAYER_LEFT_X, PLAYER_Y, PACMAN_COLOR);
		drawGhost(lcd, PLAYER_RIGHT_X, PLAYER_Y, GHOST_COLOR);
	}
}

/**
*	@brief This function generates new target note after a certain time and draws the target
*			   on the screen.
* @param lcd [in] pointer to the ili9341 handle structure used to access the tft screen
* @param target_color [in] pointer to an ili9341_color_t color that specifies the color of the target token. The color 
*				 is supposed to change when the player token is in contact with the target
* @param target_pos_y [out] unsigned integer pointer that indicates the height of the target. This value can be used to 
*        choose the music note and to verify that the player token coincides with the target. 
*/
void drawRandomTarget(ili9341_t *lcd, ili9341_color_t *target_color, uint16_t *target_pos_y) {
	const uint16_t target_pos_x = 290;
	static uint16_t previous_pos_y = 0;
	
	// Mise à jour de la position après counter * délai principal ms (délai d'appel à drawRandomTarget)
	static uint16_t counter = 0;
	const uint8_t update_threshold = 25;
	if(counter > update_threshold) {
		*target_pos_y = rand() % (SCREEN_Y + 1);
		counter = 0;
	}
	counter += 1;
	ili9341_fill_rect(lcd, ILI9341_BLACK, target_pos_x, previous_pos_y, INDICATOR_WIDTH, INDICATOR_HEIGHT);
	ili9341_fill_rect(lcd, *target_color, target_pos_x, *target_pos_y, INDICATOR_WIDTH, INDICATOR_HEIGHT);
	previous_pos_y = *target_pos_y;
}

/**
* @brief This function increments the value of the power-bar a bar-graph-like indicator. The first player capable 
*        of filling up its power-bar serves a fatal attack to the opponent and wins the battle.
* @param lcd [in] pointer to the ili9341 handle structure used to access the tft screen
* @param bar_color [in] pointer to an ili9341_color_t color of the bar. The bar should have a color that is in 
*        accordance with the players' game character.
* @param power [out] pointer to an unsigned integer that holds the power value of the player. Whenever the
*        target token and the players' token coincide, the power-value is increased.
* @param target_matched [in] pointer to an unsigned integer that indicates that both the players' and the
*        target token are in contact. 
*/
void incrementPowerBar(ili9341_t *lcd, ili9341_color_t *bar_color, uint16_t *power, uint8_t target_matched) {
	const uint16_t x_pos = 310;
	if (target_matched) {
		(*power) += 1;
		ili9341_fill_rect(lcd, *bar_color, x_pos, 240 - *power, INDICATOR_WIDTH, 5);
	}
}

/**
* @brief This function draws the power-bar of the opponent.
* @param lcd [in] pointer to the ili9341 handle structure used to access the tft screen
* @param bar_color [in] pointer to the ili9341_color_t color of the opponents' power-bar
* @param enemy_power [in] pointer to an unsigned integer that represents the current enemy power.
*/
void updateEnemyPowerBar(ili9341_t *lcd, ili9341_color_t *bar_color, uint16_t *enemy_power) {
		// Donnée à remplacer avec celle reçue par la carte du joueur adverse
		int random_val = rand() % 10;
		if(random_val <= 5) {
			(*enemy_power) += 1;
		}
		ili9341_fill_rect(lcd, *bar_color, 0, 240 - *enemy_power, INDICATOR_WIDTH, 5);
}

/**
* @brief Function that checks whether one of the two players have reached the maximum power and hence won 
*        the battle. In case of a victory, the function visually indicates the losing player and announces
*        which player won. 
* @param lcd [in] pointer to the ili9341 handle structure used to access the tft screen
* @param local_player [in] pointer to the player_t structure that holds the local players information
* @param local_power [in] pointer to an unsigned integer containing the local players' current power level
* @param enemy_power [in] pointer to an unsigned integer holding the opponents power
* @return unsigned integer value that indicates 0 - no winner, 1 - local player wins, 2 - opponent wins
*/
uint8_t checkWinner(ili9341_t *lcd,  player_t *local_player, uint16_t *local_power, uint16_t *enemy_power) {
	uint8_t winner = 0;

	// Player right
	if(*local_power == MAX_POWER) {
		drawGrave(lcd, PLAYER_LEFT_X, PLAYER_Y);
		winner = 1;
	}
	// Player left
	else if(*enemy_power == MAX_POWER){
		drawGrave(lcd, PLAYER_RIGHT_X, PLAYER_Y);
		winner = 2;
	}
	
	if(winner){
		*local_power = 0;
		*enemy_power = 0;
		ili9341_fill_rect(lcd, ILI9341_BLACK, 0, 0, INDICATOR_WIDTH, 240);
		ili9341_fill_rect(lcd, ILI9341_BLACK, 310, 0, INDICATOR_WIDTH, 240);
		const int x_pos = 290;
		ili9341_fill_rect(lcd, ILI9341_BLACK, x_pos, 0, INDICATOR_WIDTH, 240);
		char buffer[40] = {0};
		ili9341_text_attr_t text_attr = {&ili9341_font_11x18,  ILI9341_WHITE, ILI9341_BLACK, 80, 0};
		sprintf(buffer,"%s Player Wins!", winner == 1 ? "Right" : "Left");
		ili9341_draw_string(lcd, text_attr, buffer);
		HAL_Delay(1000);
	}
	return winner;
}

/**
* @brief This function implements the game logic of the battle stage of the game.
* @param lcd [in] pointer to the ili9341 handle structure used to access the tft screen
* @param players [in] pointer to an array of player_t structs containing information about the players
*/ 
void battle(ili9341_t *lcd, player_t *players) {
	player_t* local_player = &players[LOCAL_PLAYER_ID];
	player_t* enemy_player = &players[ENEMY_PLAYER_ID];
	uint16_t local_power = 0;
	uint16_t enemy_power = 0;
	
	uint16_t target_pos = 0;		// À comparer avec la distance mesurée par la capteur ultrason
	uint8_t target_matched = 0;	// VRAI si la target_pos est aligné avec la distance de l'objet au capteur
	
	drawBattleScreen(lcd, local_player);
	
	uint8_t winner = 0;
	while(winner == 0) {
		HAL_Delay(20); // À remplacer avec un timer
		
		drawRandomTarget(lcd, &local_player->character.color, &target_pos);
		incrementPowerBar(lcd, &local_player->character.color, &local_power, target_matched);
		updateEnemyPowerBar(lcd, &enemy_player->character.color, &enemy_power);

		winner = checkWinner(lcd, local_player, &local_power, &enemy_power);
	}
}
