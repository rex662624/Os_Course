#include "mailbox.h"
#include<linux/spinlock.h>
MODULE_LICENSE("Dual BSD/GPL");

static void get_process_name(char *ouput_name);
static ssize_t mailbox_read(struct kobject *kobj,
                            struct kobj_attribute *attr, char *buf);
static ssize_t mailbox_write(struct kobject *kobj,
                             struct kobj_attribute *attr, const char *buf, size_t count);

static struct kobject *hw2_kobject;
static struct kobj_attribute mailbox_attribute
    = __ATTR(mailbox, 0660, mailbox_read, mailbox_write);

static int num_entry_max = 2;
//static char *box;
module_param(num_entry_max, int, S_IRUGO);
//static LIST_HEAD(bufferhead);//create list head named bufferhead
static spinlock_t a_lock ;//定義lock
//DEFINE_SPINLOCK(a_lock);
static SPIN_LOCK_UNLOCKD(a_lock);
//static spin_lock_init(a_lock);
static unsigned long  flag;//lock flag

struct mailbox_entry_t bufferhead = {
	.count = -1,
};

static struct mailbox_entry_t *tmp ;
static struct mailbox_entry_t *ptr ;//指向目前運作的node

static void get_process_name(char *ouput_name)
{
	memcpy(ouput_name, current->comm, sizeof(current->comm));
}
static char box[100] ;
static ssize_t mailbox_read(struct kobject *kobj,
                            struct kobj_attribute *attr, char *buf)
{
	int a;//return值
	char  process[100] ;
	get_process_name(process);
	//printk("process: %s\n",process);
	//if(process=="slave")

	//else    //master
	//    box = "123456789abcdefghijk";
	//return sprintf(buf,"%s",box);

//   ptr = list_entry(ptr,struct mailbox_entry_t,entry);//指向目前node
//   ptr = ptr->count;
//   ptr = bufferhead.next;//ptr->entry.next;
//   ptr->count=5;
	/* if(ptr->entry.prev!=NULL)
	 ptr = ptr->entry.prev;*/

//	int a = sprintf(buf,"%s",ptr->count);
//    char* process = "123456789abcdefgh";
	//int a = sprintf(buf,"%s",process);

	spin_lock_irqsave(&a_lock,flag);//lock

	if(strcmp(process,"master")==0) {
		//master
		if(ptr->index!=num_entry_max) { //not empty
			ptr = list_prev_entry(ptr,entry);//讀出一個node 先指向要變成空的
//            printk("master in, pointer tag = %d\n",ptr->tag);
			if(ptr->tag==2) { //master should read slave's message
				strncpy(buf,ptr->name,strlen(ptr->name));//a=sprintf(buf,"%s",ptr->name);
				a = strlen(buf);
				printk("master read %d: %s \n",ptr->index,ptr->name);
			} else { //master shouldn't read
				ptr = list_next_entry(ptr,entry);//restore
				//a= sprintf(buf,"%s","a");
				a = -1;
				//            printk("master shouldn't read\n");
			}
		} else {
			a = -1;
		}

	} else {
		if(ptr->index!=num_entry_max) {
			ptr = list_prev_entry(ptr,entry);//讀出一個node 先指向要變成空的
			if(ptr->tag==1) { //slave should read master's message
				strncpy(buf,ptr->name,strlen(ptr->name));//a=sprintf(buf,"%s",ptr->name);
				a= strlen(buf);
				printk("slave read %d: %s \n",ptr->index,ptr->name);
			} else { //shouldn't read
				ptr = list_next_entry(ptr,entry);//restore
				//a= sprintf(buf,"%s","");
				a=-1;
				//            printk("slave read error: empty\n");
			}
		} else {
			a= -1;
		}
	}
	spin_unlock_irqrestore(&a_lock,flag);//釋放lock

//	a = sprintf(buf,"%s ",box);
//	printk("aaa\n");
//   ptr = ptr->entry.next;

	if(a>=0)printk("read a: %d\n",a);

	return a;
}

static ssize_t mailbox_write(struct kobject *kobj,
                             struct kobj_attribute *attr, const char *buf, size_t count)
{
	char  process[100] ;
	get_process_name(process);


	spin_lock_irqsave(&a_lock,flag);//lock
	int a=-2 ;//initilize to full
//        printk("%s\n",process);

	if(strcmp(process,"master")==0) { //master

		//開始write
		if(ptr->index!=0) { // if list not full
			printk("master write node%d : %s\n",ptr->index,buf);
			//sscanf(buf,"%s",ptr->name);
			strncpy(ptr->name,buf,strlen(buf));
			ptr->tag = 1 ; // 1 represent write by master
			ptr = list_next_entry(ptr,entry);
			a = 0 ;
		}

	} else { //slave

		if(ptr->index!=0) { // if list not full
			printk("slave write node%d : %s\n",ptr->index,buf);
			//sscanf(buf,"%s",ptr->name);
			strncpy(ptr->name,buf,strlen(buf));
			ptr->tag = 2 ; // -1 represent write by slave
			ptr = list_next_entry(ptr,entry);
			a = 0 ;
		}
	}

	spin_unlock_irqrestore(&a_lock,flag);//釋放lock


//	sscanf(buf, "%s",tmp->name);
//    ptr = list_entry(ptr,struct mailbox_entry_t,entry);//指向目前node
//    ptr = ptr->entry.next;
//    ptr = bufferhead.next;

	/*    sscanf(buf, "%s",ptr->name);
	if(ptr->entry.next!=NULL)
	    ptr = ptr->entry.next;*/
//	printk("write\n");
//	sscanf(buf, "%s",box);
//  box = buf;

	//return 0;
	return a;
}

static int __init mailbox_init(void)
{
	printk("Insert\n");
	hw2_kobject = kobject_create_and_add("hw2", kernel_kobj);
	sysfs_create_file(hw2_kobject, &mailbox_attribute.attr);
	printk("max:%d\n",num_entry_max);
//  box = kmalloc(100,GFP_USER);
	INIT_LIST_HEAD(&bufferhead.entry);
	for(int i = 0; i<num_entry_max; i++) {
		tmp= kmalloc(sizeof(struct mailbox_entry_t),GFP_KERNEL) ;
		tmp->count = i+8;//可以comment掉
		tmp->index = i+1;
		printk("%d ",tmp->count);
		ptr = tmp;
		list_add(&tmp->entry, &bufferhead.entry);
	}

	printk("\n");
	struct list_head *listptr;
	struct mailbox_entry_t *entr;

	list_for_each(listptr, &bufferhead.entry) {
		entr = list_entry(listptr, struct mailbox_entry_t, entry);
		printk(KERN_ALERT"no = %d (list %p, prev = %p, next = %p)\n",entr->count,
		       &entr->entry, entr->entry.prev, entr->entry.next);
	}

	entr = list_next_entry(entr,entry);
//    entr = list_next_entry(entr,entry);
//  entr = list_prev_entry(entr,entry);
	int count1 = entr->count;
	ptr = entr;
	printk("%d",count1);
	int k;
	for(k=0; k<=num_entry_max; k++) {
		ptr = list_next_entry(ptr,entry);
		printk(" ptr:%d ",ptr->index);
		printk("\n");
	}
	//initilize完ptr指向head's next node
	ptr = list_next_entry(ptr,entry);//ptr point to node1(head's next node)
}

static void __exit mailbox_exit(void)
{
	printk("Remove\n");
	kobject_put(hw2_kobject);
}

module_init(mailbox_init);
module_exit(mailbox_exit);
