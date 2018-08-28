#include "V4l2.h"

V4l2::V4l2():fd(0),buffers(NULL){}

V4l2::~V4l2()
{
	if(buffers != NULL)
	{
		free(buffers);
	}
	CloseAll();
}

bool V4l2::CheckDev()
{
	fd = open(FILE_VIDEO,O_RDWR); //打开摄像头设备
	if (!fd) {
		perror("Error opening device");
		exit (EXIT_FAILURE);
	}

	struct v4l2_capability cap;
	if (-1 == ioctl (fd, VIDIOC_QUERYCAP, &cap)) { //获取设备的信息
		if (EINVAL == errno) {
			fprintf (stderr, "Device is no V4L2 device\n");
			exit (EXIT_FAILURE);
		} else {
			perror ("VIDIOC_QUERYCAP");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		printf("driver:\t\t%s\n",cap.driver);
     	printf("card:\t\t%s\n",cap.card);
     	printf("bus_info:\t%s\n",cap.bus_info);
     	printf("version:\t%d\n",cap.version);
     	printf("capabilities:\t%x\n",cap.capabilities);
	}
	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) { //判断是否支持视频捕获
		fprintf (stderr, "Device is no video capture device\n");
		exit (EXIT_FAILURE);
	}
	else
	{
		printf("[%s] supports: capture.\n",FILE_VIDEO);
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING)) { //判断是否支持I/O流
		fprintf (stderr, "Device does not support streaming i/o\n");
		exit (EXIT_FAILURE);
	}
	else
	{
		printf("[%s] supports: streaming.\n",FILE_VIDEO);
	}

	printf("[%s] supports: \n",FILE_VIDEO);
	struct v4l2_fmtdesc formats;
	memset (&formats, 0, sizeof (formats));
	formats.index = 0;
	formats.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	while (0 == ioctl (fd, VIDIOC_ENUM_FMT, &formats)) { //列出设备支持的格式信息
		printf ("\t%d.%s\n", formats.index+1,formats.description);
		formats.index++;
	}
	if (errno != EINVAL || formats.index == 0) {
		perror ("VIDIOC_ENUMFMT");
		exit(EXIT_FAILURE);
	}
	return true;
}

bool V4l2::InitDev()
{
	//设置格式
	struct v4l2_format fmt;
	memset(&fmt, 0, sizeof(fmt));	
	fmt.fmt.pix.width       = RESX;
	fmt.fmt.pix.height      = RESY;
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
	fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

	if (-1 == ioctl (fd, VIDIOC_S_FMT, &fmt)) { 
		perror ("VIDIOC_S_FMT");
		exit(EXIT_FAILURE);
	}

	//申请缓冲区
	struct v4l2_requestbuffers req;
	memset(&req, 0, sizeof(req));
	req.count               = QBUF_NUM;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == ioctl (fd, VIDIOC_REQBUFS, &req)) {
		if (EINVAL == errno) {
			fprintf (stderr, "Device does not support memory mapping\n");
			exit (EXIT_FAILURE);
		} else {
			perror ("VIDIOC_REQBUFS");
			exit(EXIT_FAILURE);
		}
	}

	if (req.count < QBUF_NUM) {
		fprintf (stderr, "Insufficient buffer memory on device\n");
		exit (EXIT_FAILURE);
	}

	buffers = (struct buffer*)calloc (req.count, sizeof (*buffers));
	if (!buffers) {
		fprintf (stderr, "Out of memory\n");
		exit (EXIT_FAILURE);
	}
	// 为每一个缓冲区进行内存映射
	for (int n_buffers = 0; n_buffers < req.count; ++n_buffers) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));

		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = n_buffers;
		//获取内部信息到buf
		if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf)) {
			perror ("VIDIOC_QUERYBUF");
			exit(EXIT_FAILURE);
		}
		//开始映射
		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start =
			mmap (NULL /* start anywhere 让系统选定*/,
			      buf.length, //映射的大小
			      PROT_READ | PROT_WRITE /* required 读写*/,
			      MAP_SHARED /* recommended 允许共享*/,
			      fd,  //文件描述符
				  buf.m.offset);//偏移量

		if (MAP_FAILED == buffers[n_buffers].start) {
			perror ("mmap");
			exit(EXIT_FAILURE);
		}
		//进缓冲队列
		if (-1 == ioctl (fd, VIDIOC_QBUF, &buf)) {
			perror ("VIDIOC_QBUF");
			exit(EXIT_FAILURE);
		}
	}
	return true;
}

bool V4l2::StartCapture()
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	// 开启流
	if (-1 == ioctl (fd, VIDIOC_STREAMON, &type)) {
		perror ("VIDIOC_STREAMON");
		exit(EXIT_FAILURE);
	}
	return true;
}

bool V4l2::ReadStream(char *img_buf,int *len)
{
	struct v4l2_buffer buf;
	memset(&buf, 0, sizeof(buf));
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = 0;
	// 出缓冲队列
	if (-1 == ioctl (fd, VIDIOC_DQBUF, &buf)) {
		perror ("VIDIOC_DQBUF");
		exit(EXIT_FAILURE);
	}
	assert (buf.index < QBUF_NUM);
	*len = buffers[buf.index].length;
	memcpy(img_buf,buffers[buf.index].start,buffers[buf.index].length);
	
/*	FILE *fp = fopen(FILE_PIC,"wb");
	if(fp != NULL)
	{
	   fwrite(buffers[buf.index].start, 1,buffers[buf.index].length, fp);
		fflush(fp);
		fclose(fp);
	}
*/
	//printf ("TRANS : %d \n", buf.bytesused);
	//重新进队，构成循环队列
	if (-1 == ioctl (fd, VIDIOC_QBUF, &buf)) {
		perror ("VIDIOC_QBUF");
		exit(EXIT_FAILURE);
	}
	return true;
}

bool V4l2::CloseAll()
{
	enum v4l2_buf_type off = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == ioctl(fd,VIDIOC_STREAMOFF, &off))
	{
		perror("Stop Camera fail");
		exit (EXIT_FAILURE);
	}
	for(int i=0;i<QBUF_NUM;i++)
	{
		munmap(buffers[i].start , buffers[i].length);
	}
	close(fd);
	return true;
}
