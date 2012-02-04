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
#include <linux/videodev2.h>
#include <SDL/SDL.h>

#include "videoTest.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef enum {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
} io_method;

struct buffer {
    void* start;
    size_t length;
};

static char* dev_name = NULL;
static io_method io = IO_METHOD_MMAP;
static int fd = -1;
struct buffer* buffers = NULL;
static unsigned int n_buffers = 0;
static const char short_options [] = "d:hmru";
SDL_Surface* screen = NULL;

static void errno_exit (const char* s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror (errno));
    exit(EXIT_FAILURE);
}

static int xioctl (int fd, int request, void* arg) {
    int r;
    do r = ioctl(fd, request, arg);
    while (r == -1 && errno == EINTR);
    return r;
}

// SDL format: 32 bits: 8 red, 8 green, 8 blue, 8 alpha
// 0xrrggbbaa

//                p1    p2
// YUYV format: [Y1,U][Y2,V]
// YCbCr = YUV

void yuv2rgb(int y, int u, int v, char *r, char *g, char *b) {
    int r1, g1, b1;
    int c = y-16, d = u - 128, e = v - 128;

    r1 = (298 * c           + 409 * e + 128) >> 8;
    g1 = (298 * c - 100 * d - 208 * e + 128) >> 8;
    b1 = (298 * c + 516 * d           + 128) >> 8;

    // Even with proper conversion, some values still need clipping.

    if (r1 > 255) r1 = 255;
    if (g1 > 255) g1 = 255;
    if (b1 > 255) b1 = 255;
    if (r1 < 0) r1 = 0;
    if (g1 < 0) g1 = 0;
    if (b1 < 0) b1 = 0;

    *r = r1 ;
    *g = g1 ;
    *b = b1 ;
}

void YUYV2RGB(void *Dest, const void *src, int width, int height) {
 unsigned char *p = (unsigned char *)src, *dest = (unsigned char *)Dest;
 for(int y = 0; y < height; y++) {
  unsigned char *line = p + y * width *2;
  for(int x = 0; x < width; x+=2) {
   int Y0 = line[x*2];
   int U = line[x*2+1];
   int Y1 = line[x*2+2];
   int V = line[x*2+3];

   char r0, g0, b0, r1, g1, b1;
   yuv2rgb(Y0, U, V, &r0, &g0, &b0);
   yuv2rgb(Y1, U, V, &r1, &g1, &b1);
   int i = (y*width+x)*4;
   dest[i+0] = b0;
   dest[i+1] = g0;
   dest[i+2] = r0;
   dest[i+3] = 255;
   dest[i+4] = b1;
   dest[i+5] = g1;
   dest[i+6] = r1;
   dest[i+7] = 255;
  }
 }
}

static void process_image (const void* p) {
 unsigned char* d = (unsigned char*)p;
 /*for (int i = 0; i < 640*480*2; i++) {
  printf("%d ", *((unsigned char*)p+i));
 }
 printf("\n");
 fflush(stdout);*/
 printf("%02x %02x %02x %02x %02x\n",d[0],d[1],d[2],d[3],d[4]);

 YUYV2RGB(screen->pixels, p, 640, 480);
 SDL_Flip(screen);
}

static int read_frame (void) {
    struct v4l2_buffer buf;
    unsigned int i;

    switch (io) {
        case IO_METHOD_READ:
            if (-1 == read (fd, buffers[0].start, buffers[0].length)) {
                switch (errno) {
                    case EAGAIN: return 0;
                    case EIO: /* Could ignore EIO, see spec. fall through */
                    default: errno_exit ("read");
                }
            }

            process_image (buffers[0].start);
            break;
        case IO_METHOD_MMAP:
            CLEAR (buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN: return 0;
                    case EIO: /* Could ignore EIO, see spec. fall through */
                    default: errno_exit ("VIDIOC_DQBUF");
                }
            }

            assert (buf.index < n_buffers);
            process_image (buffers[buf.index].start);

            if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                errno_exit ("VIDIOC_QBUF");
            break;
        case IO_METHOD_USERPTR:
            CLEAR (buf);

            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;

            if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
                switch (errno) {
                    case EAGAIN: return 0;
                    case EIO: /* Could ignore EIO, see spec. fall through */
                    default: errno_exit ("VIDIOC_DQBUF");
                }
            }

            for (i = 0; i < n_buffers; ++i)
                if (buf.m.userptr == (unsigned long)buffers[i].start
                        && buf.length == buffers[i].length) break;

            assert(i < n_buffers);
            process_image((void*)buf.m.userptr);

            if (xioctl(fd, VIDIOC_QBUF, &buf) == -1) errno_exit ("VIDIOC_QBUF");

            break;
    }

    return 2;
}

