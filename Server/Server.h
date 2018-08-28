#ifndef MY_SERVER_H
#define MY_SERVER_H


#include <iostream>  
#include<sys/socket.h>  
#include<sys/un.h>  
#include<netinet/in.h>  
#include <string.h>  
#include <sys/types.h>  
#include <unistd.h>  
#include <errno.h>  
#include <arpa/inet.h>  
#include <fcntl.h>  
#include <netdb.h>  
#include <stdio.h>  
#include <map>
#include <netinet/tcp.h> 
#include <pthread.h> 
#include <signal.h>

using namespace std;
#define MAX_COUNT	20
#define BUF_SIZE	100
#define PORT		8888
extern pthread_mutex_t mutex;

enum Type { HEART, OTHER };
typedef struct PACKET_HEAD
{
	Type type;
	int length;
}HEAD;

class CServSocket  
{  
public:  
    CServSocket();  
    virtual ~CServSocket();   
    bool Accept();  
    bool Listen(int port);  
    void Send(char* sendbuf, int len);  
    void Recv(); 
	void  getLocalAddr(char *ip);  
    std::string getCliAddr(int fd);  
    void Close();  
	void InitSelect();
	void ServerSelect();
    bool setNoDelay(bool nodelay); 
    bool setIntOptions(int option, int value); 
private:     
    int sockfd;  
    int connsockfd;  
    struct sockaddr_in servAddr, cliAddr; 
	int client[MAX_COUNT];
    fd_set server_set; 
	int max_fd ;
	map<int,int> mmap;
};



#endif
