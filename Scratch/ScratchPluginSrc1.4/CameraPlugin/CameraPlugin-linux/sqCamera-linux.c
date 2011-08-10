/*
 *  V4L2 for Scratch (Derek O'Connell, 2009)
 *
 *  This code can be used and distributed without restrictions.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>   /* getopt_long() */

#include <fcntl.h>    /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>	  /* for videodev2.h */

#include <linux/videodev2.h>

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef enum {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} io_method;

struct buffer {
	void *  start;
	size_t  length;
};



int CameraGetParam(int cameraNum, int paramNum);
int CameraGetFrame(int cameraNum, unsigned char* buf, int pixelCount);
int CameraExtent(int cameraNum);
char* CameraName(int cameraNum);
void CameraClose(int cameraNum);
int CameraOpen(int cameraNum, int frameWidth, int frameHeight);


static char * dev_name		= NULL;
static io_method io		= IO_METHOD_MMAP;
static int fd			= -1;
struct buffer *	buffers 	= NULL;
static unsigned int n_buffers 	= 0;

struct v4l2_buffer read_buf;

static void *sqBuffer 		= NULL;
static long sqBufferBytes 	= 0;
static long sqPixels 		= 0;

static int bmWidth, bmHeight; /* prob temp */

static int fc = 0;


static int xioctl (int fd, int request, void * arg) {
	int r;

	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}


/* RGB24-ARGB32 Conversion */
static void process_image (const void *p) {
	int i;
	unsigned char *s;
	unsigned long int *d;
	unsigned long int pixel;

	if (0 == sqBuffer)
		return;

	s = (unsigned char *)p;
	d = (unsigned long int *)sqBuffer;

	for ( i = 0; i < sqPixels; i++) {
		pixel = *s++ << 16;
		pixel = pixel | (*s++ << 8);
		*d++  = pixel | *s++;
	}
}

static int read_frame (void) {
	struct v4l2_buffer buf;

	fc += 1;

	CLEAR (buf);

	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
		switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				return 0;

			default:
				return -1;

		}
	}

	if (buf.index < n_buffers)
		process_image (buffers[buf.index].start);

	if (-1 == xioctl (fd, VIDIOC_QBUF, &buf)) {
		return -1;
	}

	return 0;
}

static int getFrame() {
	unsigned int retry;
	fd_set fds;
	struct timeval tv;
	int r;

	retry = 1;

	while (retry-- > 0) {
		FD_ZERO (&fds);
		FD_SET (fd, &fds);

		/* Timeout. */
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		r = select (fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
			if (EINTR == errno)
				continue;

			return -1;
		}

		if (0 == r) {
			return -1;
		}

		if (0 == read_frame ())
			return 0;

		/* EAGAIN - retry */
	}
}



static int stream_off (void) {
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	/* Throws "Device or resource busy" (errno 16) */
	/* Prob should be looped but no apparent ill effects */
	xioctl (fd, VIDIOC_STREAMON, &type);

	return 0;
}

static int stream_on (void) {
	enum v4l2_buf_type type;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl (fd, VIDIOC_STREAMON, &type)) {
		return -1;
	}

	return 0;
}


static int uninit_device (void) {
	unsigned int i;

	for (i = 0; i < n_buffers; ++i)
		if (-1 == munmap (buffers[i].start, buffers[i].length))
			return -1;

	free (buffers);
	return 0;
}


static int queue_buffers (void) {
	unsigned int i;

	for (i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;

		if (-1 == xioctl (fd, VIDIOC_QBUF, &buf)) {
			return -1;
		}
	}

	return 0;
}

static int init_mmap (void) {
	struct v4l2_requestbuffers req;

	CLEAR (req);

	req.count     = 4;
	req.type      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory    = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			return -1;
		} else {
			return -1;
		}
	}

	if (req.count < 2) {
		return -1;
	}

	buffers = calloc (req.count, sizeof (*buffers));

	if (!buffers) {
		return -1;
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;

		CLEAR (buf);

		buf.type	= V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;

		if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf)) {
			return -1;
		}

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start  = mmap (NULL /* start anywhere */,
						  buf.length,
						  PROT_READ | PROT_WRITE /* required */,
						  MAP_SHARED /* recommended */,
						  fd, buf.m.offset);

		if (MAP_FAILED == buffers[n_buffers].start) {
			return -1;
		}
	}

	return 0;
}

