#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <SDL/SDL.h>

#include "wavparam.h"

#define HEIGHT 256
#define DEPTH 32

short *allocbuf(int buflen)
	{
	short *buf=calloc(buflen+10,sizeof(short));
	return buf;
	}

short *buf; // input buffer


void Draw_pixel(SDL_Surface *screen, Uint32 x, Uint32 y, Uint32 color)
{
  Uint32 bpp, ofs;

  bpp = screen->format->BytesPerPixel;
  ofs = screen->pitch*y + x*bpp;

  SDL_LockSurface(screen);
  memcpy(screen->pixels + ofs, &color, bpp);
  SDL_UnlockSurface(screen);
}

#define SGN(x) ((x)>0 ? 1 : ((x)==0 ? 0:(-1)))
#define ABS(x) ((x)>0 ? (x) : (-x))

/* Basic unantialiased Bresenham line algorithm */
static void bresenham_line(SDL_Surface *screen, Uint32 x1, Uint32 y1, Uint32 x2, Uint32 y2,
			   Uint32 color)
{
  int lg_delta, sh_delta, cycle, lg_step, sh_step;

  lg_delta = x2 - x1;
  sh_delta = y2 - y1;
  lg_step = SGN(lg_delta);
  lg_delta = ABS(lg_delta);
  sh_step = SGN(sh_delta);
  sh_delta = ABS(sh_delta);
  if (sh_delta < lg_delta) {
    cycle = lg_delta >> 1;
    while (x1 != x2) {
      Draw_pixel(screen,x1, y1, color);
      cycle += sh_delta;
      if (cycle > lg_delta) {
	cycle -= lg_delta;
	y1 += sh_step;
      }
      x1 += lg_step;
    }
    Draw_pixel(screen, x1, y1, color);
  }
  cycle = sh_delta >> 1;
  while (y1 != y2) {
    Draw_pixel(screen,x1, y1, color);
    cycle += lg_delta;
    if (cycle > sh_delta) {
      cycle -= sh_delta;
      x1 += lg_step;
    }
    y1 += sh_step;
  }
  Draw_pixel(screen, x1, y1, color);
}


void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 *pixmem32;
    Uint32 colour;  
 
    colour = SDL_MapRGB( screen->format, r, g, b );
  
    pixmem32 = (Uint32*) screen->pixels  + y + x;
    *pixmem32 = colour;
}


void DrawScreen(SDL_Surface* screen, int bufsize)
{ 
    int x, y,oldx,oldy;
    oldx=0;oldy=128;
    static int max=-9999;

// Clean
SDL_FillRect( SDL_GetVideoSurface(), NULL, 0 );

// Trigger
/*
buf[0]=0;
while (1) {
	int len=fread(buf,1,2*sizeof(short),stdin);
	if (len!=2*sizeof(short)) exit(0); // EOF
	if (buf[0]>0x100)
		break;
	}
*/
// Read data
int len=fread(buf,1,bufsize*sizeof(short),stdin);
if (len!=bufsize*sizeof(short))
	exit(0); // EOF
// Write data (to use as in pipe)
len = write (1,buf,bufsize*sizeof(short));
// Draw data
    for(x = 0; x < bufsize; x++ ) {
	    y=buf[x]/256+128;
	    if (abs(y)>max) max=abs(y);
       //     setpixel(screen, x, y*bufsize, 255,255,255);
	    bresenham_line(screen, oldx, oldy, x,y,0x00FF00);
	    oldx=x;oldy=y;
		}
    SDL_Flip(screen); 
}


int main(int argc, char* argv[])
{

int buflen=1024;//floor(((double)SAMPLING_RATE)/((double)FREQ));
int WIDTH=buflen;
buf=allocbuf(buflen);


    SDL_Surface *screen;
    SDL_Event event;
  
    int keypress;
  
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;
   
    if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_HWSURFACE)))
    {
        SDL_Quit();
        return 1;
    }

 // Lock screen
    if(SDL_MUSTLOCK(screen)) 
    {
        if(SDL_LockSurface(screen) < 0) return 1;
    }

 // main loop
    while(1) 
    {
         DrawScreen(screen,WIDTH);
         while(SDL_PollEvent(&event)) 
         {      
              switch (event.type) 
              {
                  case SDL_QUIT:
	              keypress = 1;
	              break;
                  case SDL_KEYDOWN:
                       keypress = 1;
                       break;
              }
         }
    }

// unlock screen
    if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);  

    SDL_Quit();
  
    return 0;
}




