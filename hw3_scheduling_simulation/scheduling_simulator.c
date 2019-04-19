#include "scheduling_simulator.h"
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <stddef.h>
char qutumn = 'S';
int globalpid=0;

time_t timer_remain_value=0;//ctrl+z need store remain qutumn
suseconds_t remain_value_usec = 10000 ;//10000us = 10ms
int judge_for_ctrlz=
    0;//如果是因為ctrlz回去main ,值是1 如果是因為terminate or suspend回去,值是0

struct task {
	char stack[8192];
	ucontext_t ctx;//context
	int pid;
	char Taskname[100];//taskname
	char Qutumn;//store time qutumn ,0 is small(default) 1 is Large
	int remainQutumn;//2->1
	struct task *next;
	enum TASK_STATE state;
	long int wait_time;//要等的總時間
	struct timespec startwait_time;//開始等的時間
	int queuetime;//等了多少單位10ms
};//linked list version

static ucontext_t mainctx;//mainfunction (default=main)
struct task * head ;
struct task * tail;//pointer to tail,start = head

struct task * head_tm;//terminate queue's head

struct task * head_wait;//waiting queue head
/*
static ucontext_t ctx[2000];//task queue(0=main function)(index is pid)
int index=1;//which pid should be schedule(0=main function)(index is pid) index is also the head of the queue
int empty=1;//if add a task, which location should bee place(0 is main)
char taskname_array[2000][100];//string[2000] string = char[100]
int qutumn_array[2000];//store time qutumn ,0 is small(default) 1 is Large
TASK_STATE state[2000];
*///array version

int mode = 0;//0:shell mode
void tasktest1(void);
void tasktest2(void);
void tasktest3(void);
void tasktest4(void);
void tasktest5(void);
void tasktest6(void);
struct timespec
	globaltime;//用來看現在的時間 與suspend的時間相減就知道該不該醒了 clock_t的精準度到ms
struct timespec stoptime;//獲取要stop時的時間

void hw_suspend(int msec_10)
{

	struct task * tmp;
	//把目前running的node從ready list刪除並串在waiting list上
	head->next->state = TASK_WAITING;
	//把ready queue設好
	tmp = head->next;
	head->next=head->next->next;
	if(tail==tmp)//如果我是tail 要改tail
		tail = head;
	//串進wait list裡面
	tmp->next=head_wait->next;
	head_wait->next= tmp;
	//設定等多少時間
	tmp->wait_time = msec_10*10;//等多少個10ms
	clock_gettime(CLOCK_MONOTONIC, &tmp->startwait_time);//紀錄開始等的時間
//    tmp->startwait_time = clock();//紀錄開始等的時間
	//為下一個人重置qutum
	timer_remain_value = 0;
	remain_value_usec=10000;
	judge_for_ctrlz=1;
	swapcontext(&tmp->ctx,
	            &mainctx);//儲存目前狀態並回到main 找下一個人

	return;
}

void hw_wakeup_pid(int pid)
{
	if(head_wait!=NULL) {
		struct task * tmp=head_wait->next;//從waiting queue 找人
		struct task* waittmp = head_wait;
		while(tmp!=NULL) {
			if(tmp->pid == pid) { //找到要的人了 串到ready後面
				tmp->state = TASK_READY;
				waittmp->next = tmp->next;
				tail->next = tmp;
				tail = tmp;
				tmp->next = NULL;
				break;
			}
			waittmp = tmp;
			tmp = tmp->next;
		}
	}
	return;
}


int hw_wakeup_taskname(char *task_name)
{
	struct task * tmp=head_wait->next;//從waiting queue 找人
	struct task * tmpnext= head_wait->next;
	int count = 0;
	struct task* waittmp = head_wait;//pointer to the previous node of tmp

	while(tmp!=NULL) {

		tmpnext = tmp->next;
		if(strcmp(tmp->Taskname,task_name)==0) { //找到要的人了 串到ready後面
			count ++;
			tmp->state = TASK_READY;

			waittmp->next = tmp->next;
			tail->next = tmp;
			tail = tmp;
			tmp->next = NULL;
		} else
			waittmp = tmp;
		tmp = tmpnext;
	}
	return count;
}

