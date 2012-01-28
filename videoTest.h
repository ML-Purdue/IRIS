
#ifdef __VIDEO_TEST_H
#define __VIDEO_TEST_H

//Utility functions
static void errno_exit (const char* s);
static int xioctl (int fd, int request, void* arg);
static void usage (FILE* fp, int argc, char** argv);

//Image functions
static void yuv2rgb(int y, int u, int v, char *r, char *g, char *b);
static void YUYV2RGB(void *Dest, const void *src, int width, int height);
static void process_image (const void* p);
static int read_frame (void);

//Init functions
static void start_capturing (void);
static void init_read(unsigned int buffer_size);
static void init_mmap (void);
static void init_userp (unsigned int buffer_size);
static void init_device (void);

//Uninit functions
static void close_device (void);
static void uninit_device (void);
static void stop_capturing (void);

static void mainloop(void);

#endif /* __VIDEO_TEST_H */

