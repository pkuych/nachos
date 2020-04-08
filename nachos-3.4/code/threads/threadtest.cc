// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "elevatortest.h"
#include "synch.h"
#include "system.h"

// testnum is set in main.cc
int testnum = 1;

// int buffer[10];
// int items = 0;
// int space = 10;
// Lock *lock = new Lock("lock"); 
// Condition *isFull = new Condition("full");
// Condition *isEmpty = new Condition("empty");

// int buffer[10];
// int i = 0;
// Semaphore *full = new Semaphore("full",0);
// Semaphore *empty = new Semaphore("empty", 10);
// Semaphore *mutex = new Semaphore("mutex", 1);

// void producer(int n) {
// 	int item;
// 	while (n--) {
// 		lock->Acquire();
// 		while (space == 0) {
// 			printf("Buffer is full, the producer will sleep!\n");
// 			isFull->Wait(lock);
// 		}
// 		item = Random() % 10;
// 		buffer[items] = item;
// 		items++;
// 		space--;
// 		printf("producer add %d in buffer, buffer size: %d\n", item, items);
// 		if (items == 1) {
// 			isEmpty->Signal(lock);
// 		}
// 		lock->Release();
// 	}
// }

// void producer(int n) {
// 	int item;
// 	while(n--) {
// 		empty->P();
// 		mutex->P();
// 		item = Random() % 10;
// 		buffer[i] = item;
// 		i++;
// 		printf("producer add %d in buffer, buffer size: %d\n", item, i);
// 		mutex->V();
// 		full->V();
// 	}
// }

// void consumer(int n) {
// 	int item;
// 	while (n--)
// 	{
// 		lock->Acquire();
// 		while (items == 0) {
// 			printf("Buffer is empty, the consumer will sleep!\n");
// 			isEmpty->Wait(lock);
// 		}
// 		item = buffer[items - 1];
// 		items--;
// 		space++;
// 		printf("consumer remove %d in buffer, buffer size: %d\n", item, items);
// 		if (space == 1) {
// 			isFull->Signal(lock);
// 		}
// 		lock->Release();
// 	}
// }

// void consumer(int n) {
// 	int item;
// 	while (n--) {
// 		full->P();
// 		mutex->P();
// 		item = buffer[i];
// 		i--;
// 		printf("consumer remove %d in buffer, buffer size: %d\n", item, i);
// 		mutex->V();
// 		empty->V();
// 	}
// }

// void producer_consumer(){
// 	Thread *t1 = new Thread("producer");
// 	Thread *t2 = new Thread("consumer");
// 	t1->Fork(producer, (void*)15);
// 	t2->Fork(consumer, (void*)20);
// }

//---------------------------------------------------------------------
// 读写锁测试
//---------------------------------------------------------------------
Rwlock *rwlock;

int buffer[20] = {0};
int i = 0;

void writer(int n){
	int item;
	while(n--){
		rwlock->RwAcquire();
		item = Random() % 10;
		buffer[i++] = item;
		printf("%s write %d in buffer(size:%d)\n", currentThread->getName(), item, i); 
		if (n == 1) {
			printf("%s Yield!\n", currentThread->getName());
			currentThread->Yield();
		}
		rwlock->RwRelease();
	}
}

void reader(int n){
	int item;
	while (n--)
	{
		rwlock->RdAcquire();
		item = buffer[Random() % 20];
		printf("%s read %d from buffer(size:%d)\n", currentThread->getName(), item, i);
		if (n == 1) {
			printf("%s Yield\n", currentThread->getName());
			currentThread->Yield();
		}
		rwlock->RdRelease();
	}
	
}

void rwlockTest() {
	rwlock = new Rwlock("rwlock");
	Thread *t1 = new Thread("writer1");
	Thread *t2 = new Thread("reader1");
	Thread *t3 = new Thread("writer2");
	Thread *t4 = new Thread("reader2");
	t1->Fork(writer, (void*)5);
	t2->Fork(reader, (void*)5);
	t3->Fork(writer, (void*)5);
	t4->Fork(reader, (void*)5);
}

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void SimpleThread(int which) {
	int num;

	for (num = 0; num < 5; num++) {
		printf("*** userId: %d threadId %d looped %d times\n",
			   currentThread->getUid(), currentThread->getThreadId(), num);
		currentThread->Yield();
	}
}

//----------------------------------------------------------------------
// PrintThreads
// print threads in readylist and currentThread
//
// "n" is because Fork need an arg
//----------------------------------------------------------------------

void PrintThreads(int n) {
	if (n == 10) {
    	scheduler->Print();			//output readyList
		currentThread->Print();		//output currentThread
	}
	currentThread->Yield();
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void ThreadTest1() {
	DEBUG('t', "Entering ThreadTest1");

	Thread *t = new Thread("forked thread");

	t->Fork(SimpleThread, (void *)1);
	SimpleThread(0);
}

//-------------------------------------------------------------------------
// ThreadTest2
// new 150 Threads then print tid uid
//-------------------------------------------------------------------------

void ThreadTest2() {
	DEBUG('t', "Entering ThreadTest2");

	for (int i = 0; i < 150; i++) {
		Thread *t = new Thread("forked thread");
		printf("ThreadId %d userId %d\n", t->getThreadId(), t->getUid());
	}
}

//----------------------------------------------------------------------
// TS
// output threads in readylist and currentthread
//----------------------------------------------------------------------

void TS() {
    DEBUG('t', "Entering TS");
    
    for (int i = 1; i <= 10; i++) {
        Thread *t = new Thread("TS thread");
        t->Fork(PrintThreads, (void *)i);
    }
	PrintThreads(0);
}


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void ThreadTest() {
	switch (testnum) {
	case 1:
		ThreadTest1();
		break;
	case 2:
		ThreadTest2();
		break;
	case 3:
		// producer_consumer();
		break;
	case 4:
		rwlockTest();
		break;
	case 100:
        TS();
        break;
	default:
		printf("No test specified.\n");
		break;
	}
}
