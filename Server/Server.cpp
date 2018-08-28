#include "Server.h"


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  

CServSocket::CServSocket():max_fd(-1),sockfd(-1),connsockfd(-1)  
{  
	for(int i=0;i<MAX_COUNT;++i)
	{
		client[i] = -1;
	}
}  
  
CServSocket::~CServSocket()  
{  
    Close();
}  
  
void CServSocket::Close()  
{  
	for(int i=0;i<MAX_COUNT;++i)
	{
		if(client[i] >=0)
		{
			::close(client[i]);
		}	
	}
    if(sockfd) 
	{
        ::close(sockfd);  
	}
}  

bool CServSocket::Listen(int port)  //启动并开始监听
{  
    if(port < 0)  
    {  
        std::cout <<"the port err\n";  
        return false;  
    }  
    sockfd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  //启动socket
    if(sockfd < 0)  
    {  
        std::cout << "create socket err\n";  
        return false;  
    }  
    char ip[20]; 
    memset(ip,0,20);
    getLocalAddr(ip);  //获得本地ip地址  
    if(ip)
	{
        servAddr.sin_addr.s_addr = inet_addr(ip);  
	}
    else  
	{
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);  
	}
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);  

   //setIntOptions(SO_KEEPALIVE, 1);  
	//setNoDelay(true); 
	setIntOptions(SO_REUSEADDR,1);  //打开地址复用
    if(::bind(sockfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)  //绑定socket
    {  
        std::cout << "bind err\n";  
        return false;  
    }  
    if(::listen(sockfd, 0) < 0)  //监听
    {  
        std::cout << "listen err\n";  
        return false;         
    }  
    return true;  
} 


void CServSocket::ServerSelect() //执行select阻塞判断
{
	int ret = 0;
	while(1)  
	{
		InitSelect();
		//检测描述符状态
		ret = select(max_fd+1,&server_set,NULL,NULL,NULL);
		if(ret < 0)
		{
			std::cout <<"select error\n";
			break;
		}
		else if(ret == 0)
		{
			std::cout<<"select out_time\n";
			continue;
		}
		else
		{
			if(!Accept())  
			{  
				std::cout << "accept error\n";  
				continue;  
			}    
			Recv();  
		}
	}  
}
void CServSocket::InitSelect()//初始化select
{
	FD_ZERO(&server_set);
	FD_SET(sockfd,&server_set);
	max_fd = max_fd > sockfd ? max_fd : sockfd;
	for(int i=0 ; i<MAX_COUNT ; ++i)
	{
		if(client[i]>=0)
		{
			FD_SET(client[i],&server_set);
			max_fd = max_fd > client[i] ? max_fd : client[i];
		}
	}
}
 bool CServSocket::Accept()  //接受来自Client的请求
{  
		if(FD_ISSET(sockfd,&server_set))
		{
			socklen_t len = sizeof(cliAddr);
			if((connsockfd = ::accept(sockfd, (struct sockaddr*)&cliAddr, &len)) <0)  
			{  
				std::cout<<"accept error\n";
				return false;  
			}  
			int i;
			for(i=0 ; i<MAX_COUNT ; ++i)
			{
				if(client[i] < 0)
				{
				pthread_mutex_lock(&mutex);
					client[i] = connsockfd;
					FD_SET(connsockfd,&server_set);
				pthread_mutex_unlock(&mutex);
					mmap.insert(make_pair(connsockfd,0));
					max_fd = max_fd > connsockfd ? max_fd : connsockfd;
					break;
				}
			}
			if(i == MAX_COUNT)
			{
				std::cout<<"the number of connections is full\n";
				return false;
			}
			else
			{
				printf("accpet a new client: %s[%d]\n", inet_ntoa(cliAddr.sin_addr) , cliAddr.sin_port);
			}
		}
	
    return true;  
}  
  
