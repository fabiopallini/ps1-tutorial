#include "psx.h"

unsigned long *cd_data;
Sprite player1, player2;
DR_MODE dr_mode;
SPRT sprt;

void sprt_init(DR_MODE *dr_mode, SPRT *sprt){
	SetDrawMode(dr_mode, 0, 0, GetTPage(2, 0, 512, 0), 0);
	SetSprt(sprt);
	sprt->u0 = 0; 
	sprt->v0 = 0;
	sprt->w = 41; 
	sprt->h = 46;
	setRGB0(sprt, 255, 255, 255);
	setXY0(sprt, 10, 20);
}

int main() {
	psSetup();
	cd_open();
	cd_read_file("PLAYER1.TIM", &cd_data);
	cd_close();

	sprt_init(&dr_mode, &sprt);

	sprite_init(&player1, 41, 46, cd_data);
	player1.direction = 1;
	sprite_set_uv(&player1, 41, 0, 41, 46);
	player1.pos.vx = 180;
	player1.pos.vy = 180; 

	sprite_init(&player2, 41, 46, cd_data);
	player2.direction = 1;
	sprite_set_uv(&player2, 41, 0, 41, 46);
	player2.pos.vx = 180;
	player2.pos.vy = 180; 

	while(1) {
		psClear();

		sprite_anim(&player1, 41, 46, 0, 0, 6);
		player1.pos.vx += 2;
		if(player1.pos.vx >= 320)	
			player1.pos.vx = 0;

		if((pad >> 16 & PADLleft) == 0 && (pad >> 16 & PADLright) == 0)
			sprite_set_uv(&player2, 0, 46*2, 41, 46);
			
		if(pad >> 16 & PADLleft){
			player2.direction = 0;
			sprite_anim(&player2, 41, 46, 0, 0, 6);
			player2.pos.vx -= 2;
		}
		if(pad >> 16 & PADLright){
			player2.direction = 1;
			sprite_anim(&player2, 41, 46, 0, 0, 6);
			player2.pos.vx += 2;
		}
		if(player2.pos.vx + player2.w < 0)	
			player2.pos.vx = SCREEN_WIDTH;
		if(player2.pos.vx > SCREEN_WIDTH)	
			player2.pos.vx = 0;

		FntPrint("hello world");
		drawSprite_2d(&player1);
		drawSprite_2d(&player2);
		drawSprt(&dr_mode, &sprt);

		psDisplay();
	}

	return 0;
}

