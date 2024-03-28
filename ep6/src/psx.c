#include "psx.h"

#define SOUND_MALLOC_MAX 3 
#define OTSIZE 1024 
#define BILLBOARDS 0

typedef struct
{                
	DRAWENV draw;
	DISPENV disp;
	u_long ot[OTSIZE];
	int ot_index;
} DB;
DB db[2];
DB *cdb;

static SpriteNode *createSprite(Sprite *data) {
	SpriteNode* newNode = malloc3(sizeof(SpriteNode));
	if (newNode == NULL) {
		printf("error on SpriteNode malloc3 \n");
		return NULL; 
	}
	newNode->data = data;
	newNode->next = NULL;
	return newNode;
}

void psInit()
{
	ResetCallback();
	ResetGraph(0);
	PadInit(0);

	#ifdef PAL
		SetVideoMode(MODE_PAL);
	#else
		SetVideoMode(MODE_NTSC);
	#endif

	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 1);
	SetGeomOffset(160, 120);
	SetGeomScreen(512);

	SetDefDrawEnv(&db[0].draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	SetDefDrawEnv(&db[1].draw, 0, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
	SetDefDispEnv(&db[0].disp, 0, SCREEN_HEIGHT,SCREEN_WIDTH, SCREEN_HEIGHT);
	SetDefDispEnv(&db[1].disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	#ifdef PAL
		db[0].disp.screen.x = 0;
		db[0].disp.screen.y = 16;
		db[0].disp.screen.w = 256;
		db[0].disp.screen.h = 256;

		db[1].disp.screen.x = 0;
		db[1].disp.screen.y = 16;
		db[1].disp.screen.w = 256;
		db[1].disp.screen.h = 256;
	#endif

	SetDispMask(1);

	db[0].draw.isbg = 1;
	setRGB0(&db[0].draw, 0,0,0);
	db[1].draw.isbg = 1;
	setRGB0(&db[1].draw, 0,0,0);

 	// load the font from the BIOS into the framebuffer
	FntLoad(960, 256);
	// screen X,Y | max text length X,Y | autmatic background clear 0,1 | max characters
	font_id = FntOpen(5, 5, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 512);
	SetDumpFnt(font_id);

	//init stack 16KB heap 2 megabyte
	InitHeap3((void*)0x800F8000, 0x00200000);
}

void psClear(){
	pad[0] = PadRead(0);
	pad[1] = pad[0] >> 16;

	cdb = (cdb==db) ? db+1 : db;
	ClearOTag(cdb->ot, 1024);     
	cdb->ot_index = 0;     

	RotMatrix(&camera.rot, &camera.mtx);
	ApplyMatrixLV(&camera.mtx, &camera.pos, &camera.tmp);	
	TransMatrix(&camera.mtx, &camera.tmp);
	SetRotMatrix(&camera.mtx);
	SetTransMatrix(&camera.mtx);
}

void psDisplay(){
	opad[0] = pad[0];
	opad[1] = pad[1];
	FntFlush(font_id);
	DrawSync(0);
	VSync(0);
	PutDrawEnv(&cdb->draw);
	PutDispEnv(&cdb->disp);
	DrawOTag(cdb->ot);
}

void psExit(){
	PadStop();
	StopCallback();
}

void psGte(VECTOR pos, SVECTOR rot){
	//0'-360' = 0-4096
	MATRIX m;
	RotMatrix(&rot, &m);
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

u_short loadToVRAM(u_long *image){
	RECT rect;
	GsIMAGE tim;
	// skip the TIM ID and version (magic) by adding 0x4 to the pointer
	/* 
 		image+4 if passing u_char*, image+1 if passing u_long*
		u_char == 1 byte, u_long == 4 byte
	*/
	GsGetTimInfo(image+1, &tim);
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
	return GetTPage(tim.pmode, 1, tim.px, tim.py);
}

// AUDIO PLAYER
void audio_init() {
	SpuInit();
	SpuInitMalloc (SOUND_MALLOC_MAX, (char*)(SPU_MALLOC_RECSIZ * (SOUND_MALLOC_MAX + 1)));
	l_c_attr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR);
	l_c_attr.mvol.left  = 0x3fff; // set master left volume
	l_c_attr.mvol.right = 0x3fff; // set master right volume
	SpuSetCommonAttr (&l_c_attr);
}

void audio_vag_to_spu(u_char* sound_data, u_long sound_size, int voice_channel) {
	SpuSetTransferMode (SpuTransByDMA); // set transfer mode to DMA
	l_vag1_spu_addr = SpuMalloc(sound_size); // allocate SPU memory for sound 1
	SpuSetTransferStartAddr(l_vag1_spu_addr); // set transfer starting address to malloced area
	SpuWrite(sound_data, sound_size); // perform actual transfer
	SpuIsTransferCompleted (SPU_TRANSFER_WAIT); // wait for DMA to complete
	g_s_attr.mask =
	(
		SPU_VOICE_VOLL |
		SPU_VOICE_VOLR |
		SPU_VOICE_PITCH |
		SPU_VOICE_WDSA |
		SPU_VOICE_ADSR_AMODE |
		SPU_VOICE_ADSR_SMODE |
		SPU_VOICE_ADSR_RMODE |
		SPU_VOICE_ADSR_AR |
		SPU_VOICE_ADSR_DR |
		SPU_VOICE_ADSR_SR |
		SPU_VOICE_ADSR_RR |
		SPU_VOICE_ADSR_SL
	);

	g_s_attr.voice = (voice_channel);

	g_s_attr.volume.left  = 0x1fff;
	g_s_attr.volume.right = 0x1fff;

	g_s_attr.pitch        = 0x1000;
	g_s_attr.addr         = l_vag1_spu_addr;
	g_s_attr.a_mode       = SPU_VOICE_LINEARIncN;
	g_s_attr.s_mode       = SPU_VOICE_LINEARIncN;
	g_s_attr.r_mode       = SPU_VOICE_LINEARDecN;
	g_s_attr.ar           = 0x0;
	g_s_attr.dr           = 0x0;
	g_s_attr.sr           = 0x0;
	g_s_attr.rr           = 0x0;
	g_s_attr.sl           = 0xf;

	SpuSetVoiceAttr (&g_s_attr);
}

void audio_play(int voice_channel) {
	SpuSetKey(SpuOn, voice_channel);
}

void audio_free(unsigned long spu_address) {
	SpuFree(spu_address);
}

void drawSprite(Sprite *sprite){
	long otz;
	setVector(&sprite->vector[0], -sprite->w, -sprite->h, 0);
	setVector(&sprite->vector[1], sprite->w, -sprite->h, 0);
	setVector(&sprite->vector[2], -sprite->w, sprite->h, 0);
	setVector(&sprite->vector[3], sprite->w, sprite->h, 0);
	psGte(sprite->pos, sprite->rot);
	sprite->poly.tpage = sprite->tpage;
	RotTransPers(&sprite->vector[0], (long *)&sprite->poly.x0, 0, 0);
	RotTransPers(&sprite->vector[1], (long *)&sprite->poly.x1, 0, 0);
	RotTransPers(&sprite->vector[2], (long *)&sprite->poly.x2, 0, 0);
	otz = RotTransPers(&sprite->vector[3], (long *)&sprite->poly.x3, 0, 0);
	if(otz > 0 && otz < OTSIZE)
		AddPrim(cdb->ot+otz, &sprite->poly);
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
	AddPrim(cdb->ot+cdb->ot_index, &sprite->poly);
	cdb->ot_index++;
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
	AddPrim(cdb->ot+cdb->ot_index, &sprite->poly_rgb);
	cdb->ot_index++;
}

void drawSprt(DR_MODE *dr_mode, SPRT *sprt){
	AddPrim(cdb->ot+cdb->ot_index, sprt);
	AddPrim(cdb->ot+cdb->ot_index, dr_mode);
	cdb->ot_index++;
}

void scene_add_sprite(Sprite *data) {
	SpriteNode *last;
	SpriteNode **head = &scene.spriteNode;
	SpriteNode *newNode = createSprite(data);
	if (*head == NULL) {
		*head = newNode;
		return;
	}
	last = *head;
	while (last->next != NULL) {
		last = last->next;
	}
	last->next = newNode;
}

void printSpriteNode(SpriteNode *head) {
	SpriteNode *current = head;
	while (current != NULL) {
		printf("SpriteNode->pos.vx %ld \n", current->data->pos.vx);
		current = current->next;
	}
}

void scene_freeSprites(){
	SpriteNode *current = scene.spriteNode;
	while (current != NULL) {
		SpriteNode *nextNode = current->next;
		free(current);
		current = nextNode;
	}
}

