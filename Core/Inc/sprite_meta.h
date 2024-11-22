#ifndef SPRITE_META_GUARD
#define SPRITE_META_GUARD

#include "sprite_data.h"

//pacman_sprites/01_wall_vertical.bmp
sprite_metadata_t sprite_01_meta = {
	.width = 10,
	.height = 10,
	.id = 1,
	.data = &sprite_01[0]
};

//pacman_sprites/02_wall_horizontal.bmp
sprite_metadata_t sprite_02_meta = {
	.width = 10,
	.height = 10,
	.id = 2,
	.data = &sprite_02[0]
};

//pacman_sprites/03_wall_right_bend.bmp
sprite_metadata_t sprite_03_meta = {
	.width = 10,
	.height = 10,
	.id = 3,
	.data = &sprite_03[0]
};

//pacman_sprites/04_wall_left_down.bmp
sprite_metadata_t sprite_04_meta = {
	.width = 10,
	.height = 10,
	.id = 4,
	.data = &sprite_04[0]
};

//pacman_sprites/05_wall_left_up.bmp
sprite_metadata_t sprite_05_meta = {
	.width = 10,
	.height = 10,
	.id = 5,
	.data = &sprite_05[0]
};

//pacman_sprites/03_wall_right_down.bmp
sprite_metadata_t sprite_06_meta = {
	.width = 10,
	.height = 10,
	.id = 6,
	.data = &sprite_06[0]
};

//pacman_sprites/06_wall_right_up.bmp
sprite_metadata_t sprite_07_meta = {
	.width = 10,
	.height = 10,
	.id = 7,
	.data = &sprite_07[0]
};

//pacman_sprites/07_wall_right_T.bmp
sprite_metadata_t sprite_08_meta = {
	.width = 10,
	.height = 10,
	.id = 8,
	.data = &sprite_08[0]
};

//pacman_sprites/09_wall_up_T.bmp
sprite_metadata_t sprite_09_meta = {
	.width = 10,
	.height = 10,
	.id = 9,
	.data = &sprite_09[0]
};

//pacman_sprites/10_wall_down_T.bmp
sprite_metadata_t sprite_10_meta = {
	.width = 10,
	.height = 10,
	.id = 10,
	.data = &sprite_10[0]
};

//pacman_sprites/08_wall_rleft_T.bmp
sprite_metadata_t sprite_11_meta = {
	.width = 10,
	.height = 10,
	.id = 11,
	.data = &sprite_11[0]
};

//pacman_sprites/11_packman_left.bmp
sprite_metadata_t sprite_12_meta = {
	.width = 10,
	.height = 10,
	.id = 12,
	.data = &sprite_12[0]
};

//pacman_sprites/12_packman_right.bmp
sprite_metadata_t sprite_13_meta = {
	.width = 10,
	.height = 10,
	.id = 13,
	.data = &sprite_13[0]
};

//pacman_sprites/13_packman_up.bmp
sprite_metadata_t sprite_14_meta = {
	.width = 10,
	.height = 10,
	.id = 14,
	.data = &sprite_14[0]
};

//pacman_sprites/14_packman_down.bmp
sprite_metadata_t sprite_15_meta = {
	.width = 10,
	.height = 10,
	.id = 15,
	.data = &sprite_15[0]
};

//pacman_sprites/15_ghost_blue.bmp
sprite_metadata_t sprite_16_meta = {
	.width = 10,
	.height = 10,
	.id = 16,
	.data = &sprite_16[0]
};
sprite_metadata_t *sprites[] = {
	&sprite_01_meta,
	&sprite_02_meta,
	&sprite_03_meta,
	&sprite_04_meta,
	&sprite_05_meta,
	&sprite_06_meta,
	&sprite_07_meta,
	&sprite_08_meta,
	&sprite_09_meta,
	&sprite_10_meta,
	&sprite_11_meta,
	&sprite_12_meta,
	&sprite_13_meta,
	&sprite_14_meta,
	&sprite_15_meta,
	&sprite_16_meta,
};
#endif
