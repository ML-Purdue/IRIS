#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include "cursor.h"

int x = 960;
int y = 540;
SDL_Event event;
void PrintKeyInfo(SDL_KeyboardEvent *key);
void PrintModifiers(SDLMod mod);

int main( int argc, char *argv[] ){
	int x,y;
	int quit = 0;
	init_cursor();
	if( SDL_Init( SDL_INIT_VIDEO ) < 0){
		fprintf( stderr, "Could not initialise SDL: %s\n", SDL_GetError() );
		exit( -1 );
	}

	if( !SDL_SetVideoMode( 320, 200, 0, 0 ) ){
		fprintf( stderr, "Could not set video mode: %s\n", SDL_GetError() );
		SDL_Quit();
		exit( -1 );
	}

	SDL_EnableUNICODE( 1 );
	set_cursor(x, y);
	while( !quit ){
		while( SDL_PollEvent( &event ) ){
			switch( event.type ){
				case SDL_KEYDOWN:
					PrintKeyInfo( &event.key );
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				default:
					break;
			}

		}

	}
	SDL_Quit();
	exit( 0 );
}

void PrintKeyInfo( SDL_KeyboardEvent *key ){
	/* Is it a release or a press? */
	printf("The previous y cordinates is %d\n",y);
	if( key->type == SDL_KEYUP ){
		printf( "Release:- " );
	}
	else{
		if (key->keysym.sym == SDLK_UP) {
			printf( "Success Right!\n");
			y -= 1;
			if(y == 0) exit(0);
			if(y == 980) exit(0);
			printf("The after y coordinate is %d\n",y);
			key = &event.key;
			set_cursor(x,y);
		}
		if(key->keysym.sym == SDLK_DOWN){
			printf("success down\n");
			y += 1;
			if(y == 0) exit(0);
			if(y == 980) exit(0);
			printf("The after y coordinates is %i\n",y);
			key = &event.key;
			set_cursor(x,y);
		}
		if(key->keysym.sym == SDLK_RIGHT){
			printf("success right\n");
			x+=1;
			if(x == 0) exit(0);
			if(x == 1080) exit(0);
			printf("The after x coordinates is %i\n",x);
			key =&event.key;
			set_cursor(x,y);
		}
		if(key->keysym.sym == SDLK_LEFT){
			printf("success left\n");
			x-=1;
			if(x == 0) exit(0);
			if(x == 1080) exit(0);
			printf("The after x coordinates is %i\n",x);
			key = &event.key;
			set_cursor(x,y);
		}
	}
	PrintModifiers( key->keysym.mod );
}

/* Print modifier info */
void PrintModifiers( SDLMod mod ){
	printf( "Modifers: " );

	/* If there are none then say so and return */
	if( mod == KMOD_NONE ){
		printf( "None\n" );
		return;
	}
	printf( "\n" );
}
