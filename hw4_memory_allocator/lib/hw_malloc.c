#include "hw_malloc.h"
int firstalloc = 0 ;//第一次要去跟系統要64k
void * heapstart;
void * nowstart;
//extern struct chunk_header bin[7];
struct chunk_header* prev;//紀錄前一個人的資訊
void binsort(struct chunk_header*);
void bininit();
void *catch(int,size_t );
struct alloc {
	long address;
	struct alloc *next;
};

struct alloc *allochead;
void *hw_malloc(size_t bytes)
{
	if(firstalloc==0) {
		allochead = malloc(sizeof(struct alloc));

		bininit();
		heapstart = sbrk(65536);//要64k byte

//  printf("l:%p\n",sbrk(64000)-heapstart);
		firstalloc = 1;//之後都不要再要
//        printf("heapstart:%p\n",get_start_sbrk());
		nowstart = heapstart;
		struct chunk_header * firsthead=malloc(sizeof(struct chunk_header));
		firsthead->chunk_size =65536-40;//扣掉header
		firsthead->prev_chunk_size = 0;
		firsthead->prev_free_flag = -1;//前面沒人了 把flag設為-1
		firsthead->my_flag = 1;//一開始是free
		firsthead->prev = &bin[6];
		firsthead->next = &bin[6];
		firsthead->address = heapstart;
		memcpy(nowstart,firsthead,40);
		bin[6].next = heapstart;
		bin[6].prev = heapstart;
		free(firsthead);
	}
//    printf("size:%d\n",bin[6].next->chunk_size);

	void *ret;
	struct chunk_header * nexthead=malloc(sizeof(struct chunk_header));
//	struct chunk_header * head= nowstart;
	if(bytes<=8&&bin[0].next!=&bin[0])//直接抓bin[0]的一個人走
		return catch(0,bytes);
	else if(bytes<=16&&bin[1].next!=&bin[1])
			return catch(1,bytes);
		else if(bytes<=24&&bin[2].next!=&bin[2])
				return catch(2,bytes);
			else if(bytes<=32&&bin[3].next!=&bin[3])
					return catch(3,bytes);
				else if(bytes<=40&&bin[4].next!=&bin[4])
						return catch(4,bytes);
					else if(bytes<=48&&bin[5].next!=&bin[5])
							return catch(5,bytes);

	//從bin6找
	struct chunk_header *head = bin[6].prev;
	while(head!=&bin[6] && head->chunk_size<bytes) {
		head=head->prev;
		if(head==&bin[6]) return -1;//繞一圈回來,表示找不到了 return error
	}
	while(head->prev!=NULL&&head->prev->chunk_size==head->chunk_size)
		head=head->prev;
	nowstart = head-> address;
	//找到了(head存要切的那塊)
	if(head->chunk_size -
	   bytes<48) { //會到這邊表示是bin6 的其中一個node
		//把bin 裡面調好
		head->prev->next = head->next;
		head->next->prev = head->prev;
		head->next = NULL;
		head->prev = NULL;
		//更新資訊
		head->my_flag = 0;
		struct chunk_header * temp;//下一個人的prev flag要改
		//temp = (long)&head + 40 + head->chunk_size;
		temp = (long)head->address+(long)head->chunk_size+40;
		//  printf("--------%p\n",(long)temp-(long)heapstart);
		if(((long)temp-(long)heapstart)<65536) {
			temp->prev_free_flag = 0;
		}
		//	printf("--------%p\n",(long)temp-(long)heapstart);
		//   if(((long)temp-(long)heapstart)==18088)printf("--------%d\n",temp->prev_free_flag);
		struct alloc *node = malloc(sizeof(struct alloc));
//        printf("------------------%p",head->address+40);
		node->address=(head->address)+40;
		node->next=allochead->next;
		allochead->next=node;

		return head->address +
		       40;//addres存的是head開始的address,+40才是return值

	}//處理剩下<8 不用切的狀況

	//要切割的狀況 nexthead是剩下來要插回去bin[6]的

	size_t alloc_byte=0;//存要allocate出去的byte
	while(alloc_byte<bytes)
		alloc_byte =alloc_byte+ 8;
//	printf("actual allocate byte:%ld\n",alloc_byte);


	//nexthead存切完要插回去的那塊
	nexthead->chunk_size  = head->chunk_size - alloc_byte-40;

	nexthead->prev_chunk_size = alloc_byte;
	nexthead->prev_free_flag = 0 ;//因為前一塊被分出去了
	nexthead->address = head->address+alloc_byte+40;
	nexthead->my_flag = 1;//我還是free的

	if(nexthead->chunk_size>8) {
		nexthead->next = head->next;
		nexthead->prev = head->prev;

		memcpy(nowstart+40+alloc_byte,nexthead,40);//切完剩下的head
		head->next->prev = nowstart+40+alloc_byte;
		head->prev->next = nowstart+40+alloc_byte;
	} else { //need to insert in bin[0]~bin[5] because after cut,size<48


		int place = (nexthead->chunk_size/8)-1;//which bin should insert
		//update original bin6
		head->next->prev = head->prev;
		head->prev->next = head->next;

		//update bin1~5
		nexthead->prev = bin[place].prev;
		nexthead->next = &bin[place];
		bin[place].prev->next = nexthead;
		bin[place].prev = nexthead;
		memcpy(nowstart+40+alloc_byte,nexthead,40);//切完剩下的head

	}



//update cut chunk's next
	struct chunk_header *a=(long)(nexthead->address)+(long)nexthead->chunk_size +40;
	if((long)a-(long)heapstart<65535) {
		(a->prev_chunk_size) =(nexthead->chunk_size);
	}


	//移除並插入新的
	//if(head->prev==&bin[6])printf("\na");
	//if(head->next==&bin[6])printf("b\n");

	//----------------要allocate出去的
	head->prev= NULL;
	head->next = NULL;//從list移除,因為allocate出去了
	head->chunk_size = alloc_byte;
	head->address = nowstart;
	head->my_flag = 0;//我分配出去了
	//------------
	free(nexthead);
	ret = nowstart+40;
//	nowstart = nowstart+40+alloc_byte;//下一個人要從哪裡開始放

//    printf("end:%p\n",nowstart-heapstart);
//    struct chunk_header *tmp = heapstart;
//    printf("%d\n",tmp->chunk_size);
	struct alloc *node = malloc(sizeof(struct alloc));
	node->address=ret;
	node->next=allochead->next;
	allochead->next=node;
//    printf("%d\n",alloc_byte);
	//printf("------%p\n",ret+alloc_byte-heapstart);
//    if((long)ret-(long)heapstart==65376)printf("------%p\n",ret-heapstart);
	return ret;

}

