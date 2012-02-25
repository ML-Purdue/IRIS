#include <stdio.h>
//For SDL screen display
#include <SDL/SDL.h>

#include "../hw_interface/videoInterface.h"

#include "histogram.h"

#define WIDTH 640
#define HEIGHT 480
#define H_WIDTH 100

typedef struct {
    unsigned short x;
    unsigned short y;
    int intensity;
    int dist;
} pixel;

//Fills the array with pixels greater than a threshold
void fill_array(pixel *array);

//Returns the center of the array
pixel center(pixel *array);

//Remove pixels greater than the average distance from the center
void trim_array(pixel *array);

void drawCrosshair(SDL_Surface *image, int w, int h, int px, int py) {
	int *p = (int *)image->pixels;
	for (int x = px - 5; x > 0 && x < w && x < px + 5; x++) {
		p[py * w + x] = image->format->Gmask;
	}

	for (int y = py - 5; y > 0 && y < h && y < py + 5; y++) {
		p[y * w + px] = image->format->Gmask;
	}
}

void process_image(SDL_Surface *image, int w, int h) {
	int darkestSum = 1000000;
	int dx = 0, dy = 0;

	int *p = (int *)image->pixels;
	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int sum = 0;
			sum += p[y * w + x] & image->format->Rmask;
			sum += p[y * w + x] & image->format->Gmask;
			sum += p[y * w + x] & image->format->Bmask;

			if (sum < darkestSum) {
				darkestSum = sum;
				dx = x;
				dy = y;
			}
		}
	}

	drawCrosshair(image, w, h, dx, dy);
}

int main () {
    //Init screen
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Surface *screen = SDL_SetVideoMode(WIDTH + H_WIDTH, HEIGHT + H_WIDTH, 32, SDL_SWSURFACE);
	SDL_Surface *buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, HEIGHT, 32, 0, 0, 0, 0);

    pixel *array;

    //Init frame grabbing
    startVideo("/dev/video1");

    //Grab frames and put them on the screen
    SDL_Event event;
    do {
		SDL_FillRect(screen, NULL, SDL_MapRGB( screen->format, 0, 0, 0 ));
        read_frame(buffer->pixels);
		process_image(buffer, 640, 480);
		apply_surface(H_WIDTH, 0, buffer, screen);

		draw_hist(buffer, screen);

        SDL_Flip(screen);
        SDL_PollEvent(&event);
    } while (event.type != SDL_QUIT && event.type != SDL_KEYDOWN);

    //Stop grabbing frames
    stopVideo();

    return 0;
}
