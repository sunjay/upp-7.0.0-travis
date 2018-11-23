//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1997
// 
// uDefault.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Mar 20 18:12:31 1997
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Dec 23 09:08:11 2017
// Update Count     : 61
//
// This  library is free  software; you  can redistribute  it and/or  modify it
// under the terms of the GNU Lesser General Public License as published by the
// Free Software  Foundation; either  version 2.1 of  the License, or  (at your
// option) any later version.
// 
// This library is distributed in the  hope that it will be useful, but WITHOUT
// ANY  WARRANTY;  without even  the  implied  warranty  of MERCHANTABILITY  or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
// for more details.
// 
// You should  have received a  copy of the  GNU Lesser General  Public License
// along  with this library.
// 


#ifndef __U_DEFAULT_H__
#define __U_DEFAULT_H__


// Define the default extension heap amount in units of bytes. When the uC++ supplied heap reaches the brk address, the
// brk address is extended by the extension amount.

#define __U_DEFAULT_HEAP_EXPANSION__ (1 * 1024 * 1024)


// Define the mmap crossover point during allocation. Allocations less than this amount are allocated from buckets;
// values greater than or equal to this value are mmap from the operating system.

#define __U_DEFAULT_MMAP_START__ (512 * 1024 + 1)


// Define the default scheduling pre-emption time in milliseconds.  A scheduling pre-emption is attempted every default
// pre-emption milliseconds.  A pre-emption does not occur if the executing task is not in user code or the task is
// currently in a critical section.  A critical section begins when a task acquires a lock and ends when a user releases
// a lock.

#define __U_DEFAULT_PREEMPTION__ 10


// Define the default spin time in units of checks and context switches. The idle task checks the ready queue and
// context switches this many times before the UNIX process executing the idle task goes to sleep.

#define __U_DEFAULT_SPIN__ 1000


// Define the default stack size in bytes.  Change the implicit default stack size for a task or coroutine created on a
// particular cluster.

#if defined( __ia64__ ) || defined( __sparc__ )
// needed by IA64 unwind library and by SPARC for large stack-frames
//#define __U_DEFAULT_STACK_SIZE__ 60000
#define __U_DEFAULT_STACK_SIZE__ 240000
#else
//#define __U_DEFAULT_STACK_SIZE__ 30000
#define __U_DEFAULT_STACK_SIZE__ 240000
#endif

// often large automatic arrays for setting up the program
#define __U_DEFAULT_MAIN_STACK_SIZE__ 500000


// Define the default number of processors created on the user cluster. Must be greater than 0.

#define __U_DEFAULT_USER_PROCESSORS__ 1


// Define the default number of executor request-queues (mailboxes) written to by actors and serviced by the
// actor-executor threads. Must be greater than 0.

#define __U_DEFAULT_ACTOR_MAILBOXES__ 4

// Define the default number of threads created in the actor executor. Must be greater than 0.

#define __U_DEFAULT_ACTOR_THREADS__ 4

// Define the default number of processors created in the actor executor. Must be greater than 0.

#define __U_DEFAULT_ACTOR_PROCESSORS__ 2

// Define affinity for actor executor kernel threads and the offset from CPU 0 to start binding. -1 implies no affinity.

#define __U_DEFAULT_ACTOR_OFFSET__ -1

// Define if actor executor is created in a separate cluster

#define __U_DEFAULT_ACTOR_SEPCLUS__ false

extern unsigned int uDefaultHeapExpansion();		// heap expansion size (bytes)
extern unsigned int uDefaultMmapStart();		// cross over point to use mmap rather than buckets
extern unsigned int uDefaultStackSize();		// cluster coroutine/task stack size (bytes)
extern unsigned int uMainStackSize();			// uMain task stack size (bytes)
extern unsigned int uDefaultSpin();			// processor spin time for idle task (context switches)
extern unsigned int uDefaultPreemption();		// processor scheduling pre-emption durations (milliseconds)
extern unsigned int uDefaultProcessors();		// number of processors created on the user cluster
extern unsigned int uDefaultBlockingIOProcessors();	// number of blocking I/O processors created on the blocking I/O cluster

extern unsigned int uDefaultActorMailboxes();		// executor request queues
extern unsigned int uDefaultActorThreads();		// user threads servicing executor thread-pool
extern unsigned int uDefaultActorProcessors();		// kernel threads (processors) servicing executor thread-pool
extern int uDefaultActorOffset();			// affinity and offset (-1 => no affinity, default)
extern bool uDefaultActorSepClus();			// create processors on separate cluster

extern void uStatistics();				// print user defined statistics on interrupt


#endif // __U_DEFAULT_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