static int init_device (int w, int h) {
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	unsigned int min;

	if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)) {
		if (EINVAL == errno) {
			return -1;
		} else {
			return -1;
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		return -1;
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
		return -1;
	}


	/* Select video input, video standard and tune here. */

	CLEAR (cropcap);

	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) {
			if (EINVAL == errno) {
				/* Cropping not supported (ignored) */
			} else {
				/* Errors ignored. */
			}
		}
	} else {
		/* Errors ignored. */
	}

/*
	fmt.fmt.pix.field	= V4L2_FIELD_INTERLACED;
	fmt.fmt.pix.field	= V4L2_FIELD_TOP;
*/

	CLEAR (fmt);

	fmt.type		= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width	= w;
	fmt.fmt.pix.height	= h;
	fmt.fmt.pix.pixelformat	= V4L2_PIX_FMT_RGB24;
	fmt.fmt.pix.field	= V4L2_FIELD_INTERLACED;

	if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt)) {
		return -1;
	}

	/* Note VIDIOC_S_FMT may change width and height. */

	if ((w != fmt.fmt.pix.width) | (h != fmt.fmt.pix.height) | (V4L2_PIX_FMT_RGB24 != fmt.fmt.pix.pixelformat)) {
		return -1;
	}

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 3;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	if (0 > init_mmap())
		return -1;

	if (0 > queue_buffers())
		return -1;

	/* cache returned dims (make fmt a global?) */
	bmWidth = fmt.fmt.pix.width;
	bmHeight = fmt.fmt.pix.height;

	/* How many bytes to transfer? */
	/* NB: Scratch dictates equal or *larger* sized frame than requested */
	/* >>> For initial test assuming got frame size requested and "min" above is correct */
	sqBufferBytes = min;
	sqPixels = bmWidth * bmHeight;

	return 0;
}

static int close_device (void) {
	if (-1 == close (fd))
		return -1;

	fd = -1;
	return 0;
}

static int open_device (void) {
	struct stat st;

	if (-1 == stat (dev_name, &st)) {
		return -1;
	}

	if (!S_ISCHR (st.st_mode)) {
		return -1;
	}

	fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

	if (-1 == fd) {
		return -1;
	}

	return 0;
}




/* ============================================= SCRATCH I/F ==================================================== */


int CameraGetParam(int cameraNum, int paramNum) {
	return 0;
}

int CameraGetFrame(int cameraNum, unsigned char* buf, int pixelCount) {
	if (-1 == fd)
		return -1;

	sqBuffer = (void *)buf;

	/* ToDo: Should fail on wrong sized frame and return frames since last req */

	if (0 > getFrame())
		return 0;

	return 1;
}

int CameraExtent(int cameraNum) {
	if (-1 == fd)
		return 0;
	return (bmWidth << 16) + bmHeight;
}

char* CameraName(int cameraNum) {
	if (-1 == fd)
		return "camera not open";
	return "default camera";
}

void CameraClose(int cameraNum) {
	if (-1 == fd)
		return;
	stream_off();
	uninit_device();
	close_device();
}

int InitCamera(int w, int h) {
	read_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	read_buf.memory = V4L2_MEMORY_MMAP;

	dev_name = "/dev/video";
	io = IO_METHOD_MMAP; /* leave in case range of operation expanded */

	if (0 > open_device())
		return -1;

	if (0 > init_device(w,h)) {
		close_device();
		return -1;
	}

	if (0 > stream_on()) {
		uninit_device();
		close_device();
		return -1;
	}

	return 0;
}

int CameraOpen(int cameraNum, int frameWidth, int frameHeight) {
	if (-1 == fd)
		if (0 > InitCamera(frameWidth, frameHeight))
			return 0;
	return 1;
}
