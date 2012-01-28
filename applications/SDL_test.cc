#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>

#define WIDTH 1920
#define HEIGHT 1080
#define BPP 4
#define DEPTH 32

SDL_Surface *screen = NULL;
SDL_Surface *message = NULL;

SDL_Color textColor = {255, 255, 255};

TTF_Font *font = NULL;

// Note: y is actually ytimesw
void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 *pixmem32;
    Uint32 colour;

    colour = SDL_MapRGB( screen->format, r, g, b );

    pixmem32 = (Uint32*) screen->pixels  + y + x;
    *pixmem32 = colour;
}


void DrawScreen(SDL_Surface* screen, int h)
{
    int x, y, ytimesw;

    if(SDL_MUSTLOCK(screen))
    {
        if(SDL_LockSurface(screen) < 0) return;
    }

    for(y = 0; y < screen->h; y++ )
    {
        ytimesw = y*screen->pitch/BPP;
        for( x = 0; x < screen->w; x++ )
        {
            setpixel(screen, x, ytimesw, (x*x)/256+3*y+h, (y*y)/256+x+h, h);
        }
    }

    if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

    SDL_Flip(screen);
}


void DrawRect (SDL_Surface * screen, int cornerx, int cornery, int height, int width) {
    int x, y, ytimesw;

    if(SDL_MUSTLOCK(screen))
    {
        if(SDL_LockSurface(screen) < 0) return;
    }

    for (y = cornery; y < cornery + height; y++) {
        ytimesw = y*screen->pitch/BPP;

        for (x = cornerx; x < cornerx + width; x++) {
            setpixel(screen, x, ytimesw, 0, 255, 0);
        }
    }
    if(SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);

    SDL_Flip(screen);
}


int init () {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 )
    {
        return -1;
    }

    if (!(screen = SDL_SetVideoMode(WIDTH, HEIGHT, DEPTH, SDL_FULLSCREEN | SDL_HWSURFACE)))
    {
        SDL_Quit();
        return -1;
    }
    SDL_WM_SetCaption ("Iris Configuration", NULL);

    if( TTF_Init() < 0) {
        fprintf( stderr, "Could not initalize TTF\n");
        return -1;
    }

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

int load_files() {
    font = TTF_OpenFont( "/usr/share/fonts/corefonts/arial.ttf", 28 );

    if (font == NULL) {
        return -1;
    }
    return 0;
}

void apply_surface (int x, int y, SDL_Surface * source, SDL_Surface * dest) {
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, NULL, dest, &offset);
}

void apply_centered (SDL_Surface * source, SDL_Surface * dest) {
    int x, y;
    x = dest->w / 2 - source->w / 2;
    y = dest->h / 2 - source->h / 2;
    apply_surface( x, y, source, dest );
}

void clean_up () {
    SDL_FreeSurface( message );

    TTF_CloseFont( font );

    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[])
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
        return 1;
    }

    Uint32 background_color = SDL_MapRGB( screen->format, 0x33, 0x99, 0xFF );
    SDL_FillRect(screen, NULL, background_color);


    message = TTF_RenderText_Solid ( font, "Test text", textColor);

    if ( message == NULL ) {
        fprintf( stderr, "Could not render message.\n");
        return 1;
    }

    apply_centered( message, screen );


    SDL_Flip(screen);


    // Wait for a keypress or an exit button push
    while(!keypress)
    {
        //DrawScreen(screen,h++);
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

    clean_up();

    return 0;
}





