#include <SDL/SDL.h>
#include <stdio.h>
#include "cursor.h"
#include <sys/time.h>
#include <time.h>
#include <sched.h>
#include <stdlib.h>

int x = 960;
int y = 540;
SDL_Event event;
time_t last_mouse;
int bClick ;

void PrintKeyInfo(SDL_KeyboardEvent *key);
void PrintModifiers(SDLMod mod);
void DrawCircle(int x, int y,int time_para);

int checkHold();
int check_time();

int main( int argc, char *argv[] ){
    int x,y;
    int quit = 0;
    init_cursor();
    bClick = 0;
    if( SDL_Init( SDL_INIT_VIDEO ) < 0){
        exit( -1 );
    }

    if( !SDL_SetVideoMode( 320, 200, 0, 0 ) ){
        SDL_Quit();
        exit( -1 );
    }

    time(&last_mouse);
    SDL_EnableUNICODE( 1 );
    set_cursor(x, y);
    while( !quit ){
        check_time();
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
        sleep(.05);

    }
    SDL_Quit();
    exit( 0 );
}


int check_time () {
    time_t current_time;
    time(&current_time);
    int DiffTime = current_time - last_mouse;
    if((DiffTime > 2)&&(DiffTime < 5)){
        
    }
 //   printf("current time %d\n", current_time);
  //  printf("last time %d\n", last_mouse);
    if ((DiffTime > 5)&&(bClick == 0)) {
        printf("More than 3 seconds.\n");
        // I left the press funvtion unfinished, because the click function 
        // requires get the status of the button, 
        // now we don't know about the condition about the button, 
        // therefore it is really hard for me to write this part, just
        // print out click. 
        bClick = 1;
     //   time(&last_mouse);
    }

}

int checkHold(){
    int x0, y0, x1, y1;
    time_t start;
    time_t end;
    static int firstCall = 1;

    if (firstCall) {
        firstCall = 0;
        time(&start);
        get_cursor(&x0, &y0);
        return 0;
    }  

    sched_yield();
    get_cursor(&x1, &y1);
    
    if ( (abs(x0-x1)+abs(y0-y1)) > 3 ) {
        x0 = x1;
        y0 = y1;
        time(&start);
        return 0;
    }

    time(&end);
    printf("%lf\n", difftime(end, start) );

    if ( difftime(end, start) > 3 ) {
        time(&start);
        get_cursor(&x0, &y0);
        printf("I have got the cursor\n");
        return 1;
    }

    return 0;
}

void DrawCircle(int x, int y, int time){
   int rx;

   get_cursor(&); 
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
            bClick = 0;
        }

        if(key->keysym.sym == SDLK_DOWN){
            y += 10;
            if(y == 0) exit(0);
            if(y == 980) exit(0);
            key = &event.key;
            set_cursor(x,y);
            time(&last_mouse);
            bClick = 0;
        }

        if(key->keysym.sym == SDLK_RIGHT){
            x+=10;
            if(x == 0) exit(0);
            if(x == 1080) exit(0);
            key =&event.key;
            set_cursor(x,y);
            time(&last_mouse);
            bClick = 0;
        }

        if(key->keysym.sym == SDLK_LEFT){
            x-=10;
            if(x == 0) exit(0);
            if(x == 1080) exit(0);
            key = &event.key;
            set_cursor(x,y);
            time(&last_mouse);
            bClick = 0;
        }

      //  PrintModifiers( key->keysym.mod );
    }
}

/* Print modifier info */
void PrintModifiers( SDLMod mod ){
    printf( "Modifers: \n" );
    if( mod == KMOD_NONE ){
        return;
    }
    printf( "\n" );
}
