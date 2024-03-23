#include "psx.h"
#include "rand.h"

#define n_blocks 65 
#define n_rods 4 
#define SPEED 1 
#define GRAVITY 2 
#define JUMP_SPEED 8 
#define JUMP_FRICTION 0.9 
#define n_balls 2 
#define p1_start_vx 50 
#define p2_start_vx SCREEN_WIDTH - 80
#define BALL_SPAWN_TIME 100
#define BALL_LIFE_TIME 600

u_long *cd_data[4];
u_short tpages[3];
Sprite player[2];
Sprite bullet[2];
int points[2];
char die[2];
char fall[2];
int onRod[2];
u_char skill[2];
int spawner_timer;
int spawner_i;
char firstStart = 1;

typedef enum {
	GUN = 1,
	SHOCK,	
	DEATH,
} SKILL;

typedef struct {
	Sprite sprite;
	int dirX, dirY;
	SKILL skill;
	char active;
	int life_time;
} BALL;
BALL balls[n_balls];

typedef struct {
	DR_MODE dr_mode;
	SPRT sprt;
} BLOCK;
BLOCK blocks[n_blocks];
int block_index = 0;

typedef struct {
	DR_MODE dr_mode;
	SPRT sprt;
} ROD;
ROD rods[n_rods][14];
int rods_length[n_rods];

void gravity(Sprite *s, int n);
int collision(Sprite s1, Sprite s2);
void jump(Sprite *player, int i);
int random(int max);
void playerDie(Sprite *p, int i);
void playerDead(int n);
void balls_spawner();
void skills_action(Sprite *player, int i);
void init_rod(ROD *r);

void init_block(BLOCK *b) {
	SetDrawMode(&b->dr_mode, 0, 0, GetTPage(2, 0, 768, 0), 0);
	SetSprt(&b->sprt);
	b->sprt.u0 = 0; 
	b->sprt.v0 = 0;
	b->sprt.w = 14; 
	b->sprt.h = 7;
	setRGB0(&b->sprt, 255, 255, 200);
}

void init_map() {
	int i = 0;
	int k = 0;
	int seed = 0;
	const int max_plat_blocks = 5;
	int plat_space = 0;
	int col_index = 0;
	int row = 0;
	int col[3];
	int rod_i;
	block_index = 0;

	// reset random rods position out of camera
	for(i = 2; i < n_rods; i++){
		for(rod_i = 0; rod_i < rods_length[i]; rod_i++){
			setXY0(&rods[i][rod_i].sprt, -100, -100);
		}
	}

	if(firstStart == 1){
		firstStart = 0;
		col[0] = 4;
		col[1] = 4;
		col[2] = 2;
		col[3] = 4;
	}
	else
		seed = player[0].pos.vx + player[0].pos.vy + player[1].pos.vx + player[1].pos.vy;

	for(row = 0; row < 4; row++){
		int counter = 0;
		// first row is always the same as the last one
		if(row == 0){
			col[0] = 4;
			col[1] = 2;
			col[2] = 2;
			col[3] = 4;
		}
		if(row > 0 && seed != 0){
			for(i = 0; i < 4; i++){
				seed += i;
				col[i] = random(max_plat_blocks);
				//printf("col[%d] %d\n", i, col[i]);
			}
		}
		//printf("row %d\n", row);
		// columns
		for(i = 0; i < 4; i++){
			for(col_index = 1; col_index <= col[i]; col_index++){
				int x = 0;
				int y = 0;
				int block_y = 48;
				counter++;
				if(counter <= 14){
					x = 20+plat_space+(15*k);			
					//printf("col_index:%d x:%d k:%d \n", col_index, x, k);
					y = block_y*(row+1);
					init_block(&blocks[block_index]);
					setXY0(&blocks[block_index].sprt, x, y);
					k++;
					block_index++;

					if(row == 3 && col_index == (col[i]/2)+1){
						if(i == 1){
							for(rod_i = 0; rod_i < rods_length[2]; rod_i++){
								setXY0(&rods[2][rod_i].sprt, x+6, y-16-(rod_i*16));
							}
						}
						if(i == 2){
							for(rod_i = 0; rod_i < rods_length[3]; rod_i++){
								setXY0(&rods[3][rod_i].sprt, x+6, y-16-(rod_i*16));
							}
						}
					}
				}
			}
			plat_space += 25; // space between platforms
		}
		k = 0;
		plat_space = 0;
	}

	// ROW 5 (last one)
	k = 0;
	for(i = 0; i < 8; i++){
		int x = 0;
		int y = 0;
		int block_y = 48;
		init_block(&blocks[block_index]);
		if(i < 4){
			x = 35+(15*k++);
			y = block_y*5;
		}
		else{
			x = SCREEN_WIDTH-(15*k++);
			y = block_y*5;
		}
		setXY0(&blocks[block_index].sprt, x, y);
		block_index++;
	}
}

