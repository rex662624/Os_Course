#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

int sockfd = 0;
int num =0;//現在有幾個client
int *forClientSockfd;//動態配置,存放每個client的descriper
pthread_t *thread;

void *thread_function(void *);
void getprocessid(int []);
void getstat(char[],int,char);
void getchild(int[], int,char );
void getcmdline(char[], int );
void getthread(int[], int);
void getancient(int [],int,int);

int main(int argc, char **argv)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		printf("Fail to create a socket.");
	}

	//socket的連線
	//提供socket訊息
	struct sockaddr_in serverInfo,clientInfo;
	int addrlen = sizeof(clientInfo);
	bzero(&serverInfo,sizeof(serverInfo));//初始化
	serverInfo.sin_family = PF_INET;//sockaddr_in為Ipv4結構
	serverInfo.sin_addr.s_addr =  inet_addr("127.0.0.1");//IP
	serverInfo.sin_port = htons(59487);//port

	//綁定server在socket上
	bind(sockfd,(struct sockaddr *)&serverInfo,sizeof(serverInfo));
	int threadid;
	while(1) {
		//監聽有沒有client來
		listen(sockfd,10000);
		//增加新的client,要把舊的data移到新allocate的地方
		int temp[num];
		pthread_t ptemp[num];
		int i;
		for(i=0; i<num; i++) {
			temp[i]=forClientSockfd[i];//先用temp存放剛剛的
			ptemp[i]=thread[i];
		}
		//重新allocate array(記得把num+1)
		forClientSockfd = malloc(sizeof(int)*(num+1));
		thread = malloc(sizeof(pthread_t)*(num+1));
		for(i=0; i<num; i++) {
			forClientSockfd[i]=temp[i];//還原剛剛備份的data
			thread[i]=ptemp[i];
		}

		forClientSockfd[num] = accept(sockfd,(struct sockaddr*)&clientInfo,&addrlen);
		int ptr = num;

		if(pthread_create( &(thread[num]), NULL, thread_function,(void*)&ptr)!=0) {
			printf("thread create failed");
		}
		threadid=thread[num];
		num++;
	}

	pthread_join(threadid,NULL);

	return 0;
}

void *thread_function(void *arg)
{
	char inputBuffer;//[256]={};
	char *infor=(char*)malloc(sizeof(char)*256);
	int localindex = *((int*)arg);
	char ready='i' ;
	char message[512]=
	    "==========================================\n(a)list all process ids\n(b)thread's IDs\n(c)child's PIDs\n(d)process name\n(e)state of process(D,R,S,T,t,W,X,Z)\n(f)command line of excuting process(cmdline)\n(g)parent's PID\n(h)all ancients of PIDs\n(i)virtual memory size(VmSize)\n(j)Physical memory size(VmRSS)\n(k)exit\n\0";
	int size;
	while(1) {
		printf("%d",sizeof(message));
		size = sizeof(message);
//        send(forClientSockfd[localindex],&size,1,0);
		send(forClientSockfd[localindex],message,sizeof(message),0);
		recv(forClientSockfd[localindex],&inputBuffer,sizeof(inputBuffer),0);
		printf("Get:thread %d :%c\n",localindex,inputBuffer);
		int processid[1024]= {0,};
		int getid;
		if(inputBuffer=='a')

		{
			getprocessid(processid);

			size = sizeof(processid);
			printf("%d",sizeof(processid));
//            send(forClientSockfd[localindex],&size,1,0);
			send(forClientSockfd[localindex],processid,sizeof(processid),0);
		}

		else if(inputBuffer=='g'||inputBuffer=='e'||inputBuffer=='d'||inputBuffer=='i'
		        ||inputBuffer=='j') {
			char stat[256]= {' ',};
			infor="pid ?";
			send(forClientSockfd[localindex],infor,sizeof(infor),0);
			recv(forClientSockfd[localindex],&getid,sizeof(getid),0);
			getstat(stat,getid,inputBuffer);
			send(forClientSockfd[localindex],stat,sizeof(stat),0);
//            printf("\nstat:%s\n",stat);
		} else if(inputBuffer=='c') {
			int child[128]= {0,};
			infor="pid ?";
			send(forClientSockfd[localindex],infor,sizeof(infor),0);
			recv(forClientSockfd[localindex],&getid,sizeof(getid),0);
			printf("%d",getid);
			getchild(child,getid,'c');
			send(forClientSockfd[localindex],child,sizeof(child),0);
		} else if(inputBuffer=='f') {
			char cmdline [256]= {' ',};
			infor="pid ?";
			send(forClientSockfd[localindex],infor,sizeof(infor),0);
			recv(forClientSockfd[localindex],&getid,sizeof(getid),0);
			getcmdline(cmdline,getid);
			send(forClientSockfd[localindex],cmdline,sizeof(cmdline),0);

		} else if(inputBuffer=='b') {
			int thread[128]= {0,};
			infor="pid ?";
			send(forClientSockfd[localindex],infor,sizeof(infor),0);
			recv(forClientSockfd[localindex],&getid,sizeof(getid),0);
			printf("%d\n",getid);
			getthread(thread,getid);
			printf("thread%d\n",thread[0]);
			send(forClientSockfd[localindex],thread,sizeof(thread),0);
		} else if(inputBuffer=='h') {
			int ancient[128]= {0,};
			infor="pid ?";
			send(forClientSockfd[localindex],infor,sizeof(infor),0);
			recv(forClientSockfd[localindex],&getid,sizeof(getid),0);
			printf("%d\n",getid);
			getancient(ancient,getid,0);
			send(forClientSockfd[localindex],ancient,sizeof(ancient),0);
		} else if(inputBuffer=='k')
			break;
		recv(forClientSockfd[localindex],&ready,sizeof(ready),0);
	}
	close(forClientSockfd[localindex]);
}

