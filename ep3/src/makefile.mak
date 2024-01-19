# ----------------------------
# PlayStation 1 Psy-Q MAKEFILE
# ----------------------------
all:
	ccpsx -O3 -Xo$80010000 -Wall psx.c sprite.c main.c -llibds -o main.cpe
	cpe2x /ce main.cpe

	del main.cpe

	..\cdrom\buildcd.exe -l -i..\cdrom\temp.img ..\cdrom\conf.cti
	..\cdrom\stripiso.exe s 2352 ..\cdrom\temp.img ..\cdrom\game.iso

	del CDW900E.TOC
	del QSHEET.TOC
	del main.exe
	del ..\cdrom\TEMP.IMG 