bool CServSocket::setIntOptions(int option, int value)  //socket相关信息设定
{  
    bool res = false;  
    if(sockfd)  
    {  
        res = (setsockopt(sockfd, SOL_SOCKET, option, (const void*)&value, sizeof(value)) == 0);  
    }  
    return res;  
} 

bool CServSocket::setNoDelay(bool nodelay)  //设置发送包是否支持延迟
{  
    bool res = false;  
    if(sockfd)  
    {  
        int ndelay = nodelay?1:0;  
        res = (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,(const void*)&ndelay, sizeof(ndelay)) == 0);  
        res =true;        
    }  
    return res;  
}

void  CServSocket::getLocalAddr(char *ip)  //获得本地的ip地址
{    
    char ipaddr[20]={'\0'};  
    const char* shellstr = "ifconfig | sed -n '2p' | awk -F'[ :]+' '{printf $4}'";    
    FILE *fp = popen(shellstr, "r");  
    fread(ipaddr, sizeof(char), sizeof(ipaddr), fp);  
    if(ipaddr)  
    {  
        strcpy(ip, ipaddr);  
    }
    pclose(fp);  
    
} 


std::string CServSocket::getCliAddr(int fd)  //获得客户机的ip地址和端口号
{  
    char cliip[20];  
    socklen_t size = sizeof(cliAddr);  
    if(getpeername(fd, (sockaddr*)&cliAddr, &size))  
    {  
        strcpy(cliip, "0.0.0.0");  
    }  
    else  
    {  
        sprintf(cliip, "%s[%d]",inet_ntoa(cliAddr.sin_addr),cliAddr.sin_port); 
    }  
    return std::string(cliip);  
} 

void CServSocket::Send(char *sendbuf, int len)  //发送消息
{  
    if(sendbuf==NULL || len < 0) 
	{
		std::cout<<"read stream error\n";
		return;
	}
	for(int i=0;i<MAX_COUNT;++i)
	{
		if(client[i] < 0)
		{
			continue;
		}
		int dataleft = len, total = 0, ret =0;  
		while(total < len)
		{
			pthread_mutex_lock(&mutex);
			ret = ::send(client[i], sendbuf+total, dataleft, 0);
			pthread_mutex_unlock(&mutex);
			if(ret < 0)
			{
				return;
			}
			total += ret;  
			dataleft = len-total; 
		}
		if(total != len)
		{
			printf("client(%d) send error\n",i);
		}
	}
}

void CServSocket::Recv()  //接收来自客户端的消息
{  
	char recvbuf[BUF_SIZE]; 
	int recvlen;
	for(int i=0;i<MAX_COUNT;++i)
	{
		if(client[i] < 0)
		{
			continue;
		}
		if(FD_ISSET(client[i],&server_set))
		{
			HEAD head;
			memset(recvbuf, 0, sizeof(recvbuf));
			recvlen = ::recv(client[i], recvbuf, sizeof(head), 0);
			if(recvlen > 0)
			{
				memset(&head,0,sizeof(head));
				memcpy(&head,recvbuf,sizeof(head));
				if(head.type == HEART)
				{
					mmap.find(client[i])->second = 0;
					 continue;
				}
				else
				{
					memset(recvbuf, 0, sizeof(recvbuf));
					recvlen = ::recv(client[i], recvbuf, head.length, 0);
					if(recvlen != head.length)
					{
						printf("recv error\n");
					}
					else
					{
						printf("%s:%s\n",getCliAddr(client[i]).c_str(),recvbuf);
					}
					continue;
				}
			}

		}
		else
		{
			mmap.find(client[i])->second +=1;
			if(mmap.find(client[i])->second < 5)
				continue;
		}
		pthread_mutex_lock(&mutex);
		::close(client[i]);
		FD_CLR(client[i],&server_set);
		mmap.erase(mmap.find(client[i]));
		client[i] = -1;
		printf("client(%d) exit!\n",i);
		pthread_mutex_unlock(&mutex);
	}
} 

