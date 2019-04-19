#include "master.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
int globalVariable = 2;
int index=0;
int getname(char store[100][100],char*path);
void substr(char *dest, const char* src, unsigned int start,
            unsigned int cnt);//find substring

int main(int argc, char **argv)
{
	int slavenumber=1;
	char * sIdentifier=malloc(100);
	int    iStackVariable = 20;
	int sysfs_fd = open("/sys/kernel/hw2/mailbox",O_RDWR);
	if(argc>=7) { //optional
		slavenumber = atoi(argv[6]);
	}
	char * dir=malloc(100);
	char * keyword=malloc(100);
	dir =  argv[4];
	keyword = argv[2];
//	printf("infor: %s %s %d\n",keyword,dir,slavenumber);
//    pid_t pID = fork();
	pid_t child_pid,waitid;
	int i;
	for(i = 0; i<slavenumber; i++) {
		if ((child_pid= fork())== 0) { // child
			// Code only executed by child process
			sleep(1);
			execl("/home/user/hw2_mailbox/slave","/home/user/hw2_mailbox/slave",NULL);
			sIdentifier = "Child Process: ";
			printf("childpid:%d  ",child_pid);
			globalVariable++;
			iStackVariable++;
			printf("%s  ",sIdentifier);
			printf("Global variable %d  ",globalVariable);
			printf("Stack variable %d  ",iStackVariable);
			while(1);
			exit(0);
		} else if (child_pid < 0) { // failed to fork
			printf("Failed to fork\n");
			exit(1);
			// Throw exception
		}

	}


	// Code only executed by parent process

	errno = 0;
	char *buf = (char*)malloc(100);
	sprintf(buf,"%s",dir);
	//buf = "/home/user/hw2_mailbox/text/";//store file directionary
	printf("%s\n",buf);
	//getfilename
	char file[100][100];
	int filecount=getname(file,buf);//filecount = how many files
	int n = 0;
	printf("filecount:%d\n",filecount);
	/*
	    for(n=0; n<filecount; n++) {
	        printf("file: %s \n",file[n]);
	    }
	*/
//start send and receive
	//store in mailbox_t
	int boxcount=0;//always < slave
	int receivecount=0;
	int nowsend=0;
	struct mail_t *mail = malloc(sizeof(struct mail_t)) ;
	while(receivecount<filecount) {
		for(n=nowsend; n<filecount; n++) {
			if(boxcount<slavenumber) {
				sprintf(mail->data.query_word,keyword);
				sprintf(mail->file_path,file[n]);
				int a = send_to_fd(sysfs_fd,mail);
				if(a>=0) {

					boxcount++;
					nowsend++;
//            printf("%s %s n:%d\n",mail->file_path,mail->data.query_word,n);
				}
			} else {
				break;
			}
			//printf("master write %d\n",a);
		}
		int b = receive_from_fd( sysfs_fd,mail);

		if(b>0) {
			boxcount--;
			receivecount++;
			printf("total number “%s” in %s :%d\n",keyword,mail->file_path,
			       mail->data.word_count);

		} else {
			int i,j;
			for( i =0; i<32767; i++)for( j=0; j<10; j++);
		}
	}

	//killchild

	//    sleep(2);
	int  killReturn = killpg(getpgid(child_pid),
	                         SIGKILL);  // Kill child process group
	if(killReturn == -1) {
		if( errno == ESRCH) {    // pid does not exist
			printf("Group does not exist!\n");
		} else if( errno == EPERM) { // No permission to send signal
			printf("No permission to send signal!\n");

		} else
			printf("Signal sent. All Ok!\n");


	}
}

int getname(char store[100][100],char*path)//find file name in directionary
{
	DIR *d;
	struct dirent *dir;
	char filename[100];
	sprintf(filename,path);
	d = opendir(filename);
	int count=0;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			char *temp = malloc(200);
			char *subpath = malloc(200);
			// concat path/ with 1.txt => path/1.txt
			if(dir->d_type==4&&dir->d_name[0]!='.') { //sub folder
				sprintf(subpath,path);
				strcat(subpath,dir->d_name);
				strcat(subpath,"/");
				//printf("sub: %s\n",subpath);
				getname(store,subpath);
			} else {
				sprintf(temp,path);
				sprintf(store[index],dir->d_name);
				strcat(temp,store[index]);
				sprintf(store[index++],temp);
				//printf("name: %s :%d\n",dir->d_name,dir->d_type);
				//     printf("%s\n",dir->d_name);
				//count++;//前兩個是. and ..（測試結果）

				if(dir->d_name[0]=='.')index--;
			}
		}
		closedir(d);
	} else printf("ERROR");
//    printf("index:%d\n",index);
	return index;


}


int send_to_fd(int sysfs_fd, struct mail_t *mail)
{
	//set as key-path+
	char *buf =malloc(200);
	sprintf(buf,mail->data.query_word);
	strcat(buf,"-");
	strcat(buf,mail->file_path);
	strcat(buf,"+");

	int ret_val = write(sysfs_fd,buf,100);
	if (ret_val == ERR_FULL) {

	} else {

	}
	return ret_val;
}


int receive_from_fd(int sysfs_fd, struct mail_t *mail)
{
	char *infor = malloc(100);
	int ret_val = read(sysfs_fd,infor,100);
	close(sysfs_fd);
	sysfs_fd = open("/sys/kernel/hw2/mailbox",O_RDWR);

	int i =0;
	if(ret_val>0) {
		while(infor[i]!='-') { //find keyword
			i++;
		}
		char*temp = malloc(100);
		substr(temp,infor,0,i);//XXXXX-
		mail->data.word_count = atoi(temp);
		int j=0;
		while(infor[j]!='+') { //find path
			j++;
		}
		substr(mail->file_path,infor,i+1,j-i-1);//-XXXXX+ count = j-i-1
	}

	return ret_val;
}
void substr(char *dest, const char* src, unsigned int start, unsigned int cnt)
{
	strncpy(dest, src + start, cnt);
	dest[cnt] = 0;
}


