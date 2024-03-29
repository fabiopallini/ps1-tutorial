#include "psx.h"

#define n_blocks 55
#define n_rods 4
#define SPEED 1 
#define GRAVITY 2 
#define JUMP_SPEED 10 
#define JUMP_FRICTION 0.9 
#define n_balls 1 

u_long *cd_data[2];
u_short tpages[2];
Sprite player[2];
char fall[2];
int onRod[2];
int skill[2];

typedef struct {
	Sprite sprite;
	int dirX, dirY;
	int skill;
	char active;
} BALL;
BALL balls[n_balls];

typedef struct {
	DR_MODE dr_mode;
	SPRT sprt;
} BLOCK;
BLOCK blocks[n_blocks];

typedef struct {
	DR_MODE dr_mode;
	SPRT sprt;
} ROD;
ROD rods[n_rods][14];
int rods_length[n_rods];

void gravity(Sprite *s, int n);
int collision(Sprite s1, Sprite s2);
void jump(Sprite *player, int i);

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

void init_ball(BALL *ball){
	sprite_init(&ball->sprite, 16, 16, tpages[1]);
	ball->sprite.direction = 1;
	sprite_set_uv(&ball->sprite, 32, 0, 16, 16);
	ball->dirX = 1;
	ball->dirY = 1;
}

void ball_skill(BALL *ball, int skill){
	ball->skill = skill;
	sprite_set_uv(&ball->sprite, 32+(16*skill), 0, 16, 16);
}

