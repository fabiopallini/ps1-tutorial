#include "psx.h"

u_long *cd_data[2];
Sprite player[2];
DR_MODE dr_mode;
SPRT sprt;

typedef struct {
	DR_MODE dr_mode;
	SPRT sprt;
} BLOCK;
BLOCK blocks[2];
// 13x7

void loadToVRAM(u_char *image){
	RECT rect;
	GsIMAGE tim;
	// skip the TIM ID and version (magic) by adding 0x4 to the pointer
	GsGetTimInfo ((u_long *)(image+4), &tim);
	// Load pattern into VRAM
	rect.x = tim.px;
	rect.y = tim.py;
	rect.w = tim.pw;
	rect.h = tim.ph;
	LoadImage(&rect, tim.pixel);
	// Load CLUT into VRAM
	rect.x = tim.cx;
	rect.y = tim.cy;
	rect.w = tim.cw;
	rect.h = tim.ch;
	LoadImage(&rect, tim.clut);
}

void sprt_init(DR_MODE *dr_mode, SPRT *sprt){
	SetDrawMode(dr_mode, 0, 0, GetTPage(2, 0, 768, 0), 0);
	SetSprt(sprt);
	sprt->u0 = 0; 
	sprt->v0 = 0;
	sprt->w = 41; 
	sprt->h = 46;
	setRGB0(sprt, 255, 255, 255);
	setXY0(sprt, 20, 20);
}

void block_init(BLOCK *b){
	SetDrawMode(&b->dr_mode, 0, 0, GetTPage(2, 0, 768, 0), 0);
	SetSprt(&b->sprt);
	b->sprt.u0 = 0; 
	b->sprt.v0 = 0;
	b->sprt.w = 32; 
	b->sprt.h = 32;
	setRGB0(&b->sprt, 255, 255, 255);
}

int main() {
	int i = 0;
	psSetup();
	cd_open();
	cd_read_file("PLAYER1.TIM", &cd_data[0]);
	cd_read_file("GFX1.TIM", &cd_data[1]);
	cd_close();
	
	loadToVRAM((u_char*)cd_data[1]);

	sprt_init(&dr_mode, &sprt);
	block_init(&blocks[0]);
	setXY0(&blocks[0].sprt, 50, 50);

	for(i = 0; i <= 1; i++){
		sprite_init(&player[i], 41, 46, cd_data[0]);
		/*if(i == 0)
			sprite_init(&player[i], 41, 46, cd_data[0]);
		else
			sprite_init(&player[i], 41, 46, cd_data[1]);*/
		player[i].direction = 1;
		sprite_set_uv(&player[i], 41, 0, 41, 46);
		player[i].pos.vx = 20;
		player[i].pos.vy = 190; 
	}

	while(1) {
		psClear();

		// PLAYER 1 INPUT
		if((pad & PADLleft) == 0 && (pad & PADLright) == 0)
			sprite_set_uv(&player[0], 0, 46*1, 41, 46);
			
		if(pad & PADLleft){
			player[0].direction = 0;
			sprite_anim(&player[0], 41, 46, 0, 0, 6);
			player[0].pos.vx -= 2;
		}
		if(pad & PADLright){
			player[0].direction = 1;
			sprite_anim(&player[0], 41, 46, 0, 0, 6);
			player[0].pos.vx += 2;
		}
		if(player[0].pos.vx + player[0].w < 0)	
			player[0].pos.vx = SCREEN_WIDTH;
		if(player[0].pos.vx > SCREEN_WIDTH)	
			player[0].pos.vx = 0;

		// PLAYER 2 INPUT
		if((pad2 & PADLleft) == 0 && (pad2 & PADLright) == 0)
			sprite_set_uv(&player[1], 0, 46*1, 41, 46);
			
		if(pad2 & PADLleft){
			player[1].direction = 0;
			sprite_anim(&player[1], 41, 46, 0, 0, 6);
			player[1].pos.vx -= 2;
		}
		if(pad2  & PADLright){
			player[1].direction = 1;
			sprite_anim(&player[1], 41, 46, 0, 0, 6);
			player[1].pos.vx += 2;
		}
		if(player[1].pos.vx + player[1].w < 0)	
			player[1].pos.vx = SCREEN_WIDTH;
		if(player[1].pos.vx > SCREEN_WIDTH)	
			player[1].pos.vx = 0;

		FntPrint("hello world");
		drawSprite_2d(&player[0]);
		drawSprite_2d(&player[1]);

		drawSprt(&dr_mode, &sprt);
		drawSprt(&blocks[0].dr_mode, &blocks[0].sprt);

		psDisplay();
	}

	return 0;
}

