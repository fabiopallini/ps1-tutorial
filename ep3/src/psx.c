#include "psx.h"

#define OTSIZE 1024

DISPENV	dispenv[2];
DRAWENV	drawenv[2];
int dispid = 0;
u_long ot[OTSIZE];
u_short otIndex;

void psSetup()
{
	ResetCallback();
	ResetGraph(0);
	PadInit(0);

	if(*(char *)0xbfc7ff52=='E')
		SetVideoMode(MODE_PAL);
	else
		SetVideoMode(MODE_NTSC);

	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 1);
	SetGeomOffset(160, 120);
	SetGeomScreen(512);

	SetDefDrawEnv(&drawenv[0], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	SetDefDrawEnv(&drawenv[1], 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
	SetDefDispEnv(&dispenv[0], 0, SCREEN_HEIGHT,SCREEN_WIDTH, SCREEN_HEIGHT);
	SetDefDispEnv(&dispenv[1], 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	SetDispMask(1);

	drawenv[0].isbg = 1;
	setRGB0(&drawenv[0], 0,0,0);
	drawenv[1].isbg = 1;
	setRGB0(&drawenv[1], 0,0,0);

	// load the font from the BIOS into the framebuffer
	FntLoad(960, 256);
	// screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters
	SetDumpFnt(FntOpen(5, 5, 320, 240, 0, 512));

	//init stack 16KB heap 2 megabyte
	InitHeap3((void*)0x800F8000, 0x00200000);
}

void psDisplay(){
	opad = pad;

	DrawSync(0);
	VSync(0);

	PutDispEnv(&dispenv[dispid]);
	PutDrawEnv(&drawenv[dispid]);

	//DrawOTag(ot);
	DrawOTag(ot+OTSIZE-1);

	FntFlush(-1);
	otIndex = 0;
}

void psClear(){
	dispid = (dispid + 1) %2;
	//ClearOTag(ot, OTSIZE);
	ClearOTagR(ot, OTSIZE);
	pad = PadRead(0);

	RotMatrix(&camera.rot, &camera.mtx);
	ApplyMatrixLV(&camera.mtx, &camera.pos, &camera.tmp);	
	TransMatrix(&camera.mtx, &camera.tmp);
	SetRotMatrix(&camera.mtx);
	SetTransMatrix(&camera.mtx);
}

void psExit(){
	PadStop();
	StopCallback();
}

void psGte(VECTOR pos, SVECTOR *rot)
{
	//0'-360' = 0-4096
	MATRIX m;
	RotMatrix(rot, &m);
	TransMatrix(&m, &pos);
	CompMatrixLV(&camera.mtx, &m, &m);
	SetRotMatrix(&m);
	SetTransMatrix(&m);
}

// LOAD DATA FROM CD-ROM
int didInitDs = 0;
SpuCommonAttr l_c_attr;
SpuVoiceAttr  g_s_attr;
unsigned long l_vag1_spu_addr;

void cd_open() {
	if(!didInitDs) {
		didInitDs = 1;
		DsInit();
	}
}

void cd_close() {
	if(didInitDs) {
		didInitDs = 0;
		DsClose();
	}
}

void cd_read_file(unsigned char* file_path, u_long** file) {

	u_char* file_path_raw;
	int* sectors_size;
	DslFILE* temp_file_info;
	sectors_size = malloc3(sizeof(int));
	temp_file_info = malloc3(sizeof(DslFILE));

	// Exit if libDs isn't initialized
	if(!didInitDs) {
		printf("LIBDS not initialized, run cdOpen() first\n");	
		return;
	}

	// Get raw file path
	file_path_raw = malloc3(4 + strlen(file_path));
	strcpy(file_path_raw, "\\");
	strcat(file_path_raw, file_path);
	strcat(file_path_raw, ";1");
	printf("Loading file from CD: %s\n", file_path_raw);

	// Search for file on disc
	DsSearchFile(temp_file_info, file_path_raw);

	// Read the file if it was found
	if(temp_file_info->size > 0) {
		printf("...file found\n");
		printf("...file size: %lu\n", temp_file_info->size);
		*sectors_size = temp_file_info->size + (SECTOR % temp_file_info->size);
		printf("...file buffer size needed: %d\n", *sectors_size);
		printf("...sectors needed: %d\n", (*sectors_size + SECTOR - 1) / SECTOR);
		*file = malloc3(*sectors_size + SECTOR);

		DsRead(&temp_file_info->pos, (*sectors_size + SECTOR - 1) / SECTOR, *file, DslModeSpeed);
		while(DsReadSync(NULL));
		printf("...file loaded!\n");
	} else {
		printf("...file not found");
	}

	// Clean up
	free3(file_path_raw);
	free3(sectors_size);
	free3(temp_file_info);
}

void drawSprite(Sprite *sprite){
	long otz;
	setVector(&sprite->vector[0], -sprite->w, -sprite->h, 0);
	setVector(&sprite->vector[1], sprite->w, -sprite->h, 0);
	setVector(&sprite->vector[2], -sprite->w, sprite->h, 0);
	setVector(&sprite->vector[3], sprite->w, sprite->h, 0);
	psGte(sprite->pos, &sprite->rot);
	sprite->poly.tpage = sprite->tpage;
	RotTransPers(&sprite->vector[0], (long *)&sprite->poly.x0, 0, 0);
	RotTransPers(&sprite->vector[1], (long *)&sprite->poly.x1, 0, 0);
	RotTransPers(&sprite->vector[2], (long *)&sprite->poly.x2, 0, 0);
	otz = RotTransPers(&sprite->vector[3], (long *)&sprite->poly.x3, 0, 0);
	if(otz > 0 && otz < OTSIZE)
		AddPrim(ot+otz, &sprite->poly);
}

static void moveSprite(Sprite *sprite, long x, long y){
	sprite->poly.x0 = x;
	sprite->poly.y0 = y;
	sprite->poly.x1 = x + sprite->w;
	sprite->poly.y1 = y;
	sprite->poly.x2 = x;
	sprite->poly.y2 = y + sprite->h;
	sprite->poly.x3 = x + sprite->w;
	sprite->poly.y3 = y + sprite->h;
}

void drawSprite_2d(Sprite *sprite){
	moveSprite(sprite, sprite->pos.vx, sprite->pos.vy);
	sprite->poly.tpage = sprite->tpage;
	AddPrim(&ot[otIndex++], &sprite->poly);
}

void drawSprite_2d_rgb(Sprite *sprite){
	long x = sprite->pos.vx;
	long y = sprite->pos.vy;
	sprite->poly_rgb.x0 = x;
	sprite->poly_rgb.y0 = y;
	sprite->poly_rgb.x1 = x + sprite->w;
	sprite->poly_rgb.y1 = y;
	sprite->poly_rgb.x2 = x;
	sprite->poly_rgb.y2 = y + sprite->h;
	sprite->poly_rgb.x3 = x + sprite->w;
	sprite->poly_rgb.y3 = y + sprite->h;
	AddPrim(&ot[otIndex++], &sprite->poly_rgb);
}

void drawSprt(DR_MODE *dr_mode, SPRT *sprt){
	AddPrim(&ot[otIndex++], sprt);
	AddPrim(&ot[otIndex++], dr_mode);
}
