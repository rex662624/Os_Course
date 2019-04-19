#include "hw4_mm_test.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
int main()
{

	char * cmd = malloc(100);
	char * cmdname = malloc(50);
	char * cmdnum = malloc(50);
//    struct chunk_header head ;
//    struct chunk_header head2;
//    head.prev = &head2;
//    head.chunk_size = 5 ;

//	printf("chunk_header size: %ld\n", sizeof(struct chunk_header));

	while(gets(cmd)!=NULL) { //while != EOF
		sscanf(cmd,"%s %s",cmdname,cmdnum);
		if(strcmp(cmdname,"alloc")==0) { //alloc
			int num=0;
			num = atoi(cmdnum);
//          printf("alloc:%d\n",num);
			void * ret = hw_malloc(num);
			if(ret!=-1) {
				printf("0x%08lx\n",ret-get_start_sbrk());//改%p
			} else {
				printf("ERROR\n");
			}
		} else if(strcmp(cmdname,"free")==0) { //free
			//printf("free:%s\n",cmdnum);
			long freenum;
			sscanf(cmdnum,"%llx",&freenum);
			//printf("%d\n",freenum-40);
			if(hw_free(freenum-40)==1)printf("success\n");
			else printf("fail\n");

		} else if(strcmp(cmdname,"print")==0) { //print
			int num = atoi(&cmdnum[4]);
			struct chunk_header* temp = bin[num].next;
			while(temp!=&bin[num]) {
				printf("0x%08lx--------%ld\n",(temp->address)-(long int)get_start_sbrk(),
				       temp->chunk_size+40);//記得改%p(temp->address)-(long int)get_start_sbrk()
				temp = temp->next;
			}

		}

	}



	/*
	printf("chunk_header size: %ld\n", sizeof(struct chunk_header));
	printf("%p\n", hw_malloc(8));
	printf("%s\n", hw_free(NULL) == 1 ? "success" : "fail");
	printf("start_brk: %p\n", get_start_sbrk());*/


	return 0;
}
