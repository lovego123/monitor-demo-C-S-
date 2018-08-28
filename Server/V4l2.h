#ifndef V4L2_H
#define V4L2_H


#include <libv4l2.h>
#include <linux/videodev2.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <assert.h>
#include <sys/ioctl.h>


#define FILE_VIDEO 	"/dev/video0"
//#define FILE_PIC	"./test.jpg"
#define QBUF_NUM	4
#define BUFFER_SIZE	1000000
#define RESX 640
#define RESY 480

struct buffer
{
	void* start;
	size_t length;
};

class V4l2
{
	public:
		V4l2();
		virtual ~V4l2();
		bool CheckDev();
		bool InitDev();
		bool StartCapture();
		bool ReadStream(char *img_buf,int *len);
		bool CloseAll();
	private:
		int fd;
		struct  buffer* buffers;
		
};





#endif
