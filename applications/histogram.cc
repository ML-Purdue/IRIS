#include <SDL/SDL.h>
#include <stdio.h>
#include <time.h>

#define WIDTH 420
#define HEIGHT 500
#define BPP 4
#define DEPTH 32

#define HIST_W 100

#define THRESH 50

SDL_Surface * screen = NULL;
SDL_Surface * image = NULL;

int init();
//int build_screen();
int load_files();
int poll_keypress();
SDL_Surface * load_image(const char * filename);
void apply_surface( int x, int y, SDL_Surface * src, SDL_Surface * dest);


int
load_files()
{
    image = load_image("face.bmp");
    if (image == NULL) {
        return -1;
    }
    return 0;
}

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

// Note: y is actually ytimesw
void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b)
{
    Uint32 *pixmem32;
    Uint32 colour;

    colour = SDL_MapRGB( screen->format, r, g, b );

    pixmem32 = (Uint32*) screen->pixels  + y + x;
    *pixmem32 = colour;
}

void apply_surface (int x, int y, SDL_Surface * source, SDL_Surface * dest) {
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, NULL, dest, &offset);
}

int poll_keypress (){
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                return 1;
                break;
            case SDL_KEYDOWN:
                return 1;
                break;
        }
    }
    return 0;

}

void clean_up () {
    SDL_FreeSurface(image);

    SDL_Quit();
}

// Generates an array containing the normalized dark-density for each row
int *rowhist (SDL_Surface *src) {
    int x, y, ytimesw;
    int *rowsum = (int *) malloc(sizeof(int) * src->h);
    int rowmax = -1;

    Uint32 *pixmem32 = (Uint32*) src->pixels;
    Uint32 color;

    Uint8 r, g, b;

    for (y = 0; y < src->h; y++) {
        rowsum[y] = 0;
        for (x = 0; x < src->w; x++) {
            color = *pixmem32;
            SDL_GetRGB (color, src->format, &r, &g, &b);
            if (r < THRESH && g < THRESH && b < THRESH) {
                rowsum[y]++;
            }
            pixmem32++;
        }
        if (rowsum[y] > rowmax) {
            rowmax = rowsum[y];
        }
    }

    for (int y = 0; y < src->h; y++) {
        rowsum[y] *= HIST_W;
        rowsum[y] /= rowmax;
    }

    return rowsum;
}

// Generates an array containing the normalized dark-density for each column
int *colhist (SDL_Surface *src) {
    int x, y, ytimesw;
    int *colsum = (int *) malloc(sizeof(int) * src->w);
    int colmax = -1;

    Uint32 *pixmem32 = (Uint32*) src->pixels;
    Uint32 color;

    Uint8 r, g, b;

    for (x = 0; x < src->w; x++) {
        colsum[x] = 0;
    }

    for (y = 0; y < src->h; y++) {
        for (x = 0; x < src->w; x++) {
            color = *pixmem32;
            SDL_GetRGB (color, src->format, &r, &g, &b);
            if (r < THRESH && g < THRESH && b < THRESH) {
                colsum[x]++;
            }
            pixmem32++;
        }
    }

    for (x = 0; x < src->w; x++) {
        if (colsum[x] > colmax) {
            colmax = colsum[x];
        }
    }

    for (x = 0; x < src->w; x++) {
        colsum[x] *= HIST_W;
        colsum[x] /= colmax;
    }

    return colsum;

}

// finds the local maximums of an array, returns them in maxes
int * localMaxes (int *data, int len, int WINDOW){

    // Find local maximums for rows
    // Generate our initial running sum
    int max = 0;
    int maxloc = 0;
    int winstart;
    int winend;
    int * maxes = (int *) malloc(sizeof(int) * len);
    memset(maxes, 0, sizeof(int) * len);

    for (int i = 0; i < len; i++) {
        winstart = i - WINDOW < 0   ? 0   : i-WINDOW;
        winend   = i + WINDOW > len ? len : i+WINDOW;
        max = 0;
        maxloc = winstart;
        for (int j = winstart; j < winend; j++) {
            if (data[j] > max) {
                max = data[j];
                maxloc = j;
            }
        }

        if (maxloc == i) {
            maxes[i] = 1;
        }
    }
    return maxes;
}


int draw_hist (SDL_Surface *src, SDL_Surface *dest) {
    int x, y, ytimesw;
    int *rowsum, *colsum;
    rowsum = rowhist(src);
    colsum = colhist(src);


    if(SDL_MUSTLOCK(dest))
    {
        if(SDL_LockSurface(dest) < 0) return -1;
    }

    // Draw histogram for rows
    for (int y = 0; y < src->h; y++) {
        ytimesw = y * dest->pitch / BPP;
        for (int x = 0; x < rowsum[y]; x++) {
            setpixel(dest, x, ytimesw, 0, 255, 0);
        }
    }

    // Draw histogram for cols
    // TODO: optimize
    for (x = 0; x < src->w; x++) {
        for (int i = colsum[x]; i > 0; i--) {
            y = HIST_W + src->h - i;
            ytimesw = y * dest->pitch / BPP;
            setpixel(dest, x + HIST_W, ytimesw, 0, 255, 0);
        }
    }

    const int WINDOW = 10;
    int * rowMaxes = localMaxes(rowsum, src->h, 10);
    int * colMaxes = localMaxes(colsum, src->w, 30);
    for (y = 0; y < src->h; y++) {
        ytimesw = y * dest->pitch / BPP;
        for (x = 0; x < src->w; x++) {
            if (rowMaxes[y]) {
                setpixel(dest, x + HIST_W, ytimesw, 255, 0, 0);
            }
            if (colMaxes[x]) {
                setpixel(dest, x + HIST_W, ytimesw, 0, 0, 255);
            }
        }
    }



    if(SDL_MUSTLOCK(dest)) SDL_UnlockSurface(dest);

    /*

    for (int x = 0; x < src->w; x++) {
        winstart = x - WINDOW < 0      ? 0      : x-WINDOW;
        winend   = x + WINDOW > src->w ? src->w : x+WINDOW;
        max = 0;
        maxloc = winstart;
        for (int i = winstart; i < winend; i++) {
            if (colsum[i] > max) {
                max = colsum[i];
                maxloc = i;
            }
        }

        if (maxloc == x) {
            for (int y = 0; y < src->h; y++) {
                ytimesw = y * dest->pitch / BPP;
                setpixel(dest, x + HIST_W, ytimesw, 0, 0, 255);
            }
        }
    }
    */





    return 0;
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

    time_t start = time(&start);
    draw_hist(image, screen);
    time_t end = time(&end);

    double diff = difftime(end, start);

    printf("Drew histograms in %f sec\n", diff);

    SDL_Flip(screen);

    while (true) {

        if (poll_keypress() == 1){
            clean_up();
            return 0;
        }
    }


}