int hw_free(void *mem)
{
	void *nodejudge = mem;
	mem = (long ) mem + heapstart;
	struct chunk_header * headaddress =mem;//頭的address
	int index ;
	//printf("%p\n",mem);
	//printf("address:%p size:%d\n",headaddress->address,headaddress->chunk_size);
	struct alloc *node =allochead->next;
	struct alloc *nodeprev = allochead;
	while(node!=NULL&&node->address!=((long)mem+40)) {

		//printf("%p , %p\n",(long)mem+40,node->address);
		nodeprev=node;
		node=node->next;
	}

	if(node==NULL)return 2;
	else {

		nodeprev->next=node->next;
		free(node);
	}

	//要插到哪個bin[index]
	index = ((headaddress->chunk_size)/8)
	        -1;//直接算出要free插到哪個bin下面
	if(index>=6) index = 6;

	//把後一個人的prev flag改成free

	struct chunk_header * next = (long)mem+40+(headaddress->chunk_size);
	if(next!=(65536+heapstart))next->prev_free_flag = 1 ;

	//把自己的flag改成free
	headaddress->my_flag = 1;
	//要merge的狀況
	struct chunk_header * tempnext;
	tempnext =(long)mem+40+(headaddress->chunk_size);//指向後一個人的頭

//	 printf("next:%p %p\n",(long)headaddress-(long)heapstart,(long)tempnext-(long)heapstart);//
	//如果有merge 一定會插在bin[6]裡面 因為最小＝8+40+8+40＝96
	int merge = 0;//判斷有沒有merge過

//	 printf("flag:%d\n",headaddress->prev_free_flag);
	// printf("flag:%d\n",tempnext->my_flag);
//    if(tempnext->my_flag==1){
	if(headaddress->prev_free_flag==1) { //如果前一個人是free
		//	  printf("merge previous  \n");
		struct chunk_header *test = headaddress;
		merge=1;
		//保留前面那個人的head
		//前一個人的頭的位置
		//printf("prevsize:%d\n",headaddress->prev_chunk_size);
		int addsize =  headaddress->chunk_size;//目前的chunksize
		headaddress = (long)mem -(headaddress->prev_chunk_size)
		              -40;//新頭是前面那個人
//		printf("newhead %p \n",(long) headaddress -(long) get_start_sbrk());

		headaddress->chunk_size = headaddress->chunk_size + 40 + addsize;
		//test為前面那個人的head 現在要變大了 要改bin(因為他本來插在bin裡面)
		//	printf("**********%p\n",(long)headaddress-(long)heapstart);

		headaddress->prev->next = headaddress->next;
		headaddress->next->prev = headaddress->prev;
		// printf("**********%p\n",(long)headaddress-(long)heapstart);

		//原本有兩個頭 現在merge要剩前面的頭所以要清掉後面的頭
		memset(test,0,40);//test是要清掉的頭的位置 清掉head=40bytes
		//把後一個人的head->prev_chunk_size更新

		test =  (long)headaddress + 40+ headaddress->chunk_size;//下一顆頭的位置
		test->prev_chunk_size =  headaddress->chunk_size;

	}


	if(tempnext!=(65536+heapstart)&&tempnext->my_flag==1) { //後一個人是free
		//	printf("merge next\n");
		//要留的是目前的那個headaddress 清掉後一個人的headaddress
		headaddress->chunk_size =40+ tempnext->chunk_size + headaddress->chunk_size ;
		//     printf("%p newsize:%d\n",(long) headaddress -(long) get_start_sbrk    (),headaddress->chunk_size);
		//記得把bin改掉
		tempnext->prev->next = tempnext->next;
		tempnext->next->prev = tempnext->prev;
		//  printf("--------%p\n",(long)tempnext-(long)heapstart);
		//memset(tempnext,0,40);//test是要清掉的頭的位置 清掉head=40bytes
		memset(tempnext,0,40);
		merge = 1;
		//更新後面一個人的prev_size
		//printf("--------%p\n",(headaddress->address)-heapstart);
//        if(headaddress->chunk_size<65496){
//      memset(tempnext,0,40);
		struct chunk_header *temp = (long)headaddress+40+headaddress->chunk_size;
		//    printf("-------------%p %d\n",(long)temp-(long)heapstart, headaddress->chunk_size);
		if(temp<65536+heapstart)
			temp->prev_chunk_size = headaddress->chunk_size;
		//      }
	}


	if(merge==1)//有merge過一定是串到6裡面
		index = 6;


	if(index==6
	   &&bin[6].prev!=&bin[6]) { //是index6而且bin[6]裡面有東西的要經過sort
		struct chunk_header* temp = bin[6].prev;
		while(temp->chunk_size<headaddress->chunk_size&&temp!=&bin[6])
			temp = temp->prev;

		//到這裡表示temp指向比要插入的還大的那個人,應該插在temp的後面
		headaddress->prev = temp;
		headaddress->next = temp->next;
		temp->next->prev = headaddress;
		temp->next = headaddress;
	} else { //不是index6的,或是因為bin[6]裡面沒東西所以不用經過sort
		//開始插到bin[index]的最後面
		headaddress->prev = bin[index].prev;
		headaddress->next = &bin[index];
		(bin[index].prev)->next = headaddress;
		bin[index].prev = headaddress;
	}
	merge=0;
	return 1;
}

