#include "psx.h"

unsigned long *cd_data;
Sprite sprite;

int main() {
	psSetup();
	cd_open();
	cd_read_file("PLAYER1.TIM", &cd_data);
	cd_close();

	sprite_init(&sprite, 41*2, 46*2, cd_data);
	sprite_set_uv(&sprite, 0, 0, 41, 46);
	sprite.pos.vx = 50;
	sprite.pos.vy = 50;

	camera.pos.vx = 0;
	camera.pos.vz = 2300;
	camera.pos.vy = 900;
	camera.rot.vx = 200;
	camera.rot.vy = 0;
	camera.rot.vz = 0;

	while(1) {
		psClear();

		FntPrint("hello world");

		drawSprite_2d(&sprite);

		psDisplay();
	}

	return 0;
}

