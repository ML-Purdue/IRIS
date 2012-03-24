#include <SDL/SDL.h>
//#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include "cursor.h"
#include <sys/time.h>
#include <time.h>
#include <sched.h>

int x = 960;
int y = 540;
SDL_Event event;
time_t last_mouse;

void PrintKeyInfo(SDL_KeyboardEvent *key);
void PrintModifiers(SDLMod mod);
bool checkHold();
int main( int argc, char *argv[] ){
    int x,y;
    int quit = 0;
    init_cursor();
    if( SDL_Init( SDL_INIT_VIDEO ) < 0){

        exit( -1 );
    }

    if( !SDL_SetVideoMode( 320, 200, 0, 0 ) ){
        SDL_Quit();
        exit( -1 );
    }

    SDL_EnableUNICODE( 1 );
    set_cursor(x, y);
    if(checkHold()) {
        printf("More than 3 seconds.");
    }
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
                    checkHold();
                    break;
            }
        }

    }
    SDL_Quit();
    exit( 0 );
}

bool checkHold(){
    int x0, y0, x1, y1;
    time_t start;
    time_t end;
    static bool firstCall = true;

    if (firstCall) {

        firstCall = false;
        time(&start);
        get_cursor(&x0, &y0);
        return false;
    }  

    sched_yield();

    get_cursor(&x1, &y1);

    if ( (abs(x0-x1)+abs(y0-y1)) > 3 ) {
        puts("aaa");
        x0 = x1;
        y0 = y1;
        time(&start);
        return false;
    }

    time(&end);
    printf("%lf\n", difftime(end, start) );


    if ( difftime(end, start) > 3 ) {
        time(&start);
        get_cursor(&x0, &y0);
        return true;
    }

    return false;
}

void drawcircle(int x, int y){
}


void PrintKeyInfo( SDL_KeyboardEvent *key ){

    if( key->type == SDL_KEYUP ){
        printf( "Release:- " );
    }
    else{
        if (key->keysym.sym == SDLK_UP) {
            y -= 10;
            if(y == 0) exit(0);
            if(y == 980) exit(0);
            key = &event.key;
            set_cursor(x,y);
            time(&last_mouse);
        }

        if(key->keysym.sym == SDLK_DOWN){
            y += 10;
            if(y == 0) exit(0);
            if(y == 980) exit(0);
            key = &event.key;
            set_cursor(x,y);
            time(&last_mouse);
        }

        if(key->keysym.sym == SDLK_RIGHT){
            x+=10;
            if(x == 0) exit(0);
            if(x == 1080) exit(0);
            key =&event.key;
            set_cursor(x,y);
            time(&last_mouse);
        }

        if(key->keysym.sym == SDLK_LEFT){
            x-=10;
            if(x == 0) exit(0);
            if(x == 1080) exit(0);
            key = &event.key;
            set_cursor(x,y);
            time(&last_mouse);
        }

        time_t current_time;
        time(&current_time);
        if (current_time - last_mouse > 3 seconds) {
            printf("More than 3 seconds.");
            
    }
    PrintModifiers( key->keysym.mod );
}

/* Print modifier info */
void PrintModifiers( SDLMod mod ){
    printf( "Modifers: " );
    if( mod == KMOD_NONE ){
        return;
    }
    printf( "\n" );
}
