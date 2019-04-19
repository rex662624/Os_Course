#include "slave.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
int countword(struct mail_t*);
void substr(char *dest, const char* src, unsigned int start,
            unsigned int cnt);//find substring

int main(int argc, char **argv)
{

	int sysfs_fd = open("/sys/kernel/hw2/mailbox",O_RDWR);



	char *recmsg = (char*)malloc(100);
	int a,i=0;
	int count;

	struct mail_t * mail= malloc(sizeof(struct mail_t));



	while(1) {
		int b = receive_from_fd(sysfs_fd,mail);
		//printf("slave read %d : %s\n",b,mail->file_path);
		close(sysfs_fd);
		sysfs_fd = open("/sys/kernel/hw2/mailbox",O_RDWR);
//        printf("b:%d\n",b);
		while(b>=0) {
			// printf("slave read %d : %s\n",b,mail->file_path);
//        printf("path %s\n",mail->file_path);
			count = countword(mail);
			mail->data.word_count = count;
			//while(send_to_fd(sysfs_fd,mail));
			int c = send_to_fd(sysfs_fd,mail);
			if(c==0)break;
		}
		//printf("count %s : %d\n",mail->file_path,count);
	}
	//printf("count: %d",count);
	close(sysfs_fd);
//    while(1);
}

int countword(struct mail_t *mail)
{
	FILE *fp;
	int wordcount=0;
	char *path=malloc(100);
	char *keyword=malloc(100);
	char *string = malloc(100);
	int i =0;
	sprintf(keyword,mail->data.query_word);
	sprintf(path,mail->file_path);
	/*    while(infor[i]!='-')//find keyword
	    {
	        i++;
	    }
	    substr(keyword,infor,0,i);//XXXXX-
	    int j=0;
	    while(infor[j]!='+')//find path
	    {
	        j++;
	    }
	    substr(path,infor,i+1,j-i-1);//-XXXXX+ count = j-i-1*/
//    printf("key:%s path:%s\n",keyword,path);

	char*tmp =malloc(100);
	sprintf(tmp,keyword);
	strcat(tmp,".");//apple.
	fp = fopen(path,"r");
	if ( fp ) {
		while(fscanf(fp,"%s",string)==1) {
			if(strcmp(keyword,string)==0) wordcount++;
			else if (strcmp(keyword,string)!=0) {
				//apple, apple. apple? apple! apple;
				int i2;
				int judge=0;
				for(i2=0; i2<strlen(keyword); i2++)
					if(string[i2]!=keyword[i2])judge=-1;
				char a = string[strlen(keyword)];
				if(a!='.'&&a!='?'&&a!=','&&a!=';'&&a!='!')judge=-1;
				if(judge==0) {
					wordcount++;
					int j2;
					//apple,apple ; apple.Apple
					if(string[strlen(keyword)+1]!=keyword[0]
					   &&string[strlen(keyword)+1]!=toupper(keyword[0]))judge=-1;//.a && .A
					for(j2=1; j2<strlen(keyword); j2++) { //pple
						if(keyword[j2]!=string[strlen(keyword)+1+j2])judge=-1;
					}
					if(judge==0)wordcount++;
				}

				//Apple
				char up = toupper(keyword[0]);
				//    printf("%c \n",up);
				if(strlen(keyword)==strlen(string)&&string[0]==up)wordcount++;

			}
		}
	} else {
//        printf("Failed to open the file\n");
	}
	return wordcount;
}
void substr(char *dest, const char* src, unsigned int start, unsigned int cnt)
{
	strncpy(dest, src + start, cnt);
	dest[cnt] = 0;
}

int send_to_fd(int sysfs_fd, struct mail_t *mail)
{
	//set as key-path+
	char *buf =malloc(200);
	sprintf(buf,"%d",mail->data.word_count);
	strcat(buf,"-");
	strcat(buf,mail->file_path);
	strcat(buf,"+");
	//printf("slave write: %s\n",buf);
	int ret_val = write(sysfs_fd,buf,100);

	/*
	 * write something or nothing
	 */

//	int ret_val = write(sysfs_fd, ...);
//	if (ret_val == ERR_FULL) {
	/*
	 * write something or nothing
	 */
//	} else {
	/*
	 * write something or nothing
	 */
//	}

	/*
	 * write something or nothing
	 */
	return ret_val;
}

int receive_from_fd(int sysfs_fd, struct mail_t *mail)
{
	/*
	 * write something or nothing
	 */
	char *infor = malloc(100);
	int ret_val = read(sysfs_fd,infor,100);
	close(sysfs_fd);
	sysfs_fd = open("/sys/kernel/hw2/mailbox",O_RDWR);

//   printf("%d,slave rec:%s\n",ret_val,infor);

	if(ret_val>0) {
		//  printf("slave rec:%s\n",infor);
		int i =0;
		while(infor[i]!='-') { //find keyword
			i++;
		}
		substr(mail->data.query_word,infor,0,i);//XXXXX-
		int j=0;
		while(infor[j]!='+') { //find path
			j++;
		}
		substr(mail->file_path,infor,i+1,j-i-1);//-XXXXX+ count = j-i-1

	}
//	if (ret_val == ERR_EMPTY) {
	/*
	 * write something or nothing
	 */
//	} else {
	/*
	 * write something or nothing
	 */
//	}

	/*
	 * write something or nothing
	 */
	return ret_val;
}


