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

#include "videoInterface.h"

static char* video_node = NULL;
static io_method io = IO_METHOD_MMAP;
static int fd = -1;
struct buffer* buffers = NULL;
static unsigned int n_buffers = 0;

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
    if (-1 == close (fd)) errno_exit ("close");

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
            fprintf (stderr, "%s does not support memory mapping\n", video_node);
            exit (EXIT_FAILURE);
        } else errno_exit ("VIDIOC_REQBUFS");
    }

    if (req.count < 2) {
        fprintf (stderr, "Insufficient buffer memory on %s\n", video_node);
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
            fprintf (stderr, "%s does not support user pointer i/o\n", video_node);
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

    if (-1 == stat (video_node, &st)) {
        fprintf (stderr, "Cannot identify '%s': %d, %s\n", video_node, errno, strerror (errno));
        exit (EXIT_FAILURE);
    }

    //Is it the correct file type (character device)
    if (!S_ISCHR(st.st_mode)) {
        fprintf (stderr, "%s is no device\n", video_node);
        exit (EXIT_FAILURE);
    }

    fd = open (video_node, O_RDWR /* required */ | O_NONBLOCK, 0);

    if (-1 == fd) {
        fprintf (stderr, "Cannot open '%s': %d, %s\n", video_node, errno, strerror (errno));
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
            fprintf (stderr, "%s is no V4L2 device\n", video_node);
            exit (EXIT_FAILURE);
        } else {
            errno_exit ("VIDIOC_QUERYCAP");
        }
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf (stderr, "%s is no video capture device\n", video_node);
        exit (EXIT_FAILURE);
    }

    switch (io) {
    case IO_METHOD_READ:
        if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
            fprintf (stderr, "%s does not support read i/o\n", video_node);
            exit (EXIT_FAILURE);
        }
        break;
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            fprintf (stderr, "%s does not support streaming i/o\n", video_node);
            exit (EXIT_FAILURE);
        }
        break;
    }

    /* Select video input, video standard and tune here. */

    CLEAR (cropcap);

    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; // reset to default
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

// SDL format: 32 bits: 8 red, 8 green, 8 blue, 8 alpha
// 0xrrggbbaa

//                p1    p2      pixel 1 and 2
// YUYV format: [Y1,U][Y2,V]
// YCbCr is the same as YUV

// converts a pixel from YUV color to RGB color
void pixel_YUV2RGB(int y, int u, int v, char *r, char *g, char *b) {
    int c = y - 16;
	int d = u - 128;
	int e = v - 128;

    int r1 = (298 * c           + 409 * e + 128) >> 8;
    int g1 = (298 * c - 100 * d - 208 * e + 128) >> 8;
    int b1 = (298 * c + 516 * d           + 128) >> 8;

    // Even with proper conversion, some values still need clipping.
    *r = r1 > 255 ? 255 : r1 < 0 ? 0 : r1;
    *g = g1 > 255 ? 255 : g1 < 0 ? 0 : g1;
    *b = b1 > 255 ? 255 : b1 < 0 ? 0 : b1;
}

// converts an array of pixels from YUYV color to RGB color
void array_YUYV2RGB(void *Dest, const void *src, int width, int height) {
    unsigned char *dest = (unsigned char *)Dest;
    for(int y = 0; y < height; y++) {
        unsigned char *line = (unsigned char *)src + y * width * 2;
        for(int x = 0; x < width; x += 2) {
            int Y0 = line[2 * x + 0];
            int U  = line[2 * x + 1];
            int Y1 = line[2 * x + 2];
            int V  = line[2 * x + 3];

            for (int j = 0; j < 2; j++) {
                char r, g, b;
                pixel_YUV2RGB(Y0, U, V, &r, &g, &b);
                int l = (y * width + x + j) * 4;
                dest[l + 0] = b;
                dest[l + 1] = g;
                dest[l + 2] = r;
                dest[l + 3] = 255;
            }
        }
    }
}

void read_frame (void *frame_dest) {
    struct v4l2_buffer buf;
    unsigned int i;

    switch (io) {
    case IO_METHOD_READ:
        if (-1 == read (fd, buffers[0].start, buffers[0].length)) {
            switch (errno) {
                case EAGAIN: return;
                case EIO: /* Could ignore EIO, see spec. fall through */
                default: errno_exit ("read");
            }
        }

        array_YUYV2RGB(frame_dest, buffers[buf.index].start, 640, 480);

        break;
    case IO_METHOD_MMAP:
        CLEAR (buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
                case EAGAIN: return;
                case EIO: /* Could ignore EIO, see spec. fall through */
                default: errno_exit ("VIDIOC_DQBUF");
            }
        }

        assert (buf.index < n_buffers);

        array_YUYV2RGB(frame_dest, buffers[buf.index].start, 640, 480);

        if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
            errno_exit ("VIDIOC_QBUF");
        break;
    case IO_METHOD_USERPTR:
        CLEAR (buf);

        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
                case EAGAIN: return;
                case EIO: /* Could ignore EIO, see spec. fall through */
                default: errno_exit ("VIDIOC_DQBUF");
            }
        }

        for (i = 0; i < n_buffers; ++i)
            if (buf.m.userptr == (unsigned long)buffers[i].start
                    && buf.length == buffers[i].length) break;

        assert(i < n_buffers);

        array_YUYV2RGB(frame_dest, buffers[buf.index].start, 640, 480);

        if (xioctl(fd, VIDIOC_QBUF, &buf) == -1) errno_exit ("VIDIOC_QBUF");

        break;
    }

    return;
}

void startVideo(const char *dev_name, io_method i) {
    if(!dev_name){
        errno_exit("NULL dev_name");
    }

    io = i;

    //Copy the string so it doesn't get freed on us unintentionally
    video_node = strdup(dev_name);
    init_device();
    start_capturing();
}

void stopVideo() {
    free(video_node);

    uninit_device();
    stop_capturing();
}

