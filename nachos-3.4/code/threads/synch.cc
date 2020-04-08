// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "synch.h"
#include "copyright.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char *debugName, int initialValue) {
	name = debugName;
	value = initialValue;
	queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore() { delete queue; }

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void Semaphore::P() {
	IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts

	while (value == 0) {					  // semaphore not available
		queue->Append((void *)currentThread); // so go to sleep
		currentThread->Sleep();
	}
	value--; // semaphore available,
			 // consume its value

	(void)interrupt->SetLevel(oldLevel); // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void Semaphore::V() {
	Thread *thread;
	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	thread = (Thread *)queue->Remove();
	if (thread != NULL) // make thread ready, consuming the V immediately
		scheduler->ReadyToRun(thread);
	value++;
	(void)interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
/**
 * 锁的构造函数 初始化一个二元信号量
 * @param debugName 
 */
Lock::Lock(char *debugName) {
	name = debugName;
	lock = new Semaphore(debugName, 1);
}

Lock::~Lock() {
	delete lock;
}
/**
 * 线程获得锁 需要是原子操作
 */ 
void Lock::Acquire() {
	IntStatus oldvalue = interrupt->SetLevel(IntOff); //关中断
	lock->P();
	owner = currentThread;
	(void)interrupt->SetLevel(oldvalue);	//恢复中断
}
/**
 * 释放锁 需要是原子操作
 */ 
void Lock::Release() {
	IntStatus oldvalue = interrupt->SetLevel(IntOff);
	// 只有获得锁的线程可以释放锁
	ASSERT(currentThread == owner);
	lock->V();
	owner = NULL;
	(void)interrupt->SetLevel(oldvalue);
}

bool Lock::isHeldByCurrentThread() {
	return currentThread == owner;
}

Rwlock::Rwlock(char* debugName) {
	name = debugName;
	r_lock = new Lock("read");
	w_lock = new Semaphore("write", 1);
	readers = 0;
}

Rwlock::~Rwlock() {
	delete r_lock;
	delete w_lock;
}

void Rwlock::RwAcquire(){
	w_lock->P();
	printf("%s acquire w_lock...\n", currentThread->getName());
}

void Rwlock::RwRelease(){
	w_lock->V();
	printf("%s release w_lock...\n", currentThread->getName());
}

void Rwlock::RdAcquire(){
	r_lock->Acquire();
	if (readers == 0) {
		w_lock->P();
	}
	readers++;
	r_lock->Release();
	printf("%s acquire r_lock...\n", currentThread->getName());
}

void Rwlock::RdRelease(){
	r_lock->Acquire();
	readers--;
	if (readers == 0) {
		w_lock->V();
	}
	r_lock->Release();
	printf("%s release r_lock...\n", currentThread->getName());
}

bool Rwlock::isHeldByCurrentThread(){

}

Condition::Condition(char *debugName) {
	name = debugName;
	queue = new List;
}
Condition::~Condition() {
	delete queue;
}
/**
 * release the lock, relinquish the CPU until signaled, then re-acquire the lock 
 */
void Condition::Wait(Lock *conditionLock) { 
	//ASSERT(FALSE);
	IntStatus oldvalue = interrupt->SetLevel(IntOff);
	ASSERT(conditionLock->isHeldByCurrentThread());	//	必须是当前线程持有的锁
	conditionLock->Release();						// 这里不判断也可 因为release函数有判断
	queue->Append(currentThread);					// 当前线程加入条件的等待队列
	currentThread->Sleep();							
	conditionLock->Acquire();						// 被唤醒后 重新获得锁

	(void)interrupt->SetLevel(oldvalue);
}
/**
 * wake up a thread, if there are any waiting on the condition
 */ 
void Condition::Signal(Lock *conditionLock) {
	IntStatus oldvalue = interrupt->SetLevel(IntOff);
	ASSERT(conditionLock->isHeldByCurrentThread());
	if (!queue->IsEmpty()){		//通知一个等待的线程 从等待队列移除 加入就绪队列
		Thread *thread = (Thread *)queue->Remove();
		scheduler->ReadyToRun(thread);
	}
	(void)interrupt->SetLevel(oldvalue);
}
/**
 * wake up all threads waiting on the condition
 */
void Condition::Broadcast(Lock *conditionLock) {
	Thread *thread;
	IntStatus oldvalue = interrupt->SetLevel(IntOff);
	ASSERT(conditionLock->isHeldByCurrentThread());
	while (!queue->IsEmpty()) {		//通知所有等待的线程
		thread = (Thread *)queue->Remove();
		scheduler->ReadyToRun(thread);
	}
	(void)interrupt->SetLevel(oldvalue);
}