int hw_task_create(char *task_name)
{
	struct task * tmp = malloc(sizeof( struct task));//new node
	sprintf(tmp->Taskname,"%s",task_name);
//  printf("create: %s\n",tmp->Taskname);
	tmp->state = TASK_READY;
	tmp->pid = ++globalpid;
	tmp->Qutumn = qutumn;
	if(qutumn=='L') {
		tmp->remainQutumn=2;
	} else {
		tmp->remainQutumn=1;
	}
	tmp->next= NULL;

	getcontext(&tmp->ctx);//initilize ctx
	tmp->ctx.uc_stack.ss_sp = tmp->stack;
	tmp->ctx.uc_stack.ss_size = sizeof tmp->stack;
	tmp->ctx.uc_link = &mainctx;//terminate,back to main

	tmp->queuetime=0;
	//set context of function ,Task x  or task x
	if(strcmp(task_name,"Task1")==0||strcmp(task_name,"task1")==0)
		makecontext(&tmp->ctx,task1, 0);
//makecontext(&tmp->ctx,tasktest1, 0);
	else if(strcmp(task_name,"Task2")==0||strcmp(task_name,"task2")==0)
		makecontext(&tmp->ctx,task2, 0);
//makecontext(&tmp->ctx,tasktest2,0);
	else if(strcmp(task_name,"Task3")==0||strcmp(task_name,"task3")==0)
		makecontext(&tmp->ctx,task3, 0);
//makecontext(&tmp->ctx,tasktest3,0);
	else if(strcmp(task_name,"Task4")==0||strcmp(task_name,"task4")==0)
		makecontext(&tmp->ctx,task4, 0);
//makecontext(&tmp->ctx,tasktest4,0);
	else if(strcmp(task_name,"Task5")==0||strcmp(task_name,"task5")==0)
		makecontext(&tmp->ctx,task5, 0);
//makecontext(&tmp->ctx,tasktest5,0);
	else if(strcmp(task_name,"Task6")==0||strcmp(task_name,"task6")==0)
		makecontext(&tmp->ctx,task6, 0);
//makecontext(&tmp->ctx,tasktest6,0);
	else return -1;//沒有這個task
	//add to queue
	tail->next = tmp;
	tail = tmp;
	// printf("%d  %s  %d  %c\n",tail->pid,tail->Taskname,tail->state,tail->Qutumn);

	return tmp->pid; // the pid of created task name
}

struct itimerval new_value, old_value;//timer declare