static void mainloop(void) {
    unsigned int count;

    int keypress = 0;
    SDL_Event event;

    while (!keypress) {
        for (;;) {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO (&fds);
            FD_SET (fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select (fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                if (EINTR == errno) continue;
                errno_exit ("select");
            }

            if (0 == r) {
                fprintf (stderr, "select timeout\n");
                exit (EXIT_FAILURE);
            }

            if (read_frame ()) break;
            /* EAGAIN - continue select loop. */
        }

        // Read all keypress events
        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    keypress = 1;
                    break;
                case SDL_KEYDOWN:
                    keypress = 1;
                    break;
            }
        }
    }
}

static void stop_capturing (void) {
    enum v4l2_buf_type type;

    switch (io) {
        case IO_METHOD_READ: /* Nothing to do. */ break;
        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                                                  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

                                                  if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type))
                                                      errno_exit ("VIDIOC_STREAMOFF");

                                                  break;
    }
}

static void start_capturing (void) {
    unsigned int i;
    enum v4l2_buf_type type;

    switch (io) {
        case IO_METHOD_READ: /* Nothing to do. */ break;
        case IO_METHOD_MMAP:
                                                  for (i = 0; i < n_buffers; ++i) {
                                                      struct v4l2_buffer buf;

                                                      CLEAR (buf);

                                                      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                                                      buf.memory      = V4L2_MEMORY_MMAP;
                                                      buf.index       = i;

                                                      if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                                                          errno_exit ("VIDIOC_QBUF");
                                                  }

                                                  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

                                                  if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
                                                      errno_exit ("VIDIOC_STREAMON");

                                                  break;
        case IO_METHOD_USERPTR:
                                                  for (i = 0; i < n_buffers; ++i) {
                                                      struct v4l2_buffer buf;

                                                      CLEAR (buf);

                                                      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                                                      buf.memory      = V4L2_MEMORY_USERPTR;
                                                      buf.index       = i;
                                                      buf.m.userptr   = (unsigned long) buffers[i].start;
                                                      buf.length      = buffers[i].length;

                                                      if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                                                          errno_exit ("VIDIOC_QBUF");
                                                  }

                                                  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

                                                  if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
                                                      errno_exit ("VIDIOC_STREAMON");

                                                  break;
    }
}

static void uninit_device (void) {
    unsigned int i;

    switch (io) {
        case IO_METHOD_READ: free(buffers[0].start);
                             break;
        case IO_METHOD_MMAP:
                             for (i = 0; i < n_buffers; ++i)
                                 if (-1 == munmap (buffers[i].start, buffers[i].length))
                                     errno_exit ("munmap");
                             break;

        case IO_METHOD_USERPTR:
                             for (i = 0; i < n_buffers; ++i)
                                 free(buffers[i].start);
                             break;
    }

    free(buffers);

    //close device
    if (-1 == close (fd))
        errno_exit ("close");

    fd = -1;
}

static void init_read(unsigned int buffer_size) {
    buffers = calloc(1, sizeof (*buffers));

    if (!buffers) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    buffers[0].length = buffer_size;
    buffers[0].start = malloc (buffer_size);

    if (!buffers[0].start) {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }
}

static void init_mmap (void) {
    struct v4l2_requestbuffers req;

    CLEAR (req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s does not support memory mapping\n", dev_name);
            exit (EXIT_FAILURE);
        } else errno_exit ("VIDIOC_REQBUFS");
    }

    if (req.count < 2) {
        fprintf (stderr, "Insufficient buffer memory on %s\n", dev_name);
        exit (EXIT_FAILURE);
    }

    buffers = calloc (req.count, sizeof (*buffers));

    if (!buffers) {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;

        if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
            errno_exit ("VIDIOC_QUERYBUF");

        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start =
            mmap (NULL /* start anywhere */,
                    buf.length,
                    PROT_READ | PROT_WRITE /* required */,
                    MAP_SHARED /* recommended */,
                    fd, buf.m.offset);

        if (MAP_FAILED == buffers[n_buffers].start)
            errno_exit ("mmap");
    }
}

