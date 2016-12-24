#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include "mythread.h"
#define MEMORY 1024*16

int thread_count=0;
 typedef struct Thread 
 {
 	ucontext_t context;
 	struct Thread* parent;
 	struct Thread* seq; 
 	struct Queue* child;
}Thread; 
Thread* Running;


ucontext_t initial_context, exit_context;


 typedef struct Queue_node
{
	  Thread* thread;
	   struct Queue_node* next;
} Queue_node;


typedef struct Queue
{
Queue_node* front;
Queue_node* rear;

} Queue;
Queue* Q_ready;
Queue* Q_blocked;

typedef struct Semaphore
{
int initialvalue, currentvalue;
Queue* Q_sem;
}Semaphore;


void initalize_queue(Queue* Queue)
{
  Queue->front==NULL;
  Queue->rear==NULL;

}
void enqueue(Queue* Queue,Thread* newthread)
{

 Queue_node* temp = (Queue_node*)malloc(sizeof(Queue_node));
temp-> thread = newthread;
temp-> next = NULL;

if(Queue->front==NULL && Queue->rear==NULL)
{
 Queue->front=temp;
 Queue->rear=temp;
}
else
{
	Queue->rear->next=temp;
	Queue->rear=temp;
}
}
 Thread* dequeue(Queue* Queue)
{
 Queue_node* temp= NULL;
temp=Queue->front;
if(Queue->front==Queue->rear)
{
	if(temp==NULL)
		return NULL;
 Queue->front=NULL;
 Queue->rear=NULL;

}
else if(Queue->front==NULL)
{
 return 0;
}
else
{
	Queue->front= Queue->front->next;
}
return temp->thread;
}

int empty(Queue* Queue)
{

	if(Queue->front==NULL && Queue->rear==NULL)
	{
		return 0;
	}
	else
		{return 1;}
}
int search(Queue *Q, Thread *t)
{
	if (Q->front==NULL)
	{
		return 0;
	}
	Queue_node* temp;
	temp=Q->front;
	while(1)
	{
		if (temp->thread==t)
		{
			return 1;
		}
		if (temp->next==NULL)
		{
			return 0;
		}
		temp=temp->next;
	}
}


MySemaphore MySemaphoreInit(int initialvalue) 
{

Semaphore* s = (Semaphore*)malloc(sizeof(Semaphore));
s->initialvalue=initialvalue;

 s-> Q_sem = (Queue*)malloc(sizeof(Queue));
 initalize_queue(s-> Q_sem);
 return (MySemaphore)s;
}

void MySemaphoreSignal(MySemaphore sem)
{
  Semaphore* s;
  s=(Semaphore*)sem;
if(s->currentvalue<=s->initialvalue)
{
(s->currentvalue)++;
Thread* t= dequeue(s->Q_sem);
enqueue( Q_ready, t);
}
}
void MySemaphoreWait(MySemaphore sem)
{
   Semaphore* s;
   s=(Semaphore*)sem;
   (s->currentvalue)--;
   if(s->currentvalue<0){
    enqueue(s->Q_sem, Running);
    Thread* th= dequeue(Q_ready);
    Thread* temp= Running;
    Running=th;
    swapcontext(&temp->context,&th->context);
 }
}
int MySemaphoreDestroy(MySemaphore sem)
{
	Semaphore* s;
	s=(Semaphore*)sem;
	if(empty(s->Q_sem)==0)
	{
        return -1;
	}
	else
	{    s->Q_sem==NULL;
		return 0;
	}

}



