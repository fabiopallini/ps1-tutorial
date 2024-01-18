#include "psx.h"

unsigned long *cd_data;
Sprite sprite;

int main() {
	psSetup();
	cd_open();
	cd_read_file("PLAYER1.TIM", &cd_data);
	cd_close();

	sprite_init(&sprite, 41, 46, cd_data);
	sprite_set_uv(&sprite, 41, 0, 41, 46);
	sprite.pos.vx = 50;
	sprite.pos.vy = 50; 

	while(1) {
		psClear();

		sprite_anim(&sprite, 41, 46, 0, 0, 6);
		sprite.pos.vx += 2;
		if(sprite.pos.vx >= 320)	
			sprite.pos.vx = 0;

		FntPrint("hello world");
		drawSprite_2d(&sprite);

		psDisplay();
	}

	return 0;
}

