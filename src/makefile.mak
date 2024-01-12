# ----------------------------
# PlayStation 1 Psy-Q MAKEFILE
# ----------------------------
all:
	ccpsx -O3 -Xo$80010000 -Wall psx.c sprite.c main.c -llibds -o main.cpe
	cpe2x /ce main.cpe

	del main.cpe

	#..\cdrom\buildcd.exe -i..\cdrom\temp.img ..\cdrom\conf.cti
	#..\cdrom\stripiso.exe s 2352 ..\cdrom\temp.img ..\cdrom\game.iso

	..\tools\mkpsxiso\mkpsxiso.exe -o ..\tools\mkpsxiso\game.iso -y ..\tools\mkpsxiso\cuesheet.xml
