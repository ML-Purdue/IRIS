#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <SDL/SDL.h>

#include "videoTest.h"


static const char short_options [] = "d:hmru";
SDL_Surface* screen = NULL;

static void usage (FILE* fp, int argc, char** argv) {
    fprintf (fp,
            "Usage: %s [options]\n\n"
            "Options:\n"
            "-d | --device name   Video device name [/dev/video0]\n"
            "-h | --help   Print this message\n"
            "-m | --mmap   Use memory mapped buffers\n"
            "-r | --read   Use read() calls\n"
            "-u | --userp  Use application allocated buffers\n",
            argv[0]);
}

static const struct option long_options [] = {
    {"device", required_argument, NULL, 'd'},
    {"help", no_argument, NULL, 'h'},
    {"mmap", no_argument, NULL, 'm'},
    {"read", no_argument, NULL, 'r'},
    {"userp", no_argument, NULL, 'u'},
    {0, 0, 0, 0}
};

int main (int argc, char** argv) {
    //Init screen
    SDL_Init(SDL_INIT_EVERYTHING);
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    const char *dev_name = "/dev/video0";
    io_method io = IO_METHOD_MMAP;

    //Parse command-line arguments
    while (1) {
        int index;
        int c = getopt_long (argc, argv, short_options, long_options, &index);

        if (c == -1) break;

        switch (c) {
        case 0: break; /* getopt_long() flag */
        case 'd':
		    dev_name = optarg;
	        break;
        case 'h':
            usage (stdout, argc, argv);
            exit (EXIT_SUCCESS);
            break;
        case 'm':
            io = IO_METHOD_MMAP;
            break;
        case 'r':
            io = IO_METHOD_READ;
            break;
        case 'u':
            io = IO_METHOD_USERPTR;
            break;
        default:
            usage (stderr, argc, argv);
            exit (EXIT_FAILURE);
	        break;
        }
    }

    //Init frame grabbing
    startVideo(dev_name, io);

    //Grab frames and put them on the screen
    SDL_Event event;
    do {
        read_frame(screen->pixels);
        SDL_Flip(screen);
        SDL_PollEvent(&event);
    } while (event.type != SDL_QUIT && event.type != SDL_KEYDOWN);

    //Stop grabbing frames
    stopVideo();

    exit (EXIT_SUCCESS);
}
