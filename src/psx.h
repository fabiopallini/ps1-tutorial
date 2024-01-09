#ifndef PSX_H
#define PSX_H

#include <stdlib.h>
#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <stdio.h>
#include <libspu.h>
#include <libds.h>
#include <strings.h>
#include <libmath.h>
#include <libapi.h>

#include "sprite.h"

#define SCREEN_WIDTH 320
#define	SCREEN_HEIGHT 256
#define SECTOR 2048

#define FNT_HEIGHT 29 
#define FNT_WIDTH 39 

#define PADLsquare 128
#define PADLcircle 32 
#define PADLcross 64 
#define PADLtriangle 16 

#define PI 3.14

typedef struct {
	VECTOR pos;
	SVECTOR rot;
	MATRIX mtx;
	VECTOR tmp;
} Camera;

Camera camera;

typedef struct SpriteNode {
	Sprite *data;
	struct SpriteNode *next;
} SpriteNode;

typedef struct {
	SpriteNode *spriteNode;
} Scene;

Scene scene;
u_long pad, opad;
long font_id[2];

void psSetup();
void psClear();
void psExit();
void psGte(VECTOR pos, SVECTOR *ang);
void psDisplay();
void psAddPrimF4(POLY_F4 *poly);
void psAddPrimFT4(POLY_FT4 *poly);
void psAddPrimFT4otz(POLY_FT4 *poly, long otz);
void cd_open();
void cd_close();
void cd_read_file(unsigned char* file_path, u_long** file);
void drawSprite(Sprite *sprite);
void drawSprite_2d(Sprite *sprite);
void drawSprite_2d_rgb(Sprite *sprite);

#endif