static void init_userp (unsigned int buffer_size) {
    struct v4l2_requestbuffers req;
    unsigned int page_size;

    page_size = sysconf(_SC_PAGESIZE);
    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

    CLEAR (req);

    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s does not support user pointer i/o\n", dev_name);
            exit (EXIT_FAILURE);
        } else errno_exit ("VIDIOC_REQBUFS");
    }

    buffers = calloc (4, sizeof (*buffers));

    if (!buffers) {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
        buffers[n_buffers].length = buffer_size;
        buffers[n_buffers].start = memalign (/*boundary*/ page_size, buffer_size);

        if (!buffers[n_buffers].start) {
            fprintf (stderr, "Out of memory\n");
            exit (EXIT_FAILURE);
        }
    }
}

static void init_device (void) {
    //open device
    struct stat st;

    if (-1 == stat (dev_name, &st)) {
        fprintf (stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno, strerror (errno));
        exit (EXIT_FAILURE);
    }

    //Is it the correct file type (character device)
    if (!S_ISCHR(st.st_mode)) {
        fprintf (stderr, "%s is no device\n", dev_name);
        exit (EXIT_FAILURE);
    }

    fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd) {
        fprintf (stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror (errno));
        exit (EXIT_FAILURE);
    }

    //init device
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf (stderr, "%s is no V4L2 device\n", dev_name);
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf (stderr, "%s is no video capture device\n", dev_name);
        exit (EXIT_FAILURE);
    }

    switch (io) {
        case IO_METHOD_READ:
            if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                fprintf (stderr, "%s does not support read i/o\n", dev_name);
                exit (EXIT_FAILURE);
            }
            break;
        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
            if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                fprintf (stderr, "%s does not support streaming i/o\n", dev_name);
                exit (EXIT_FAILURE);
            }
            break;
    }

    /* Select video input, video standard and tune here. */

    CLEAR (cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
                case EINVAL: break; /* Cropping not supported. */
                default: break; /* Errors ignored. */
            }
        }
    } else {
        /* Errors ignored. */
    }

    CLEAR (fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
        errno_exit ("VIDIOC_S_FMT");

    /* Note VIDIOC_S_FMT may change width and height. */

    /* Buggy driver paranoia. */
    min = fmt.fmt.pix.width * 2;
    if (fmt.fmt.pix.bytesperline < min)
        fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    if (fmt.fmt.pix.sizeimage < min)
        fmt.fmt.pix.sizeimage = min;

    switch (io) {
        case IO_METHOD_READ:
            init_read (fmt.fmt.pix.sizeimage);
            break;

        case IO_METHOD_MMAP:
            init_mmap ();
            break;

        case IO_METHOD_USERPTR:
            init_userp (fmt.fmt.pix.sizeimage);
            break;
    }
}

static void usage (FILE* fp, int argc, char** argv) {
    fprintf (fp,
            "Usage: %s [options]\n\n"
            "Options:\n"
            "-d | --device name   Video device name [/dev/video0]\n"
            "-h | --help   Print this message\n"
            "-m | --mmap   Use memory mapped buffers\n"
            "-r | --read   Use read() calls\n"
            "-u | --userp  Use application allocated buffers\n"
            "",
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

int main (int argc, char**  argv) {
    SDL_Init(SDL_INIT_EVERYTHING);
    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);

    dev_name = "/dev/video0";

    for (;;) {
        int index;
        int c;

        c = getopt_long (argc, argv, short_options, long_options, &index);

        if (-1 == c) break;

        switch (c) {
            case 0: break; /* getopt_long() flag */
            case 'd': dev_name = optarg; break;
            case 'h':
                      usage (stdout, argc, argv);
                      exit (EXIT_SUCCESS);
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
        }

    }

    init_device ();
    start_capturing ();
    mainloop ();
    stop_capturing ();
    uninit_device ();
    exit (EXIT_SUCCESS);
}