void init_ball(BALL *ball){
	sprite_init(&ball->sprite, 16, 16, tpages[1]);
	ball->sprite.direction = 1;
	ball->dirX = 1;
	ball->dirY = 1;
}

void ball_spawn(BALL *ball){
	int r, pos;
	srand(player[0].pos.vx + player[0].pos.vy + player[1].pos.vx + player[1].pos.vy);
	// The first skill is 1, so the random number must be at least 1 (random(..) + 1) 
	r = random(2)+1;
	pos = random(2);
	ball->skill = r;
	sprite_set_uv(&ball->sprite, 32+(16*(ball->skill-1)), 0, 16, 16);
	if(pos == 0){
		ball->sprite.pos.vx = 0;
		ball->sprite.pos.vy = SCREEN_HEIGHT / 2;
	}
	if(pos == 1){
		ball->sprite.pos.vx = SCREEN_WIDTH / 2;
		ball->sprite.pos.vy = 0;
	}
	if(pos == 2){
		ball->sprite.pos.vx = SCREEN_WIDTH - ball->sprite.w;
		ball->sprite.pos.vy = SCREEN_HEIGHT / 2;
	}
	ball->life_time = BALL_LIFE_TIME;
	ball->active = 1;
}

void init_rod(ROD *r) {
	SetDrawMode(&r->dr_mode, 0, 0, GetTPage(2, 0, 768, 0), 0);
	SetSprt(&r->sprt);
	r->sprt.u0 = 16; 
	r->sprt.v0 = 0;
	r->sprt.w = 3; 
	r->sprt.h = 16;
	setRGB0(&r->sprt, 255, 255, 200);
}

void reset_players(){
	int i;
	for(i = 0; i <= 1; i++){
		player[i].direction = (i+1)%2;
		player[i].hitted = 0;
		sprite_set_uv(&player[i], 41, 0, 41, 46);
		player[i].pos.vy = blocks[block_index-1].sprt.y0-player[i].h-10;

		bullet[i].direction = 0;
		sprite_set_uv(&bullet[i], 23, 0, 5, 1);
		bullet[i].pos.vx = -100;
		bullet[i].pos.vy = -100;
	}
	player[0].pos.vx = p1_start_vx;
	onRod[0] = -1;
	fall[0] = 0;
	skill[0] = 0;

	player[1].pos.vx = p2_start_vx; 
	onRod[1] = -1;
	fall[1] = 0;
	skill[1] = 0;
}

void init_players() {
	sprite_init(&player[0], 31, 36, tpages[0]);
	sprite_init(&player[1], 31, 36, tpages[2]);
	sprite_init(&bullet[0], 5, 1, tpages[1]);
	sprite_init(&bullet[1], 5, 1, tpages[1]);
	reset_players();
}

void init_rods() {
	int k, i;
	for(k = 0; k < n_rods; k++){
		for(i = 0; i < rods_length[k]; i++){
			init_rod(&rods[k][i]);
			if(k == 0)
				setXY0(&rods[k][i].sprt, 50, (48*5)-16-(i*16));
			if(k == 1)
				setXY0(&rods[k][i].sprt, 240, (48*5)-16-(i*16));
		}
	}
}

