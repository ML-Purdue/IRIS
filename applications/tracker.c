#include <stdio.h>
//For SDL screen display
#include <SDL/SDL.h>
#include <math.h>

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

typedef struct {
    int length;
    pixel *array;
} pix_array;

typedef struct {
    unsigned short w;
    unsigned short h;
    int *img;
	SDL_PixelFormat *fmt;
} frame;

float getBrightness(int pixel, SDL_PixelFormat *fmt) {
	unsigned char r, g, b;
	SDL_GetRGB(pixel, fmt, &r, &g, &b);
	return (r + g + b) / 3.0;
}

void blit (int x, int y, SDL_Surface * source, SDL_Surface * dest) {
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, NULL, dest, &offset);
}

//Fills the array with pixels greater than a threshold
void fill_array(pix_array *pix, frame *f, int threshold);

//Returns the center of the array
pixel center(pix_array pix, int w, int h);

//Remove pixels greater than the average distance from the center
void trim_array(pix_array *pix, pixel center);

void fill_array(pix_array *pix, frame *f, int threshold){
    for(int y = 0; y < f->h; y++){
        for(int x = 0; x < f->w; x++){
            int intensity = getBrightness(f->img[y * f->w + x], f->fmt);
			/* if (y % 400 == 0 && x % 500 == 0) printf("intensity %d\n", intensity); */
            if(intensity < threshold){
                pix->array[pix->length].x = x;
                pix->array[pix->length].y = y;
                pix->array[pix->length].intensity = intensity;
                pix->array[pix->length].dist = 0;
                pix->length++;
            }
        }
    }
}

//Returns the center of the array
pixel center(pix_array pix, int w, int h){
    pixel center_pixel; 
	if (pix.length == 0) {
		center_pixel.x = 7;
		center_pixel.y = 0;
		center_pixel.intensity = 0;
		center_pixel.dist = 0;
		return center_pixel;
	}
    unsigned short sum_x = 0;
    unsigned short sum_y = 0;
    int sum_intensity = 0;
    for(int i = 0; i < pix.length;  i++){
        sum_x = sum_x + pix.array->x;
        sum_y = sum_y + pix.array->y; 
        sum_intensity = sum_intensity + pix.array->intensity;
    }
    unsigned short center_x = sum_x/w; 
    unsigned short center_y = sum_y/h; 
    int center_intensity = sum_intensity/pix.length; 
    center_pixel.x = center_x;
    center_pixel.y = center_y;
    center_pixel.intensity = center_intensity; 
    center_pixel.dist = 0;
    return center_pixel;
}

//Remove pixels greater than the average distance from the center
void trim_array(pix_array *p_array, pixel center) {
	int centerX = center.x;
	int centerY = center.y;
	
	unsigned int sum = 0;
	for(int i=0; i < p_array->length; i++) {
		p_array->array[i].dist = 0.5 + sqrt(( p_array->array[i].x - centerX ) * ( p_array->array[i].x - centerX )
									  + ( p_array->array[i].y - centerY ) * ( p_array->array[i].y - centerY ));
		sum += p_array->array[i].dist;
	}
	
	int average = sum / p_array->length;
	int t = 0;
	for(int i = 0; i < p_array->length; i++) {
		if (p_array->array[i].dist <= average) {
			p_array->array[t++] = p_array->array[i];
		}
	}
	p_array->length = t; 
}


void drawCrosshair(SDL_Surface *image, int w, int h, int px, int py) {
    int *p = (int *)image->pixels;
    for (int x = px - 5; x > 0 && x < w && x < px + 5; x++) {
        p[py * w + x] = image->format->Gmask;
    }

    for (int y = py - 5; y > 0 && y < h && y < py + 5; y++) {
        p[y * w + px] = image->format->Gmask;
    }
}

void process_image(SDL_Surface *image, SDL_Surface *distribution, int w, int h) {
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
    SDL_Surface *screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE);
    SDL_Surface *buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, HEIGHT - H_WIDTH, 32, 0, 0, 0, 0);

	frame f = { 640, 480, screen->pixels, screen->format };
	pix_array pix = { 0, malloc(WIDTH * HEIGHT * sizeof(pixel)) };

    //Init frame grabbing
    startVideo("/dev/video0");

    //Grab frames and put them on the screen
    SDL_Event event;
    do {
        read_frame(screen->pixels);
		memcpy(f.img, screen->pixels, WIDTH * HEIGHT * sizeof(int));
		pix.length = 0;
		fill_array(&pix, &f, 50);
		pixel c = center(pix, WIDTH, HEIGHT);
		if (c.x != 0) {
			printf("Drawing crosshair x %d y %d\n", c.x, c.y);
			drawCrosshair(screen, WIDTH, HEIGHT, c.x, c.y);
		}
        /* process_image(screen, buffer, 640, 480); */

        /* blit(0, 480 - H_WIDTH, buffer, screen); */
        SDL_Flip(screen);
    } while (!SDL_PollEvent(&event) || event.type != SDL_QUIT && event.type != SDL_KEYDOWN);

	end:

    //Stop grabbing frames
    stopVideo();

    return 0;
}
