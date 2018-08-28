#include "V4l2.h"
#include "Server.h"


void* GrabThread(void*);
V4l2 myV4l2;
CServSocket servSocket; 


int main()
{
	signal(SIGPIPE,SIG_IGN);  //忽略SIGPIPE，防止异常中断进程
	myV4l2.CheckDev();
	myV4l2.InitDev();
	myV4l2.StartCapture();
	pthread_t ptr;
	if(pthread_create(&ptr,NULL,GrabThread,NULL) != 0)//创建读取视频帧线程
	{
		std::cout<<"create video thread failed!\n";
		exit(1);
	}

    if(!servSocket.Listen(PORT))//启动socket并监听端口
	{
		std::cout<<"socket start failed!\n";
		exit(1);
	}
	std::cout<<"dev init complete, waiting for connect...\n";
	
	servSocket.ServerSelect(); //执行select
	
	return 0;
}
void* GrabThread(void* arg)
{
	int length;
        char img_buf[BUFFER_SIZE];
	while(1)
	{
		length = 0;
		bzero(img_buf, BUFFER_SIZE);  
		myV4l2.ReadStream(img_buf,&length); 

		servSocket.Send((char*)&length,sizeof(int));
		servSocket.Send(img_buf,sizeof(char)*length);
	} 
	return NULL;
}