void *get_start_sbrk(void)
{
	return heapstart;
}

void bininit()
{
	int i ;
	for(i=0; i<7; i++) {
		bin[i].prev_free_flag = -1;//表示是bin[]
		bin[i].next = &bin[i];
		bin[i].prev = &bin[i];
	}

}
void *catch(int index,size_t bytes)
{
	struct chunk_header * temp;
	struct chunk_header * tempnext;
	temp = bin[index].next;
//	tempnext = &temp + 40 + temp->chunk_size;
	tempnext = (long)temp->address+(long)temp->chunk_size+40;
	//改自己的flag和後面一個人的flag
	temp->my_flag = 0;
	if(((long)tempnext-(long)heapstart)<65536)
		tempnext->prev_free_flag = 0;
	//改好list
	bin[index].next = temp->next;
	temp->next->prev = temp->prev;
	temp->prev->next = temp->next;
	temp->next = NULL;
	temp->prev = NULL;
//	temp->chunk_size = bytes;
//    temp-> address

	struct alloc *node = malloc(sizeof(struct alloc));
	node->address=(long)temp->address+40;
	node->next=allochead->next;
	allochead->next=node;
//    printf("------%p\n",(long)temp->address+40-(long)heapstart);
	return temp->address+40;//addres存的是head開始的address,+40才是return值
}

