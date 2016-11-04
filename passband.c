    /*
     * passband.c: compute FFT and IFFT from an array
     */
    #include <stdio.h>
    #include <stdlib.h>
    #include <fftw3.h>
    #include <unistd.h>
    #include <math.h>
    #include "wavparam.h"
    #include <SDL/SDL.h>

#define PI (3.141592653589793)

#define HEIGHT 256 
#define WIDTH 512
#define DEPTH 32



     
short *allocbuf(int buflen)
	{
	short *buf=calloc(buflen,sizeof(short));
	return buf;
	}

// calculates average power in spectrum
float calc_power(fftw_complex *fft_result,int min,int max,float normalization) {
	float result=-9999999;
	int i;
        for( i = min ; i <= max ; i++ )  {
           float a=fft_result[i][0];
	   float b=fft_result[i][1];
	   float power=(a*a+b*b)/normalization;
	   if (power>result) result=power;
	}
	return result;
	}

void DrawScreen(SDL_Surface* screen, int bufsize, unsigned int color)
{ 
SDL_FillRect( SDL_GetVideoSurface(), NULL, color );
SDL_Flip(screen); 
};


int main( int argc, char** argv )
{

int SIZE = 32768;
int buflen=SIZE; // five cycles

short *buf=allocbuf(buflen);


    SDL_Surface *screen;
  
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




        fftw_complex    *fft_result, *ifft_result;
        fftw_plan       plan_forward;
	double *data;
        int             i;
       
data        = ( double *) malloc ( sizeof( double ) * SIZE );
fft_result  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * SIZE );
ifft_result = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * SIZE );
plan_forward = fftw_plan_dft_r2c_1d(SIZE, data, fft_result,FFTW_ESTIMATE);
int len;
while((len=fread(buf,1,buflen*sizeof(short),stdin))==buflen*sizeof(short))
	{
        /* populate input data */
   	for(i=0;i<buflen;i++) {
			data[i] = buf[i];
			}
	/* hann window */
	for (i = 0; i < SIZE; i++) {
	    double multiplier = 0.5 * (1 - cos(2*PI*i/(SIZE-1)));
	//    data[i] = multiplier * data[i];
	}

        /* print initial data */
     //   for( i = 0 ; i < SIZE ; i++ ) 
     //       fprintf( stdout, "data[%d] = { %2.2f }\n", i, data[i]);
        
       
        fftw_execute( plan_forward );
       
	/* normalize */
	float max=-999999;
        for( i = 0 ; i < SIZE ; i++ )  {
           float a=fft_result[i][0];
	   float b=fft_result[i][1];
	   float power=(a*a+b*b);
           if (max<power) max=power;
	}
	//max=1;
        /* print fft result */
        for( i = 0 ; i < 40 ; i++ )  {
           float a=fft_result[i][0];
	   float b=fft_result[i][1];
	   float power=(a*a+b*b);
	   float freq=  (i * (SAMPLING_RATE/2))/(SIZE/2);
           //fprintf( stdout, "fft_result[%d] = { %2.5f } freq: %2.5f\n",i, power/max,freq);
	}

	/* bin carrier, and vote freqs */

	float carrier_power=calc_power(fft_result,19,22,max);
	float vote_A_power=calc_power(fft_result,30,37,max);
	float vote_A1_power=calc_power(fft_result,23,29,max);
	float vote_B_power=calc_power(fft_result,67,72,max);
	float vote_B1_power=calc_power(fft_result,61,66,max);
 //       fprintf( stdout, "carrier_power= %2.5f \t vota_A_power = %2.5f \t vote_B_power = %2.5f\n",carrier_power,vote_A_power,vote_B_power);
   //     fprintf( stdout, "carrier_power= %2.5f \t vota_A1_power = %2.5f \t vote_B1_power = %2.5f\n",carrier_power,vote_A1_power,vote_B1_power);
	if (carrier_power>0.90) {
		DrawScreen(screen,WIDTH,SDL_MapRGB(screen->format, 255, 0, 0));
		fprintf(stdout, "carrier detected\n");
		/*
		if (vote_B_power>vote_B1_power)
			fprintf(stdout, "VOTE B detected (33)\n");
		else
		if (vote_A_power>vote_A1_power)
			fprintf(stdout, "VOTE A detected (3F)\n");
		else
			fprintf(stdout, "unknown vote\n");
		*/
		}
	else  DrawScreen(screen,WIDTH,0);

	}
     
   /* free memory */
fftw_destroy_plan( plan_forward );
    
fftw_free( data );
fftw_free( fft_result );
fftw_free( ifft_result );
       
// unlock screen
    if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);  

    SDL_Quit();
  

return 0;
}

