#include <stdio.h>
//For SDL screen display
#include <SDL/SDL.h>
#include <math.h>

#include "../hw_interface/videoInterface.h"

#include "histogram.h"

#define WIDTH 640
#define HEIGHT 480
#define H_WIDTH 100
#define DARK 1
#define LIGHT 0

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
void fill_array(pix_array *pix, frame *f, int threshold, int pick_dark);

//Returns the center of the array
pixel center(pix_array pix, int w, int h);

//Remove pixels greater than the average distance from the center
void trim_array(pix_array *pix, pixel center, int dist);

void fill_array(pix_array *pix, frame *f, int threshold, int pick_dark){
    for(int y = 0; y < f->h; y++){
        for(int x = 0; x < f->w; x++){
            int intensity = getBrightness(f->img[y * f->w + x], f->fmt);
            if((pick_dark && intensity < threshold) || (!pick_dark && intensity > threshold)){
            //if((intensity - threshold) ^ pick_dark){
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

    //sanity check
    if (pix.length == 0) {
        center_pixel.x = 7;
        center_pixel.y = 0;
        center_pixel.intensity = 0;
        center_pixel.dist = 0;
        return center_pixel;
    }

    unsigned long sum_x = 0;
    unsigned long sum_y = 0;
    int sum_intensity = 0;
    for(int i = 0; i < pix.length;  i++){
        sum_x = sum_x + pix.array[i].x;
        sum_y = sum_y + pix.array[i].y;
        sum_intensity = sum_intensity + pix.array[i].intensity;
    }

    unsigned long center_x = sum_x/pix.length;
    unsigned long center_y = sum_y/pix.length;
    int center_intensity = sum_intensity/pix.length;

    center_pixel.x = (unsigned short)center_x;
    center_pixel.y = (unsigned short)center_y;
    center_pixel.intensity = center_intensity;
    center_pixel.dist = 0;

    return center_pixel;
}

//Remove pixels greater than the average distance from the center
void trim_array(pix_array *p_array, pixel center, int dist) {
    int centerX = center.x;
    int centerY = center.y;

    if(p_array->length == 0){
        return;
    }

    //unsigned int sum = 0;
    for(int i=0; i < p_array->length; i++) {
        p_array->array[i].dist = 0.5 + sqrt(( p_array->array[i].x - centerX ) * ( p_array->array[i].x - centerX )
                                      + ( p_array->array[i].y - centerY ) * ( p_array->array[i].y - centerY ));
        //sum += p_array->array[i].dist;
    }

    //int average = sum / p_array->length;
    int t = 0;
    for(int i = 0; i < p_array->length; i++) {
        if (p_array->array[i].dist <= dist) {
            p_array->array[t++] = p_array->array[i];
        }
    }
    p_array->length = t;
}

int avgDist (pix_array *p_array, pixel center) {
    long dist = 0;
    int i;

    for (i = 0; i < p_array->length; i++) {
        dist += sqrt(( p_array->array[i].x - center.x ) * ( p_array->array[i].x - center.x )
                   + ( p_array->array[i].y - center.y ) * ( p_array->array[i].y - center.y ));
    }

    dist /= p_array->length;
    return (int)dist;
}

int circle_err (pix_array *p_array, pixel center, int radius) {
    int * distances;
    int i, dist, error;

    distances = (int *) malloc(radius * sizeof(int));

    for (i = 0; i < p_array->length; i++) {
        dist = sqrt(( p_array->array[i].x - center.x ) * ( p_array->array[i].x - center.x )
                  + ( p_array->array[i].y - center.y ) * ( p_array->array[i].y - center.y ));
        if (dist < radius) {
            distances[dist]++;
        }
    }

    error = 0;
    for (i = 1; i < radius; i++) {
        int s = distances[i-1] * distances[i-1] - distances[i];
        error += (s > 0) ? s : -s;
    }

    free(distances);
    return error;
}



void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {

   Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
   *(Uint32 *)target_pixel = pixel;

}

// Super quick circle, uses no trig!
// n_cx and n_cy are coords of the center
void draw_circle(SDL_Surface *surface, int n_cx, int n_cy, int radius, Uint32 pixel) {

	double error = (double)-radius;
	double x = (double)radius -0.5;
	double y = (double)0.5;
	double cx = n_cx - 0.5;
	double cy = n_cy - 0.5;

	while (x >= y)
	{
	   set_pixel(surface, (int)(cx + x), (int)(cy + y), pixel);
	   set_pixel(surface, (int)(cx + y), (int)(cy + x), pixel);

	   if (x != 0)
	   {
		   set_pixel(surface, (int)(cx - x), (int)(cy + y), pixel);
		   set_pixel(surface, (int)(cx + y), (int)(cy - x), pixel);
	   }

	   if (y != 0)
	   {
		   set_pixel(surface, (int)(cx + x), (int)(cy - y), pixel);
		   set_pixel(surface, (int)(cx - y), (int)(cy + x), pixel);
	   }

	   if (x != 0 && y != 0)
	   {
		   set_pixel(surface, (int)(cx - x), (int)(cy - y), pixel);
		   set_pixel(surface, (int)(cx - y), (int)(cy - x), pixel);
	   }

	   error += y;
	   ++y;
	   error += y;

	   if (error >= 0)
	   {
		   --x;
		   error -= x;
		   error -= x;
	   }
	}
}



void drawCrosshair(SDL_Surface *image, int w, int h, int px, int py, int mask) {
    int *p = (int *)image->pixels;
    for (int x = px - 5; x > 0 && x < w && x < px + 5; x++) {
        p[py * w + x] = mask;
    }

    for (int y = py - 5; y > 0 && y < h && y < py + 5; y++) {
        p[y * w + px] = mask;
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

    drawCrosshair(image, w, h, dx, dy, image->format->Gmask);
}

int main () {
    //Init screen
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Surface *screen = SDL_SetVideoMode(WIDTH, HEIGHT, 32, SDL_SWSURFACE);
    SDL_Surface *buffer = SDL_CreateRGBSurface(SDL_SWSURFACE, WIDTH, HEIGHT - H_WIDTH, 32, 0, 0, 0, 0);

    frame f = { 640, 480, screen->pixels, screen->format };
    pix_array pix_dark = { 0, malloc(WIDTH * HEIGHT * sizeof(pixel)) };
    pix_array pix_light = { 0, malloc(WIDTH * HEIGHT * sizeof(pixel)) };

    //Init frame grabbing
    startVideo("/dev/video0");

    //Grab frames and put them on the screen
    SDL_Event event;
    pixel prev_dark = { 0 };
    pixel curr_dark = { 0 };
    pixel prev_light = { 0 };
    pixel curr_light = { 0 };

    int keypress = 0;

    do {
        read_frame(screen->pixels);
        memcpy(f.img, screen->pixels, WIDTH * HEIGHT * sizeof(int));

        //Find dark center
        pix_dark.length = 0;
        fill_array(&pix_dark, &f, 20, DARK);

        printf("darklen: %d\n", pix_dark.length);

        trim_array(&pix_dark, prev_dark, 50);

        curr_dark = center(pix_dark, WIDTH, HEIGHT);
        if(curr_dark.x != 0 && curr_dark.y != 0)
            prev_dark = curr_dark;

        if (curr_dark.x != 0 && curr_dark.y != 0) {
            printf("Drawing dark crosshair x %u y %u\n", curr_dark.x, curr_dark.y);
            drawCrosshair(screen, WIDTH, HEIGHT, curr_dark.x, curr_dark.y, screen->format->Gmask);

            // Circle the blob of darkness
            int radius = 2 * avgDist(&pix_dark, curr_dark);
            printf("Radius of darkenss is %i with circle_error %i\n", radius, circle_err(&pix_dark, curr_dark, radius));
            draw_circle(screen, curr_dark.x, curr_dark.y, radius, screen->format->Rmask);

        }else{
            printf("Drawing dark crosshair x %u y %u\n", prev_dark.x, prev_dark.y);
            drawCrosshair(screen, WIDTH, HEIGHT, prev_dark.x, prev_dark.y, screen->format->Gmask);
        }

        //Find light center
        pix_light.length = 0;
        fill_array(&pix_light, &f, 100, LIGHT);

        //printf("lightlen: %d\n", pix_light.length);

        trim_array(&pix_light, curr_dark, 50);

        curr_light = center(pix_light, WIDTH, HEIGHT);
        if(curr_light.x != 0 && curr_light.y != 0)
            prev_light = curr_light;

        if (curr_light.x != 0 && curr_light.y != 0) {
            //printf("Drawing light crosshair x %u y %u\n", curr_light.x, curr_light.y);
            drawCrosshair(screen, WIDTH, HEIGHT, curr_light.x, curr_light.y, screen->format->Bmask);
        }else{
            //printf("Drawing light crosshair x %u y %u\n", prev_light.x, prev_light.y);
            drawCrosshair(screen, WIDTH, HEIGHT, prev_light.x, prev_light.y, screen->format->Bmask);
        }

        SDL_Flip(screen);
        while (SDL_PollEvent(&event)){
            switch (event.type) {
                case SDL_QUIT:
                case SDL_KEYDOWN :
                    keypress = 1;
                    break;

                case SDL_MOUSEBUTTONDOWN :
                    printf("Changing center to %i, %i\n", event.button.x, event.button.y);
                    prev_dark.x = event.button.x;
                    prev_dark.y = event.button.y;
                    break;
            }
        }
    } while (!keypress);

    end:

    //Stop grabbing frames
    stopVideo();

    return 0;
}
