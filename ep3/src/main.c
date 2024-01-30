#include "psx.h"

unsigned long *cd_data;
Sprite sprite;
DR_MODE dr_mode;
SPRT sprt;

void sprt_init(DR_MODE *dr_mode, SPRT *sprt){
	SetDrawMode(dr_mode, 0, 0, GetTPage(2, 0, 512, 0), 0);
	SetSprt(sprt);
	sprt->u0 = 0; 
	sprt->v0 = 0;
	sprt->w = 32; 
	sprt->h = 32;
	setRGB0(sprt, 255, 255, 255);
	setXY0(sprt, 50, 50);
}

int main() {
	psSetup();
	cd_open();
	cd_read_file("PLAYER1.TIM", &cd_data);
	cd_close();

	sprt_init(&dr_mode, &sprt);

	sprite_init(&sprite, 41, 46, cd_data);
	sprite_set_uv(&sprite, 41, 0, 41, 46);
	sprite.direction = 1;
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
		drawSprt(&dr_mode, &sprt);

		psDisplay();
	}

	return 0;
}

