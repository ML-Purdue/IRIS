#ifndef HISTOGRAM_H
#define HISTOGRAM_H

int poll_keypress();
void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b);
void apply_surface (int x, int y, SDL_Surface * source, SDL_Surface * dest);
int poll_keypress ();
int *rowhist (SDL_Surface *src, int type) ;
int *colhist (SDL_Surface *src, int type) ;
int * localMaxes (int *data, int len, int WINDOW);
int draw_hist (SDL_Surface *src, SDL_Surface *dest) ;

#endif
