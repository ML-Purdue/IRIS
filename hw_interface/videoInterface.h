#ifndef __VIDEO_INTERFACE_H
#define __VIDEO_INTERFACE_H

#include <linux/videodev2.h>

//Data structure definitions
typedef enum {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
} io_method;

struct buffer {
    void* start;
    size_t length;
};

typedef struct {
	int width;
	int height;
    unsigned char* array;
} frame;

//Utility functions
static void errno_exit (const char* s);
static int xioctl (int fd, int request, void* arg);

//Image functions
static void pixel_YUV2RGB(int y, int u, int v, char *r, char *g, char *b);
static void array_YUYV2RGB(void *Dest, const void *src, int width, int height);
void read_frame (void *frame_dest);

//Init functions
void startVideo(const char *dev_name, io_method io);
static void start_capturing (void);
static void init_read(unsigned int buffer_size);
static void init_mmap (void);
static void init_userp (unsigned int buffer_size);
static void init_device (void);

//Uninit functions
void stopVideo();
static void uninit_device (void);
static void stop_capturing (void);

#define CLEAR(x) memset (&(x), 0, sizeof (x))

#endif /* __VIDEO_INTERFACE_H */

