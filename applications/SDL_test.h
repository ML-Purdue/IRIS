#ifndef SDL_TEST_H
#define SDL_TEST_H


void setpixel(SDL_Surface *screen, int x, int y, Uint8 r, Uint8 g, Uint8 b);

void DrawScreen(SDL_Surface* screen, int h);

void DrawRect (SDL_Surface * screen, int cornerx, int cornery, int height, int width);

int init ();
SDL_Surface *load_image (const char * filename) ;
int load_files() ;
void apply_surface (int x, int y, SDL_Surface * source, SDL_Surface * dest) ;
void apply_centered (SDL_Surface * source, SDL_Surface * dest) ;
void clean_up () ;
int poll_keypress ();
void delay(int i);
#endif
