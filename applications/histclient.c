#include <SDL/SDL.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "histogram.h"

#define WIDTH 420
#define HEIGHT 500
#define BPP 4
#define DEPTH 32

#define HIST_W 100

#define HI_THRESH 140
#define LOW_THRESH 50

SDL_Surface * screen = NULL;
SDL_Surface * image = NULL;



int init () {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        return -1;
    }

    if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_HWSURFACE)))
    {
        SDL_Quit();
        return -1;
    }
    SDL_WM_SetCaption ("Histogram", NULL);

    return 0;
}


SDL_Surface *load_image (const char * filename) {
    //Temporary storage for the image that's loaded
    SDL_Surface* loadedImage = NULL;
    //The optimized image that will be used
    SDL_Surface* optimizedImage = NULL;

    loadedImage = SDL_LoadBMP(filename);

    //If nothing went wrong in loading the image
    if( loadedImage != NULL ) {
        //Create an optimized image
        optimizedImage = SDL_DisplayFormat( loadedImage );
        //Free the old image
        SDL_FreeSurface( loadedImage );
    }

    return optimizedImage;

}

int
load_files()
{
    image = load_image("cropc1dan.bmp");
    if (image == NULL) {
        return -1;
    }
    return 0;
}

void clean_up () {
    SDL_FreeSurface(image);

    SDL_Quit();
}

int
main (int argc, char * argv[])
{
    SDL_Event event;

    int keypress = 0;
    int h=0;

    if ( init() < 0 ) {
        fprintf( stderr, "Could not init.\n");
        return 1;
    }


    if ( load_files() < 0 ) {
        fprintf( stderr, "Could not load files.\n");
        fprintf( stderr, "%s\n", SDL_GetError() );
        return 1;
    }


    apply_surface(HIST_W, 0, image, screen);

    struct timeval start, end;
    gettimeofday(&start, NULL);
    draw_hist(image, screen);
    gettimeofday(&end, NULL);

    long seconds  = end.tv_sec  - start.tv_sec;
    long useconds = end.tv_usec - start.tv_usec;

    long mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;



    printf("time: %li\n", mtime);

    SDL_Flip(screen);

    while (1) {

        if (poll_keypress() == 1){
            clean_up();
            return 0;
        }
    }


}