void getprocessid(int store[])
{
	DIR *d;
	struct dirent *dir;
	d = opendir("/proc");
	int index=0;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			if(dir->d_name[0]=='1'||dir->d_name[0]=='2'||dir->d_name[0]=='3'
			   ||dir->d_name[0]=='4'||dir->d_name[0]=='5'||dir->d_name[0]=='6'
			   ||dir->d_name[0]=='7'||dir->d_name[0]=='8'||dir->d_name[0]=='9')
				store[index++]=atoi(dir->d_name);

		}
		closedir(d);
	}
	return;

}
void getcmdline(char store[], int pid)
{

	char filename[1000];
	char garbage[256];
	sprintf(filename,"/proc/%d/cmdline", pid);
	FILE *f = fopen(filename, "r");
	fgets(&garbage,sizeof(garbage),f);
	sprintf(store,"%s",garbage);

	fclose(f);

}

void getstat(char store[], int pid,char mode)
{

	printf("pid = %d\n", pid);
	char filename[1000];


	sprintf(filename,"/proc/%d/status", pid);
	//else
	//    sprintf(filename,"/proc/%d/stat", pid);//

	FILE *f = fopen(filename, "r");
	//int unused;
	//char comm[1000];

	//char state;
	//int ppid;
	char garbage[256];
	int count;
	if(mode=='d')count=1;
	else if(mode=='e')count=2;
	else if(mode=='g')count=6;
	else if(mode=='i')count=17;
	else if(mode=='j')count=21;
	//if(mode=='i')
	//{
	int i=0;
	for(i=0; i<count; i++) {
		fgets(&garbage,sizeof(garbage),f);
	}
	sprintf(store,"%s",garbage);
	//}
	/*else
	{
	    fscanf(f, "%d %s %c %d", &unused,comm, &state, &ppid);//processid
	    printf("comm = %s\n", comm);
	    printf("state = %c\n", state);
	}

	if(mode=='g')
	    sprintf(store,"%d",ppid);
	else if(mode=='e')
	    sprintf(store,"%c",state);
	else if(mode=='d')
	    sprintf(store,"%s",comm);
	else if(mode=='i')
	    sprintf(store,"%s",garbage);
	else if(mode=='j')
	{
	    sprintf(store,"%s",garbage);
	}

	printf("parent pid = %d String= %s\n",ppid,store);
	*/
	fclose(f);


}

void getchild(int store[], int pid,char mode)
{
	printf("pid = %d\n", pid);
	char filename[1000];
	sprintf(filename,"/proc/%d/task/%d/children",pid,pid);
	FILE *f = fopen(filename, "r");
	char child[10];
	int i=0;

	while(fscanf(f,"%d ",&store[i])==1) {
		i++;
		printf("%d ",i);
	}



}
void getthread(int store[],int pid)
{
	DIR *d;
	struct dirent *dir;
	char filename[100];
	sprintf(filename,"/proc/%d/task", pid);
	d = opendir(filename);
	int index=0;
	int count=0;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			store[index++]=atoi(dir->d_name);
			count++;//前兩個數都是0（測試結果）
			if(count<=2)index=0;
		}
		closedir(d);
	}
	//printf("t:%d\n",store[0]);
	return;


}
void getancient(int store[],int pid,int index)
{
	char unused[100];
	int ppid;
	char filename[1000];
	sprintf(filename,"/proc/%d/stat", pid);
	FILE *f = fopen(filename, "r");
	fscanf(f, "%s %s %s %d",unused,unused,unused, &ppid);//第四個才是parent id
	if(ppid==0)
		return;
	else {
		store[index]=ppid;
		getancient(store,ppid,index+1);
	}

}
