/*
 * passband.c: compute FFT of incoming signal and differentiate vote via threshold
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


void detect(short *buf) {

}

double hamming (int i, int nn)
{
    return ( 0.54 - 0.46 * cos (2.0*M_PI*(double)i/(double)(nn-1)) );
}

int main( int argc, char** argv )
{

    int buflen=2048;

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




    fftw_complex    *fft_result;
    fftw_plan       plan_forward;
    double *data;
    int             i,q;

// Largo del ciclo a leer en samples
    int CYCLELEN = 530;
// Ciclos a promediar
    int AverageCycles=3;
// FFT Size
#define SIZE CYCLELEN

// Vote ratio threshold
#define VOTE_THRESHOLD 3

    data        = ( double *) malloc ( sizeof( double ) * SIZE );
    fft_result  = ( fftw_complex* ) fftw_malloc( sizeof( fftw_complex ) * SIZE );
    plan_forward = fftw_plan_dft_r2c_1d(SIZE, data, fft_result,FFTW_ESTIMATE);
    int len;
    while((len=fread(buf,1,buflen*sizeof(short),stdin))==buflen*sizeof(short))
    {

        int window_pointer;
        for(window_pointer=0; window_pointer<(buflen-CYCLELEN*AverageCycles); window_pointer+=200)
        {
            /* populate input data */
            for(i=0; i<CYCLELEN; i++) {
                data[i]=0;
                for(q=0; q<AverageCycles; q++)
                    data[i] += buf[window_pointer+i+(q*CYCLELEN)];
                data[i]/=AverageCycles;
                short outdata=data[i];
                //fwrite(&outdata,sizeof(short),1,stdout);
            }

            /* hamming window */
            //	for (i = 0; i < SIZE; i++)
            //	    data[i] = hamming(i,SIZE) * data[i];

            /* do fft */
            fftw_execute( plan_forward );

            /* normalize */
            float max=-999999;
            for( i = 0 ; i < SIZE ; i++ )  {
                float a=fft_result[i][0];
                float b=fft_result[i][1];
                float power=(a*a+b*b);
                if (max<power) max=power;
            }
            max=10000000000;

            /* print fft result */
            for( i = 1 ; i < 2 ; i++ )  {
                float a=fft_result[i][0];
                float b=fft_result[i][1];
                float power=(a*a+b*b);
                float freq=  (i * (SAMPLING_RATE/2))/(SIZE/2);
                //if (power/max>100)
                // 	fprintf( stdout, "fft_result[%d] = { %2.5f } freq: %2.5f\n",i, power/max,freq);
                //fprintf( stdout, "%2.5f \n",power/max);
            }

            /* bin carrier, and vote freqs */
            float carrier_power=calc_power(fft_result,1,2,max); // 80 Hz (carrier)
            float vote_A_power=calc_power(fft_result,34,42,max); // 2800-3300 Hz (0x0F0F0F0F)
            float vote_B_power=calc_power(fft_result,73,83,max); // 5800-6300 Hz (0x33333333)
            float averagePower=0;
            for( i = 1 ; i < SIZE ; i++ )  {
                float a=fft_result[i][0];
                float b=fft_result[i][1];
                float power=(a*a+b*b);
                averagePower+=power/max;
            }
            averagePower/=SIZE;
            if (averagePower>2) {// disconnected
                DrawScreen(screen,WIDTH,SDL_MapRGB(screen->format, 120, 120, 120));
                continue;
            }
            if ((carrier_power>150)) {
                fprintf( stdout, "carrier: \t %2.5f A: \t%2.5f \tAverage %2.5f ", carrier_power,vote_A_power/vote_B_power,averagePower);
                if ((vote_A_power/vote_B_power)<VOTE_THRESHOLD) {
                    fprintf(stdout, "VOTE B detected (33) %f>%f\n",vote_A_power);
                    DrawScreen(screen,WIDTH,SDL_MapRGB(screen->format, 255, 0, 0));
                }
                else	{
                    fprintf(stdout, "VOTE A detected (0F)\n");
                    DrawScreen(screen,WIDTH,SDL_MapRGB(screen->format, 0, 255, 0));
                }
            }
            else  DrawScreen(screen,WIDTH,0);
        }
    }

    /* free memory */
    fftw_destroy_plan( plan_forward );

    fftw_free( data );
    fftw_free( fft_result );

// unlock screen
    if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

    SDL_Quit();


    return 0;
}