void ctrl_z_handler(int signal)
{
	getitimer(ITIMER_REAL,
	          &new_value);//store remain timer(how many time should be set when resume)
	timer_remain_value = new_value.it_value.tv_sec;//store sec and usec
	remain_value_usec = new_value.it_value.tv_usec;//store sec and usec
	printf("timer_remain_value: %ld %ld\n",(long int )timer_remain_value,
	       new_value.it_value.tv_usec);
	if(head->next==
	   NULL) { //處理如果沒有task就跑到simulation,按了ctrl z的狀況
		timer_remain_value = 0;
		remain_value_usec = 10000;

	}


	mode = 0;//back to shell mode
	new_value.it_value.tv_sec=0;//stop timer
	new_value.it_value.tv_usec=0;
	setitimer(ITIMER_REAL, &new_value, &old_value);
	//while(1); 測試如果在signalhandler內有signal =>結果後面的signal都不會被catch到
	struct task * now=head->next;
	if(now!=NULL) {
		judge_for_ctrlz=1;
		swapcontext(&now->ctx,&mainctx);//store now and back to main

	}

	return ;

}
void tmr_handler(int signal)
{

	//when 10 ms expired
	//先把所有在ready的人的queueing time + 1單位

	if(head->next!=NULL) {
		struct task* queue =
			    head->next->next;//應該加的是在head->next->next 因為head->next 是running
		while(queue!=NULL) {
			queue->queuetime ++;//表示又等了一單位
			queue=queue->next;

		}
	}
	struct task * now=head->next;

	//suspend時間-10ms
	struct task * waitadder = head_wait->next;
	while(waitadder!=NULL) {
		(waitadder->wait_time) -=10;//suspend time -10ms
//    printf("%d ",waitadder->wait_time);
		waitadder = waitadder ->next;
	}
//    printf("\n");

	//先看看有沒有人要從waiting醒來
	struct task * wake= head_wait->next;
	struct task* waittmp= head_wait;
	long wait;
	while(wake!=NULL) { //waiting queue還有人需要看是否醒
		if(wake->wait_time<=0)
			//如果需要等的時間已經沒了
		{
			//串到ready queue後面
			wake->state= TASK_READY;
			waittmp->next = wake->next;//從wait queue 拔掉

			tail->next = wake;
			tail = wake;
			wake->next = NULL;
		} else
			waittmp = wake;
		wake = wake->next;
	}
//	printf("************************pid%d %d\n",head->next->pid,tail->pid);

	//printf("10ms expired...\n");
	if(now!=NULL) {
		struct task * coming=now->next;
		if(now->remainQutumn==2) { //remain 2 qutumn no need change
			now->remainQutumn=1;
		} else if(now->next!=NULL) { //need change
			if(now->Qutumn=='L') {
				now->remainQutumn=2;//下一次來再配給他L
			}
			//改狀態
			now->state = TASK_READY;
			coming->state = TASK_RUNNING ;
			//now 排到隊伍尾巴
			head->next = coming;
			tail->next = now;
			tail = now;
			now->next = NULL;

			swapcontext(&now->ctx,&coming->ctx);//??? 會存到正確的進度嗎
		}
		//   swapcontext(&now->ctx,&mainctx);
	}
	//printf("switch\n");
}


void tasktest1(void)
{
	int a =0,i,j;
	puts("start task1");
	while(1) {
		for(i=0; i<32767; i++)for(j=0; j<300; j++);
		printf("task1: %d \n",a++);

	}

}
void tasktest2(void)
{
	int a =0,i,j;
	puts("---start task2");

	while(1) {
		for(i=0; i<32767; i++)for(j=0; j<500; j++);
		printf("---task2: %d \n",a++);
	}
}
void tasktest3(void)
{
	int a =0,i,j;
	puts("---start task3");
	//while(1){
	for(i=0; i<32767; i++)for(j=0; j<800; j++);
	//   printf("------task3: %d \n",a++);
	printf("task3 over\n");
	//}
}

void tasktest4(void)
{
	puts("************start task4");
	while(1) {

		hw_suspend(500);//suspend 5s
		printf("*********task4 wakeup\n");
	}
}
void tasktest5(void)
{
	int i, j;
	while(1) {
		for(i=0; i<32767; i++)for(j=0; j<800; j++);
		printf("+++++++++++++++++++++++wake up pid3 man\n");
		hw_wakeup_pid(3);
	}
}
void tasktest6(void)
{
	int i, j;
	while(1) {
		for(i=0; i<32767; i++)for(j=0; j<800; j++);
		printf("############wake up Mr4 %d\n",hw_wakeup_taskname("Task4"));
	}

}

