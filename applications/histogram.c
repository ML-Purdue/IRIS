#include <SDL/SDL.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

#define WIDTH 420
#define HEIGHT 500
#define BPP 4
#define DEPTH 32

#define HIST_W 100

#define HI_THRESH 180
#define LOW_THRESH 50

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


// Generates an array containing the normalized dark-density for each row
// type 0 - upthresh, 1 - downthresh
int *rowhist (SDL_Surface *src, int type) {
    int x, y, ytimesw;
    int *rowsum = (int *) malloc(sizeof(int) * src->h); int rowmax = -1;

    Uint32 *pixmem32 = (Uint32*) src->pixels;
    Uint32 color;

    Uint8 r, g, b;

    for (y = 0; y < src->h; y++) {
        rowsum[y] = 0;
        for (x = 0; x < src->w; x++) {
            color = *pixmem32;
            SDL_GetRGB (color, src->format, &r, &g, &b);
            if (type == 0 && r > HI_THRESH && g > HI_THRESH && b > HI_THRESH) {
                rowsum[y]++;
            } else if (type == 1 && r < LOW_THRESH && g < LOW_THRESH && b < LOW_THRESH) {
                rowsum[y]++;
            }
            pixmem32++;
        }
        if (rowsum[y] > rowmax) {
            rowmax = rowsum[y];
        }
    }

    if (rowmax != 0) {
        for (int y = 0; y < src->h; y++) {
            rowsum[y] *= HIST_W;
            rowsum[y] /= rowmax;
        }
    }

    return rowsum;
}

// Generates an array containing the normalized dark-density for each column
int *colhist (SDL_Surface *src, int type) {
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
            if (type == 0 && r > HI_THRESH && g > HI_THRESH && b > HI_THRESH) {
                colsum[x]++;
            } else if (type == 1 && r < LOW_THRESH && g < LOW_THRESH && b < LOW_THRESH) {
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

    if (colmax != 0) {
        for (x = 0; x < src->w; x++) {
            colsum[x] *= HIST_W;
            colsum[x] /= colmax;
        }
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
    int *rowwhites, *colwhites;
    int *rowblacks, *colblacks;
    rowwhites = rowhist(src, 0);
    colwhites = colhist(src, 0);
    rowblacks = rowhist(src, 1);
    colblacks = colhist(src, 1);


    if(SDL_MUSTLOCK(dest))
    {
        if(SDL_LockSurface(dest) < 0) return -1;
    }

    // Draw histogram for rows
    for (int y = 0; y < src->h; y++) {
        ytimesw = y * dest->pitch / BPP;
        for (int x = 0; x < rowblacks[y]; x++) {
            setpixel(dest, x, ytimesw, 0, 255, 0);
        }
    }

    // Draw histogram for cols
    // TODO: optimize
    for (x = 0; x < src->w; x++) {
        for (int i = colblacks[x]; i > 0; i--) {
            y = HIST_W + src->h - i;
            ytimesw = y * dest->pitch / BPP;
            setpixel(dest, x + HIST_W, ytimesw, 0, 255, 0);
        }
    }

	/*
	// Whites
	int WINDOW = 30;
    int * rowMaxes = localMaxes(rowwhites, src->h, 10);
    int * colMaxes = localMaxes(colwhites, src->w, 10);
    for (y = 0; y < src->h; y++) {
        ytimesw = y * dest->pitch / BPP;
        for (x = 0; x < src->w; x++) {
            if (rowMaxes[y]) {
                setpixel(dest, x + HIST_W, ytimesw, 255, 0, 0);
            }
            if (colMaxes[x]) {
                setpixel(dest, x + HIST_W, ytimesw, 255, 0, 0);
            }
        }
    }
	*/

	// Darks
	int WINDOW = 80;
    int * rowMaxes = localMaxes(rowblacks, src->h, 10);
    int * colMaxes = localMaxes(colblacks, src->w, 10);
    for (y = 0; y < src->h; y++) {
        ytimesw = y * dest->pitch / BPP;
        for (x = 0; x < src->w; x++) {
            if (rowMaxes[y]) {
                setpixel(dest, x + HIST_W, ytimesw, 0, 0, 255);
            }
            if (colMaxes[x]) {
                setpixel(dest, x + HIST_W, ytimesw, 0, 0, 255);
            }
        }
    }


    if(SDL_MUSTLOCK(dest)) SDL_UnlockSurface(dest);

    return 0;
}




