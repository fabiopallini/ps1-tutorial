#include "psx.h"

#define n_blocks 55

u_long *cd_data[2];
u_short tpages[2];
Sprite player[2];
char fall[2];
int onRod[2];

typedef struct {
	DR_MODE dr_mode;
	SPRT sprt;
} BLOCK;
BLOCK blocks[n_blocks];

typedef struct {
	DR_MODE dr_mode;
	SPRT sprt;
} ROD;
ROD rods[1][5];

void gravity(Sprite *s, int n);

void init_block(BLOCK *b) {
	SetDrawMode(&b->dr_mode, 0, 0, GetTPage(2, 0, 768, 0), 0);
	SetSprt(&b->sprt);
	b->sprt.u0 = 0; 
	b->sprt.v0 = 0;
	b->sprt.w = 14; 
	b->sprt.h = 7;
	setRGB0(&b->sprt, 255, 255, 255);
}

void init_map() {
	int i = 0;
	int k = 0;
	for(i = 0; i < n_blocks; i++){
		int x = 0;
		int y = 0;
		int block_y = 48;
		init_block(&blocks[i]);

		if(i <= 3){
			x = 40+(15*i); 
			y = block_y*1;
		}
		if(i >= 4 && i <= 7){
			x = SCREEN_WIDTH-(15*i); 
			y = block_y*1;
		}

		if(i >= 8 && i <= 13){
			x = 10+(15*k++);
			y = block_y*2;
		}
		if(i >= 14 && i <= 17){
			x = 40+(15*k++); 
			y = block_y*2;
		}
		if(i >= 18 && i <= 21){
			x = 65+(15*k++); 
			y = block_y*2;
		}

		if(i == 22)
			k = 0;
		if(i >= 22 && i <= 28){
			x = 25+(15*k++); 
			y = block_y*3;
		}
		if(i >= 29 && i <= 32){
			x = 45+(15*k++); 
			y = block_y*3;
		}

		if(i == 33)
			k = 0;
		if(i >= 33 && i <= 35){
			x = 15+(15*k++); 
			y = block_y*4;
		}
		if(i >= 36 && i <= 37){
			x = 35+(15*k++);
		 	y = block_y*4;
		}
		if(i >= 38 && i <= 43){
			x = 55+(15*k++); 
			y =  block_y*4;
		}
		if(i >= 44 && i <= 46){
			x = 75+(15*k++);
			y = block_y*4;
		}

		if(i == 47)
			k = 0;
		if(i >= 47 && i <= 50){
			x = 35+(15*k++);
			y = block_y*5;
		}
		if(i >= 51 && i <= 54){
			x = SCREEN_WIDTH-(15*k++);
			y = block_y*5;
		}
		
		setXY0(&blocks[i].sprt, x, y);
	}
}

void init_rod(ROD *r) {
	SetDrawMode(&r->dr_mode, 0, 0, GetTPage(2, 0, 768, 0), 0);
	SetSprt(&r->sprt);
	r->sprt.u0 = 16; 
	r->sprt.v0 = 0;
	r->sprt.w = 3; 
	r->sprt.h = 16;
	setRGB0(&r->sprt, 255, 255, 255);
}

void init_players() {
	int i;
	for(i = 0; i <= 1; i++){
		sprite_init(&player[i], 31, 36, tpages[0]);
		player[i].direction = 1;
		sprite_set_uv(&player[i], 41, 0, 41, 46);
		player[i].pos.vy = blocks[n_blocks-1].sprt.y0-player[i].h-10;
	}
	player[0].pos.vx = 35;
	player[1].pos.vx = SCREEN_WIDTH - 100; 
}

void init_rods() {
	int i;
	for(i = 0; i < 5; i++){
		init_rod(&rods[0][i]);
		setXY0(&rods[0][i].sprt, 50, (SCREEN_HEIGHT-35)-(i*16));
	}
}