int main()
{
	head =  malloc(sizeof(struct task));
	tail = head;
	head_tm = NULL;//terminate 初始＝NULL
	head_wait = malloc(sizeof(struct task));//waiting queue 的head

	char* command = malloc(100);

	//****signal ctrl+z設定*****************
	//signal(SIGTSTP,ctrl_z_handler);
	struct sigaction setup_ctrlz;
	sigset_t block_mask;
	sigemptyset (&block_mask);//初始化mask
	sigaddset (&block_mask,
	           SIGALRM);//在set加入alarm 這樣在ctrl z的handler裡面就不會對alarm做出反應
	setup_ctrlz.sa_handler = ctrl_z_handler;//設定handler的function
	setup_ctrlz.sa_mask = block_mask;//設定剛剛加入的mask 集合
	setup_ctrlz.sa_flags = 0;//把flag初始化為0
	sigaction (SIGTSTP, &setup_ctrlz,
	           NULL);//把剛剛的設定設為綁在接SIGTSTP的handler function
	// ****signal timer設定*****************
	//signal(SIGALRM, tmr_handler);
	struct sigaction setup_alarm;
	sigset_t block_mask2;
	sigemptyset (&block_mask2);//初始化mask2
	sigaddset (&block_mask2,
	           SIGTSTP);//在set加入SIGTSTP 這樣在alarm的handler裡面就不會對ctrlz做出反應
	setup_alarm.sa_handler = tmr_handler;//設定handler的function
	setup_alarm.sa_mask = block_mask2;//設定剛剛加入的mask 集合
	setup_alarm.sa_flags = 0;//把flag初始化為0
	sigaction (SIGALRM, &setup_alarm,
	           NULL);//把剛剛的設定設為是接SIGTSTP的handler function

	//設定clock
	new_value.it_value.tv_sec = 0;
	new_value.it_value.tv_usec = 0;//it_value both=0,so it won't be start
	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_usec = 10000;//1000000;
	setitimer(ITIMER_REAL, &new_value, &old_value);

	double wait;
	struct task* wake;
	while(1) {
		while(mode==0) { //shell mode
			//		printf("shell mode\n");
			printf("$");
			gets(command);

			if(strcmp(command,"start")==0) { //start
				mode = 1;//simulation mode
				//把時間恢復
				new_value.it_value.tv_sec = timer_remain_value;//let timer b    egin
				new_value.it_value.tv_usec = remain_value_usec;
				new_value.it_interval.tv_sec = 0;
				new_value.it_interval.tv_usec = 10000;//1000000;
				setitimer(ITIMER_REAL, &new_value, &old_value);

			} else if(strcmp(command,"ps")==0) { //ps
				printf("ps\n");

				struct task * tmp=head->next;
				char * Status = malloc(20);
				Status = "False";
				//印出ready和running
				while(tmp!=NULL) {
					if(tmp->state==1)Status = "TASK_RUNNING";
					else if(tmp->state==2)Status = "TASK_READY";
					printf(" %d  %s%18s %c %dms \n",tmp->pid,tmp->Taskname,Status,tmp->Qutumn,
					       10*tmp->queuetime);
					tmp = tmp->next;
				}
				//印出terminate
				tmp = head_tm;
				while(tmp!=NULL) {
					printf(" %d  %s%18s %c %dms \n",tmp->pid,tmp->Taskname,"TASK_TERMINATED",
					       tmp->Qutumn,10*tmp->queuetime);
					tmp = tmp->next;
				}
				//印出waiting
				tmp = head_wait->next;
				while(tmp!=NULL) {
					printf(" %d  %s%18s %c %dms \n",tmp->pid,tmp->Taskname,"TASK_WAITING",
					       tmp->Qutumn,10*tmp->queuetime);
					tmp = tmp->next;
				}

			} else {
//        printf("%s\n",command);
				char * judge = malloc(20);
				sscanf(command,"%s",judge);
				if(strcmp(judge,"add")==0) { //add
					//*******reading information
					char * taskname = malloc(100);
					char * garbage = malloc(100);
					//char qutumn = 'S';
					char temp ;
					sscanf(command,"%s %s %s %c",judge,taskname,garbage,&temp);
					if(strcmp(garbage,"-t")==0&&(temp=='S'||temp=='L')) { //optional -t
						sscanf(command,"%s %s %s %c",judge,taskname,garbage,&qutumn);
					} else {
						qutumn='S';
					}
//					printf("add %s ,time:%c\n",taskname,qutumn);
					//*******add task
					//if(qutumn=='L')qutumn_array[empty] = 1;
					hw_task_create(taskname);

				} else if(strcmp(judge,"remove")==0) { //remove
					int pid =-1;
					sscanf(command,"%s %d",judge,&pid);
					printf("remove %d\n",pid);
					struct task * tmp=head->next;
					struct task * prev=head;
					//prev point to the previous node of tmp
					int i=0;
//12/18------i=0: remove from ready queue i=1: waiting
					for(i=0; i<2; i++) {
						if(i==1) { //waiting
							tmp = head_wait->next;
							prev = head_wait;
						}
						while(tmp!=NULL&&tmp->pid!=pid) {
							prev = tmp;
							tmp = tmp->next;
						}
						if(tmp!=NULL) { //find the node to be removed
							prev->next = tmp->next;
							if(tmp==head->next)
								//刪除ready第一個node,把剩下時間砍掉重新設為10ms
							{
								printf("delete first");
								timer_remain_value=0;
								remain_value_usec = 10000;
							} else if(tmp==tail)
								//刪除最後一個node
							{
								tail = prev;
							}
							free(tmp);
						}
					}
//----------------------remove from terminate
					struct task * tmp_tm=head_tm;
					struct task * prev_tm= tmp_tm;
					if(head_tm!=NULL&&head_tm->pid==pid) { //delete head point
						head_tm=tmp_tm->next;
						free(tmp_tm);
						tmp_tm=NULL;
					} else if(head_tm!=NULL) {
						prev_tm = tmp_tm;
						tmp_tm = tmp_tm->next;
					}

					while(tmp_tm!=NULL&&tmp_tm->pid!=pid) {
						prev_tm = tmp_tm;
						tmp_tm = tmp_tm->next;
					}
					if(tmp_tm!=NULL) {//find
						prev_tm->next = tmp_tm->next;
						free(tmp_tm);
					}


				}



			}


		}//end of shell mode

		printf("simulating...\n");

		while(mode==1) { //simulation mode
			//tmp_for_simulation = head->next;

			/*    //先把時間恢復
				new_value.it_value.tv_sec = timer_remain_value;//let timer begin
				new_value.it_value.tv_usec = remain_value_usec;
				new_value.it_interval.tv_sec = 0;
				new_value.it_interval.tv_usec = 10000;//1000000;
				setitimer(ITIMER_REAL, &new_value, &old_value);*/
			//再看看有沒有人要跑
			while(head->next!=NULL&&mode==1) {//have task need to be simula    te
				head->next->state = TASK_RUNNING;
				swapcontext(&mainctx,&head->next->ctx);//from main to task
				if(judge_for_ctrlz==1) { //back from ctrl+z or someone suspend
					//printf("back from pause or suspend\n");
					judge_for_ctrlz = 0;
					//因為如果terminate 不會去設定到這個值,所以要在這裡回復到0
					break;//到shell mode
				} else { //back because terminate ,need delete first node(terminate)
					//printf("back because terminate\n");
					struct task * tmp;
					//會回來表示head->next 已經terminate 把他從ready list刪除並串在terminate list上
					head->next->state = TASK_TERMINATED;
					//把ready queue設好
					tmp = head->next;
					head->next=head->next->next;
					if(tail==tmp)//如果我是tail 要改tail
						tail = head;

					//串進terminate list裡面
					tmp->next=head_tm;
					head_tm = tmp;

					//為下一個人重置qutumn
					timer_remain_value = 0;
					remain_value_usec=10000;

					break;
				}
			}

		}
	}
	return 0;
}
