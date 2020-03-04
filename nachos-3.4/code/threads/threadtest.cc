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
#include "system.h"

// testnum is set in main.cc
int testnum = 1;

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
	case 100:
        TS();
        break;
	default:
		printf("No test specified.\n");
		break;
	}
}
