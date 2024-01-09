# ----------------------------
# PlayStation 1 Psy-Q MAKEFILE
# ----------------------------
all:
	ccpsx -O3 -Xo$80010000 -Wall psx.c sprite.c main.c -llibds -o main.cpe
	cpe2x /ce main.cpe

	del main.cpe