void ball_spawn(BALL *ball){
	ball->active = 1;
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
	int k, i;
	for(k = 0; k < n_rods; k++){
		for(i = 0; i < rods_length[k]; i++){
			init_rod(&rods[k][i]);
			if(k == 0)
				setXY0(&rods[k][i].sprt, 50, (48*5)-16-(i*16));
			if(k == 1)
				setXY0(&rods[k][i].sprt, 90, (48*4)-16-(i*16));
			if(k == 2)
				setXY0(&rods[k][i].sprt, 240, (48*5)-16-(i*16));
			if(k == 3)
				setXY0(&rods[k][i].sprt, 200, (48*4)-16-(i*16));
		}
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

	init_ball(&balls[0]);
	ball_spawn(&balls[0]);

	onRod[0] = -1;
	onRod[1] = -1;
	//rods_length[0] = 5;
	//rods_length[0] = 8;
	//rods_length[0] = 11;
	//rods_length[0] = 14;
	//
	rods_length[0] = 14;
	rods_length[1] = 5;
	rods_length[2] = 14;
	rods_length[3] = 5;

	init_map();
	init_players();
	init_rods();

	while(1) {
		int k = 0;
		int i = 0;
		int ii = 0;
		psClear();
	
		gravity(&player[0], 0);
		gravity(&player[1], 1);
		jump(&player[0], 0);
		jump(&player[1], 1);

		if(collision(balls[0].sprite, player[0]) == 1){
			balls[0].active = 0;
			skill[0] = balls[0].skill;
		}

		if(collision(player[0], player[1]) == 1){
			player[0].hitted = 10;
			onRod[0] = -1;
			fall[0] = 0;
			player[1].hitted = 10;
			onRod[1] = -1;
			fall[1] = 0;
		}
		
		if(player[0].hitted > 0 && player[1].hitted > 0){	
			player[0].hitted -= 1;
			player[1].hitted -= 1;
			if(player[0].pos.vx <= player[1].pos.vx){
				player[0].direction = 1;
				player[0].pos.vx -= 4;
				player[1].direction = 0;
				player[1].pos.vx += 4;
			}
			if(player[0].pos.vx >= player[1].pos.vx){
				player[0].direction = 0;
				player[0].pos.vx += 4;
				player[1].direction = 1;
				player[1].pos.vx -= 4;
			}
			sprite_set_uv(&player[0], 41*5, 46, 41, 46);
			sprite_set_uv(&player[1], 41*5, 46, 41, 46);
		}

		// PLAYER 1-2 INPUT
		for(i = 0; i < 2; i++){
			if(player[i].hitted <= 0 && fall[i] == 0 && onRod[i] == -1) {
				if((pad[i] & PADLleft) == 0 && (pad[i] & PADLright) == 0)
					sprite_set_uv(&player[i], 0, 46*1, 41, 46);
					
				if(pad[i] & PADLleft){
					player[i].direction = 0;
					sprite_anim(&player[i], 41, 46, 0, 0, 6);
					player[i].pos.vx -= SPEED;
				}
				if(pad[i] & PADLright){
					player[i].direction = 1;
					sprite_anim(&player[i], 41, 46, 0, 0, 6);
					player[i].pos.vx += SPEED;
				}
				if(player[i].pos.vx + player[i].w < 0)	
					player[i].pos.vx = SCREEN_WIDTH;
				if(player[i].pos.vx > SCREEN_WIDTH)	
					player[i].pos.vx = 0;
			}
			if(onRod[i] >= 0) {
				sprite_set_uv(&player[i], 0, 46*3, 41, 46);

				if(pad[i] & PADLup && player[i].pos.vy + player[i].h / 2 > rods[ onRod[i] ] [ rods_length[ onRod[i]]-1 ].sprt.y0)
					player[i].pos.vy -= SPEED;

				if(pad[i] & PADLdown && player[i].pos.vy + player[i].h / 2 < rods[ onRod[i] ] [ 0 ].sprt.y0 - 4)
					player[i].pos.vy += SPEED;

				if((opad[i] & PADLleft) == 0 && pad[i] & PADLleft){
					player[i].pos.vx -= player[i].w / 3;
					onRod[i] = -1;
					player[i].direction = 0;
					sprite_set_uv(&player[i], 41, 46, 41, 46);
				}

				if((opad[i] & PADLright) == 0 && pad[i] & PADLright){
					player[i].pos.vx += player[i].w / 3;
					onRod[i] = -1;
					player[i].direction = 1;
					sprite_set_uv(&player[i], 41, 46, 41, 46);
				}
			}
		}

		for(i = 0; i < n_balls; i++){
			if(balls[i].active == 1){
				if(balls[i].sprite.pos.vx >= SCREEN_WIDTH - 16 || balls[i].sprite.pos.vx < 0)
					balls[i].dirX *= -1;
				if(balls[i].sprite.pos.vy >= SCREEN_HEIGHT - 16 || balls[i].sprite.pos.vy < 0)
					balls[i].dirY *= -1;

				balls[i].sprite.pos.vx += balls[i].dirX;
				balls[i].sprite.pos.vy += balls[i].dirY;
			}
		}

		// =============== 
		// 	DRAW
		// =============== 

		//FntPrint("hello world");
		for(i = 0; i < n_balls; i++){
			if(balls[i].active == 1)
				drawSprite_2d(&balls[i].sprite);
		}

		drawSprite_2d(&player[0]);
		drawSprite_2d(&player[1]);

		for(k = 0; k < n_rods; k++){
			for(i = 0; i < rods_length[k]; i++)
			{
				drawSprt(&rods[k][i].dr_mode, &rods[k][i].sprt);
				// check rod collision
				for(ii = 0; ii < 2; ii++){
					if(pad[ii] & PADLup || pad[ii] & PADLdown){
						if(player[ii].pos.vx + (player[ii].w-10) >= rods[k][i].sprt.x0 && 
						player[ii].pos.vx + 10 <= rods[k][i].sprt.x0 + rods[k][i].sprt.w &&
						player[ii].pos.vy + player[ii].h >= rods[k][i].sprt.y0 &&
						player[ii].pos.vy <= rods[k][i].sprt.y0)
						{
							onRod[ii] = k;
							player[ii].pos.vx = rods[k][i].sprt.x0 - player[ii].w/2;
						}
					}
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
			s->isJumping = 0;
			break;
		}
	}
	//if(fall[n] == 1 && s->pos.vy < SCREEN_HEIGHT - s->h)
	if(fall[n] == 1 && onRod[n] == -1){
		s->pos.vy += GRAVITY;
		sprite_anim(s, 41, 46, 1, 1, 1);
	}

	if(s->pos.vy >= SCREEN_HEIGHT+100)
		init_players();
}

int collision(Sprite s1, Sprite s2){
	if(s1.pos.vx + s1.w/2 > s2.pos.vx && 
	s1.pos.vx < s2.pos.vx + s2.w/2 &&
	s1.pos.vy + s1.h/2 > s2.pos.vy &&
	s1.pos.vy < s2.pos.vy + s2.h/2)
		return 1;
	return 0;
}

void jump(Sprite *player, int i){
	if(fall[i] == 0 && onRod[i] == -1 && (opad[i] & PADLsquare) == 0 && pad[i] & PADLsquare){
		player->isJumping = 1;
		if(pad[i] & PADLleft)
			player->isJumping = 2;
		if(pad[i] & PADLright)
			player->isJumping = 3;
		player->jump_speed = JUMP_SPEED;
		player->sideJump_speed = 8;
	}
	if(player->isJumping > 0){
		player->pos.vy -= player->jump_speed;
		player->jump_speed *= JUMP_FRICTION;
		player->sideJump_speed *= 0.9;
		if (player->isJumping == 2)
			player->pos.vx -= player->sideJump_speed;
		if (player->isJumping == 3)
			player->pos.vx += player->sideJump_speed;
	}
}

void action(Sprite *player, int i){
	if((opad[i] & PADLcross) == 0 && pad[i] & PADLcross){
		if(skill[i] == 1){
			// shooot
		}	
	}
}