int main() {
	psSetup();
	cd_open();
	cd_read_file("PLAYER1.TIM", &cd_data[0]);
	cd_read_file("GFX1.TIM", &cd_data[1]);
	cd_close();
	
	tpages[0] = loadToVRAM(cd_data[0]);
	tpages[1] = loadToVRAM(cd_data[1]);
	//free3(cd_data);
	
	onRod[0] = -1;
	onRod[1] = -1;

	init_map();
	init_players();
	init_rods();

	while(1) {
		int i = 0;
		psClear();
	
		gravity(&player[0], 0);
		gravity(&player[1], 1);

		// PLAYER 1 INPUT
		if(fall[0] == 0 && onRod[0] == -1) {
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
		}
		if(onRod[0] >= 0) {
			sprite_set_uv(&player[0], 0, 46*3, 41, 46);
			if(pad & PADLup && player[0].pos.vy + player[0].h / 2 > rods[0][4].sprt.y0)
				player[0].pos.vy -= 2;
			if(pad & PADLdown && player[0].pos.vy + player[0].h / 2 < rods[0][0].sprt.y0)
				player[0].pos.vy += 2;
			if((opad & PADLleft) == 0 && pad & PADLleft){
				player[0].pos.vx -= player[0].w / 3;
				onRod[0] = -1;
			}
			if((opad & PADLright) == 0 && pad & PADLright){
				player[0].pos.vx += player[0].w / 3;
				onRod[0] = -1;
			}
		}
		// PLAYER 2 INPUT
		if(fall[1] == 0 && onRod[1] == -1) {
			if((pad2 & PADLleft) == 0 && (pad2 & PADLright) == 0)
				sprite_set_uv(&player[1], 0, 46*1, 41, 46);
				
			if(pad2 & PADLleft){
				player[1].direction = 0;
				sprite_anim(&player[1], 41, 46, 0, 0, 6);
				player[1].pos.vx -= 2;
			}
			if(pad2 & PADLright){
				player[1].direction = 1;
				sprite_anim(&player[1], 41, 46, 0, 0, 6);
				player[1].pos.vx += 2;
			}
			if(player[1].pos.vx + player[1].w < 0)	
				player[1].pos.vx = SCREEN_WIDTH;
			if(player[1].pos.vx > SCREEN_WIDTH)	
				player[1].pos.vx = 0;
		}

		// =============== 
		// 	DRAW
		// =============== 
		//FntPrint("hello world");
		drawSprite_2d(&player[0]);
		drawSprite_2d(&player[1]);

		for(i = 0; i < 5; i++)
		{
			drawSprt(&rods[0][i].dr_mode, &rods[0][i].sprt);
			// check rod collision
			if(pad & PADLup || pad & PADLdown){
				if(player[0].pos.vx + (player[0].w-10) >= rods[0][i].sprt.x0 && 
				player[0].pos.vx + 10 <= rods[0][i].sprt.x0 + rods[0][i].sprt.w &&
				player[0].pos.vy + player[0].h >= rods[0][i].sprt.y0 &&
				player[0].pos.vy <= rods[0][i].sprt.y0)
				{
					onRod[0] = i;
					player[0].pos.vx = rods[0][i].sprt.x0 - player[0].w/2;
				}
			}
		}

		for(i = 0; i < n_blocks; i++)
			drawSprt(&blocks[i].dr_mode, &blocks[i].sprt);

		psDisplay();
	}

	return 0;
}

void gravity(Sprite *s, int n) {
	int i = 0;
	int margin = 10;
	fall[n] = 1; // falling down
	for(i = 0; i < n_blocks; i++){
		if(s->pos.vx + (s->w-margin) >= blocks[i].sprt.x0 && 
		s->pos.vx + margin <= blocks[i].sprt.x0 + blocks[i].sprt.w &&
		s->pos.vy + s->h >= blocks[i].sprt.y0 && 
		s->pos.vy + s->h <= blocks[i].sprt.y0 + 1) 
		{
			fall[n] = 0;
			break;
		}
	}
	//if(fall[n] == 1 && s->pos.vy < SCREEN_HEIGHT - s->h)
	if(fall[n] == 1 && onRod[n] == -1)
		s->pos.vy += 2;

	if(s->pos.vy >= SCREEN_HEIGHT+100)
		init_players();
}