int main() {
	int i = 0;
	psInit();
	cd_open();
	cd_read_file("PLAYER1.TIM", &cd_data[0]);
	cd_read_file("GFX1.TIM", &cd_data[1]);
	cd_read_file("LASER.VAG", &cd_data[2]);
	cd_read_file("PLAYER2.TIM", &cd_data[3]);
	cd_close();
	
	tpages[0] = loadToVRAM(cd_data[0]);
	tpages[1] = loadToVRAM(cd_data[1]);
	tpages[2] = loadToVRAM(cd_data[3]);
	//free3(cd_data);

	for(i = 0; i < n_balls; i++)
		init_ball(&balls[i]);

	onRod[0] = -1;
	onRod[1] = -1;
	rods_length[0] = 14;
	rods_length[1] = 14;
	rods_length[2] = 8;
	rods_length[3] = 8;

	init_map();
	init_players();
	init_rods();

	audio_init();
	audio_vag_to_spu((u_char*)cd_data[2], 27056, SPU_0CH);

	while(1) {
		int k = 0;
		int i = 0;
		int ii = 0;
		psClear();
	
		balls_spawner();
		gravity(&player[0], 0);
		gravity(&player[1], 1);
		jump(&player[0], 0);
		jump(&player[1], 1);

		for(i = 0; i < n_balls; i++)
		{
			if(balls[i].active == 1)
			{
				balls[i].life_time -= 1;
				if(balls[i].life_time > 0)
				{
					if(balls[i].sprite.pos.vx > SCREEN_WIDTH - 16 || balls[i].sprite.pos.vx < 0)
						balls[i].dirX *= -1;
					if(balls[i].sprite.pos.vy > SCREEN_HEIGHT - 16 || balls[i].sprite.pos.vy < 0)
						balls[i].dirY *= -1;
				}

				balls[i].sprite.pos.vx += balls[i].dirX;
				balls[i].sprite.pos.vy += balls[i].dirY;
			}
			if(balls[i].life_time < -300)
				balls[i].active = 0;
			// loop players to check collision
			for(k = 0; k < 2; k++)
			{
				if(balls[i].active == 1 && collision(balls[i].sprite, player[k]) == 1){
					skill[k] = balls[i].skill;
					balls[i].active = 0;
					if(skill[k] == DEATH)
						playerDie(&player[k], k);			
					break;
				}
			}
		}

		if(collision(player[0], player[1]) == 1){
			if(die[0] == 0 && die[1] == 0)
			{
				if(skill[0] == SHOCK && skill[1] != SHOCK)
				{
					skill[0] = 0; 
					playerDie(&player[1], 1);
				}
				else if(skill[1] == SHOCK && skill[0] != SHOCK){
					skill[1] = 0; 
					playerDie(&player[0], 0);
				}
				else 
				{
					player[0].hitted = 10;
					onRod[0] = -1;
					fall[0] = 0;
					player[1].hitted = 10;
					onRod[1] = -1;
					fall[1] = 0;
				}
			}
		}

		for(i = 0; i < 2; i++)
		{
			int inv = (i+1)%2;
			if(bullet[i].direction == 1 && bullet[i].pos.vx <= SCREEN_WIDTH && bullet[i].pos.vx >= 0)
				bullet[i].pos.vx += 4;
			if(bullet[i].direction == 0 && bullet[i].pos.vx + bullet[i].w >= 0 && bullet[i].pos.vx <= SCREEN_WIDTH)
				bullet[i].pos.vx -= 4;

			if(bullet[i].pos.vx + bullet[i].w >= player[inv].pos.vx &&
				bullet[i].pos.vx <= player[inv].pos.vx + player[inv].w &&
				bullet[i].pos.vy + bullet[i].h >= player[inv].pos.vy &&
				bullet[i].pos.vy <= player[inv].pos.vy + player[inv].h - 8){
				playerDie(&player[inv], inv);
				bullet[i].pos.vx = -100;
			}

			if(die[i] > 0){
				die[i]--;
				if(die[i] <= 0)
					playerDead(i);
			}

			if(player[i].hitted > 0){
				player[i].hitted -= 1;
				if(player[i].pos.vx <= player[(i+1)%2].pos.vx){
					player[i].direction = 1;
					player[i].pos.vx -= 4;
				}
				if(player[i].pos.vx >= player[(i+1)%2].pos.vx){
					player[i].direction = 0;
					player[i].pos.vx += 4;
				}
				sprite_set_uv(&player[i], 41*5, 46, 41, 46);
			}

			// PLAYER 1-2 INPUT
			skills_action(&player[i], i);
			if(player[i].action == 0 && player[i].hitted <= 0 && fall[i] == 0 && onRod[i] == -1 &&
			die[0] <= 0 && die[1] <= 0) {
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

		// =============== 
		// 	DRAW
		// =============== 

		FntPrint("	%d							%d", points[0], points[1]);
		for(i = 0; i < n_balls; i++){
			if(balls[i].active == 1)
				drawSprite_2d(&balls[i].sprite);
		}

		drawSprite_2d(&player[0]);
		drawSprite_2d(&player[1]);
		drawSprite_2d(&bullet[0]);
		drawSprite_2d(&bullet[1]);

		for(k = 0; k < n_rods; k++){
			for(i = 0; i < rods_length[k]; i++)
			{
				drawSprt(&rods[k][i].dr_mode, &rods[k][i].sprt);
				// check rod collision
				for(ii = 0; ii < 2; ii++){
					if(player[ii].isJumping == 0 && (pad[ii] & PADLup || pad[ii] & PADLdown)){
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

		for(i = 0; i < block_index; i++)
			drawSprt(&blocks[i].dr_mode, &blocks[i].sprt);

		psDisplay();
	}

	return 0;
}

void gravity(Sprite *s, int n) {
	int i = 0;
	int margin = 10;
	fall[n] = 1; // falling down
	for(i = 0; i < block_index; i++){
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
		playerDead(n);
}

int collision(Sprite s1, Sprite s2){
	int m = 15;
	if(s1.pos.vx + s1.w > s2.pos.vx + m && 
	s1.pos.vx < s2.pos.vx + s2.w - m &&
	s1.pos.vy + s1.h > s2.pos.vy + m &&
	s1.pos.vy < s2.pos.vy + s2.h)
		return 1;
	return 0;
}

void jump(Sprite *player, int i){
	if(player->isJumping == 0 && fall[i] == 0 && onRod[i] == -1 && (opad[i] & PADLsquare) == 0 && pad[i] & PADLsquare){
		player->isJumping = 1;
		if(pad[i] & PADLleft)
			player->isJumping = 2;
		if(pad[i] & PADLright)
			player->isJumping = 3;
		player->jump_speed = JUMP_SPEED;
		player->sideJump_speed = 3;
	}
	if(player->isJumping > 0){
		player->pos.vy -= player->jump_speed;
		player->jump_speed *= JUMP_FRICTION;
		if (player->isJumping == 2){
			player->pos.vx -= player->sideJump_speed;
			player->sideJump_speed *= 0.88;
		}
		if (player->isJumping == 3){
			player->pos.vx += player->sideJump_speed;
			player->sideJump_speed *= 0.95;
		}
	}
}

void skills_action(Sprite *player, int i){
	if(player->action == 0 && (opad[i] & PADLcross) == 0 && pad[i] & PADLcross){
		if(player->hitted <= 0 && fall[i] == 0 && onRod[i] == -1) {
			if(skill[i] == GUN){
				audio_play(SPU_0CH);
				player->action = skill[i];
				skill[i] = 0;
				bullet[i].direction = player->direction;
				sprite_set_uv(&bullet[i], 23, 0, 5, 1);
				if(player->direction == 1)
					bullet[i].pos.vx = player->pos.vx + 39;
				if(player->direction == 0)
					bullet[i].pos.vx = player->pos.vx - 39;
				bullet[i].pos.vy = player->pos.vy + 17;
			}
		}
	}
	if(player->action == GUN)
		if(sprite_anim(player, 41, 46, 1, 2, 3) == 0)
			player->action = 0;
}

void balls_spawner(){
	if(player[0].pos.vx != p1_start_vx || player[1].pos.vx != p2_start_vx)
	{
		spawner_timer++;
		if(spawner_timer >= BALL_SPAWN_TIME){
			spawner_timer = 0;
			if(spawner_i >= n_balls)
				spawner_i = 0;
			if(balls[spawner_i].active == 0)
				ball_spawn(&balls[spawner_i]);	
			spawner_i++;
		}
	}
}

int random(int max){
	return rand() % (max+1);
}

void disable_balls(){
	int i;
	spawner_timer = 0;
	for(i = 0; i < n_balls; i++)
		balls[i].active = 0;
}

void playerDie(Sprite *p, int i){
	die[i] = 80;
	p->hitted = 10;
	onRod[i] = -1;
	fall[i] = 0;
}

void playerDead(int n){
	init_map();
	reset_players();
	// if p1 is dead, set points++ for p2, and vice versa
	points[(n+1) % 2]++;
	disable_balls();
}
