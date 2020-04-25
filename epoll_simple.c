#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#define SER_PORT 8000
#define MAXFD 1024

int main(int argc,char* argv[]){
	int lfd,cfd;
	struct sockaddr_in ser_addr,cli_addr;
	socklen_t cli_addrlen=sizeof(cli_addr);
	struct epoll_event event;
	struct epoll_event events[MAXFD];
	int epfd;//return val epoll
	char buf[1024];
	int i;
	
	ser_addr.sin_family =AF_INET;
	ser_addr.sin_port=htons(SER_PORT);
	ser_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	
	lfd=socket(AF_INET,SOCK_STREAM,0);
	
	int opt=1;
	setsockopt(lfd,SOL_SOCKET,SO_REUSEADDR,(void *)&opt,sizeof(opt));
	
	bind(lfd,(struct sockaddr*)&ser_addr,sizeof(ser_addr));
	listen(lfd,128);
	
	epfd=epoll_create(MAXFD);
	
	event.events=EPOLLIN;
	event.data.fd=lfd;
	
	int ecr=epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&event);
	if(ecr<0){
		printf("epoll_ctl error\n");
	}
	
	while(1){
		int ewr=epoll_wait(epfd,events,MAXFD,-1);
		for(i=0;i<ewr;i++){
			if(!(events[i].events&EPOLLIN)){
				continue;	
			}	
			else if(events[i].data.fd==lfd){
				cfd=accept(lfd,(struct sockaddr*)&cli_addr,&cli_addrlen);
			char dst[64];
				printf("client IP: %s  PORT: %d successful connection\n",
				inet_ntop(AF_INET,&cli_addr.sin_addr.s_addr,dst,64),
				ntohs(cli_addr.sin_port));
				
				event.events=EPOLLIN;
				event.data.fd=cfd;
				epoll_ctl(epfd,EPOLL_CTL_ADD,cfd,&event);
			}
			else{
				int rr=read(events[i].data.fd,buf,1024);	
				if(rr<0){
					printf("read error\n");
				}
				else if(rr==0){
					printf("client %d Disconnect!\n",events[i].data.fd);	
					close(events[i].data.fd);
					epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,NULL);	
				}
				else{
					write(STDOUT_FILENO,buf,rr);	
				}
			}
		}
	
	}
	
	return 0;	
}

