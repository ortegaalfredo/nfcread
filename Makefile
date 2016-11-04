all: passband sdlview

clean:
	rm passband sdlview

CFLAGS= -Wall -O3 -march=native
CFLAGSD= -Wall -O0 -g -march=native

passband : passband.c wavparam.h
	gcc $(CFLAGS) -o passband passband.c -lfftw3 -lm -lSDL
sdlview : sdlview.c
	gcc $(CFLAGSD) -o sdlview sdlview.c -lSDL

