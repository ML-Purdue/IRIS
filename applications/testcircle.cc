#include<cmath>
#include"SDL.h"
int main(int argc, char* argv[]){
  static const int width = 640;
  static const int height = 480;
  static const int max_radius = 64;
  if (SDL_Init(SDL_INIT_VIDEO) != 0)
    return 1;
  atexit(SDL_Quit);
  SDL_Surface *screen = SDL_SetVideoMode(width, height, 0, SDL_DOUBLEBUF);
  if(screen = NULL)
    return 2;
  while(true)
    {
      SDL_Event event;
      while(SDL_PollEvent(&event))
	{
	  if(event.type=SDL_QUIT)
	    return 0;
	}
      int x = rand() % (width - 4) + 2;
      int y = rand() % (height - 4) + 2;
      int r = rand() % (max_radius - 10) + 10;
      int c = ((rand() % 0xff) << 16) +((rand() % 0xff) << 8) +  (rand() % 0xff); 
      if (r >= 4)
	{
          if (x < r + 2)
            x = r + 2;
	  else if (x > width - r - 2)
            x = width - r - 2;
	}
      f (y < r + 2)
	y = r + 2;
      else if (y > height - r - 2)
	y = height - r - 2;
    }}