void MyThreadInit (void(*start_funct)(void *), void *args) 
{	
    if(thread_count>0){return;}
    thread_count++;
	Q_ready = (Queue*)malloc(sizeof(Queue));
	Q_blocked = (Queue*)malloc(sizeof(Queue));
	
	initalize_queue(Q_ready);
    initalize_queue(Q_blocked);	
    

    getcontext(&initial_context);
   
 	Thread* main_thread;
 	main_thread= (Thread*)malloc(sizeof(Thread));
 	getcontext(&main_thread->context);
 	main_thread->context.uc_link=0; 
    main_thread->context.uc_stack.ss_sp=malloc(MEMORY);
    main_thread->context.uc_stack.ss_size=MEMORY;
 	main_thread->context.uc_stack.ss_flags=0;
 	main_thread-> parent=NULL;
 	main_thread-> seq=NULL;
 	main_thread->child=(Queue*)malloc(sizeof(Queue));
 	initalize_queue(main_thread->child);
 	makecontext(&main_thread->context,(void(*)())start_funct,1,args);
 	Running= (Thread*)malloc(sizeof(Thread));
 	Running=main_thread;
 	swapcontext(&initial_context,&main_thread->context);
 }

 MyThread MyThreadCreate (void(*start_funct)(void *), void *args)
 {
   Thread* newthread;

 	newthread= (Thread*)malloc(sizeof(Thread));

 	 getcontext(&newthread->context);
 	newthread->context.uc_link=0; 
    newthread->context.uc_stack.ss_sp=malloc(MEMORY);
    newthread->context.uc_stack.ss_size=MEMORY;
 	newthread->context.uc_stack.ss_flags=0;
 	newthread->parent=Running;
 	newthread->seq=NULL;
 	newthread->child=(Queue*)malloc(sizeof(Queue));
 	initalize_queue(newthread->child);
 	enqueue(Running->child,newthread);
 	enqueue(Q_ready,newthread);
  makecontext(&newthread->context,(void(*)())start_funct,1,args);
    return (MyThread)newthread;
 }

 void MyThreadYield(void)
 {
 	Thread* newthread;
 newthread=Running;
 if(empty(Q_ready)==0){
 	return;
 }
 enqueue(Q_ready, Running);
 
  Running= dequeue(Q_ready);
 
  if(Running==NULL)
  	return;
 getcontext(&newthread->context);
 swapcontext(&newthread->context,&Running->context);

 }

int MyThreadJoin(MyThread thread)
{
	Thread* given_thread;
	given_thread=(Thread *)thread;

	if(given_thread->parent==Running)
	{
      if (search(Running->child,given_thread)==1)
      {

      	
      	Running->seq=given_thread;

 		 enqueue(Q_blocked, Running);
      Thread* newthread;
        newthread=Running;
  		Running= dequeue(Q_ready);
  		
    
 		getcontext(&newthread->context);
 		swapcontext(&newthread->context,&Running->context);

      }
      else
      {
      	//printf("The child has already terminated");
      }
      return 0;
	}
	else{
		return -1;
	}
}
void MyThreadJoinAll(void)
{
    
if(empty(Running->child)){
//    printf("The child is not present");	
  }
else{
	    Thread* newthread;

 		enqueue(Q_blocked, Running);
  		Running= dequeue(Q_ready);
 		getcontext(&newthread->context);
 		swapcontext(&newthread->context,&Running->context);
}
}


void MyThreadExit(void)
{ 
	Thread* copy;
copy=Running;
Queue_node* q;
if(Running==NULL)
  return;

q=copy->child->front;
if (Q_blocked==NULL)
{
	return;
}
if(Running->parent!=NULL && empty(Q_blocked)==1 && search(Q_blocked,Running->parent)==1)
{
  copy=dequeue(Running->parent->child);
if(empty(Running->parent->child)==0 || (Running->parent->seq==copy))
{
	Running= dequeue(Q_blocked);
	enqueue(Q_ready,Running);
	free(Running);
}    
}

if (empty(Q_ready)==0)
{
	setcontext(&initial_context);
	return;
}

Running=dequeue(Q_ready);
if (Running==NULL)
{
  
	return;
}

if (q!=NULL)
{
while(q->next!=NULL)
{
	q->thread->parent=NULL;
	q=q->next;
}
}

setcontext(&Running->context);

}