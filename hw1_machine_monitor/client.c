#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
	//建立 socket
	int sockfd = 0;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		printf("Fail to create a socket.");
	}
	//連線socket
	struct sockaddr_in info;
	bzero(&info,sizeof(info));
	info.sin_family = PF_INET;
	info.sin_addr.s_addr = inet_addr("127.0.0.1");
	info.sin_port = htons(59487);

	int err = connect(sockfd,(struct sockaddr *)&info,sizeof(info));
	if(err==-1) {
		printf("Connection error");
	}

	char message;//[256]={};
	char receiveMessage[512] = {};
	char information[256]= {};
	char ready='i';
	int size;
	while(1) {
		//char* receiveMessage=malloc(sizeof(char)*512);
		//memset(receiveMessage,'\0',sizeof(receiveMessage));
//      recv(sockfd,&size,1,0);
		recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
		printf("%s",receiveMessage);

		scanf(" %c",&message);
		send(sockfd,&message,sizeof(message),0);

		if(message=='a') {
			int processid[1024];
			//        recv(sockfd,&size,1,0);
			recv(sockfd,processid,sizeof(processid),0);

			int i=0;
			printf("all process id:\n");
			while(processid[i]!=0) {
				printf("%d ",processid[i++]);
				if(i%10==0)printf("\n");
			}
			printf("\n");
		} else if(message=='g'||message=='e'||message=='d'||message=='c'||message=='i'
		          ||message=='j'||message=='f'||message=='b'||message=='h') {
			int pid;
			char stat[256]= {' ',};
			int child[128]= {0,};
			recv(sockfd,receiveMessage,sizeof(receiveMessage),0);
			printf("%s",receiveMessage);
			scanf("%d",&pid);
			send(sockfd,&pid,sizeof(pid),0);
			//recv(sockfd,stat,sizeof(stat),0);


			if(message=='g'||message=='e'||message=='i'||message=='j'||message=='d'
			   ||message=='f') {
				recv(sockfd,stat,sizeof(stat),0);
				printf("%s\n",stat);
			}
			/*else if(message=='d')//delete ()
			{
			    recv(sockfd,stat,sizeof(stat),0);
			    int i=0;//delete(
			    while(stat[++i]!=' ')
			    {
			    if(stat[i]!=')')printf("%c",stat[i]);
			    else break;

			    }
			    printf("\n");
			}*/
			else if(message=='c'||message=='b'||message=='h') {
				recv(sockfd,child,sizeof(child),0);
				int i =0;
				while(child[i]!=0) {
					printf("%d ",child[i++]);
					if(i%10==0)printf("\n");
				}
				printf("\n");
			}

		} else if(message='k') {
			close(sockfd);
			exit(0);
		}
		//memset(&ready,'\0',sizeof(ready));
		ready='i';
		send(sockfd,&ready,sizeof(ready),0);
	}
exit:
	printf("close Socket.\n");
	close(sockfd);

	return 0;
}

