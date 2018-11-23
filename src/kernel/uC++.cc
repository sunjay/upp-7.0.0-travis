//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uC++.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Dec 17 22:10:52 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Sep  8 16:01:43 2018
// Update Count     : 3139
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


#define __U_KERNEL__
#include <uC++.h>
#ifdef __U_PROFILER__
#include <uProfiler.h>
#endif // __U_PROFILER__
#include <uBootTask.h>
#include <uSystemTask.h>
#include <uFilebuf.h>
#ifdef __U_STATISTICS__
#include <uHeapLmmm.h>
#endif // __U_STATISTICS__

#include <uDebug.h>					// access: uDebugWrite
#undef uDEBUGPRT					// turn off debug prints
#define uDEBUGPRT( stmt )

#include <iostream>
#include <exception>
#include <dlfcn.h>
#include <cstdio>
#include <unistd.h>					// _exit
#if defined( __solaris__ )
#include <ieeefp.h>					// floating-point exceptions
#else
#include <fenv.h>					// floating-point exceptions
#endif // __solaris__

#if defined( __ia64__ )
#include <ia64intrin.h>					// __sync_lock_release
#endif // __ia64__


using namespace UPP;


#ifdef __U_STATISTICS__
// Kernel, signed because of the atomic inc/dec
int Statistics::ready_queue = 0, Statistics::spins = 0, Statistics::spin_sched = 0, Statistics::mutex_queue = 0,
    Statistics::owner_lock_queue = 0, Statistics::adaptive_lock_queue = 0, Statistics::io_lock_queue = 0,
    Statistics::uSpinLocks = 0, Statistics::uLocks = 0, Statistics::uOwnerLocks = 0, Statistics::uCondLocks = 0, Statistics::uSemaphores = 0, Statistics::uSerials = 0;

// I/O statistics
unsigned int Statistics::select_syscalls = 0, Statistics::select_errors = 0, Statistics::select_eintr = 0;
unsigned int Statistics::select_events = 0, Statistics::select_nothing = 0, Statistics::select_blocking = 0, Statistics::select_pending = 0;
unsigned int Statistics::select_maxFD = 0;
unsigned int Statistics::accept_syscalls = 0, Statistics::accept_errors = 0;
unsigned int Statistics::read_syscalls = 0, Statistics::read_errors = 0, Statistics::read_eagain = 0, Statistics::read_chunking = 0, Statistics::read_bytes = 0;
unsigned int Statistics::write_syscalls = 0, Statistics::write_errors = 0, Statistics::write_eagain = 0, Statistics::write_bytes = 0;
unsigned int Statistics::sendfile_syscalls = 0, Statistics::sendfile_errors = 0, Statistics::sendfile_eagain = 0, Statistics::first_sendfile = 0, Statistics::sendfile_yields = 0;

unsigned int Statistics::iopoller_exchange = 0, Statistics::iopoller_spin = 0;
unsigned int Statistics::signal_alarm = 0, Statistics::signal_usr1 = 0;

// Scheduling statistics
unsigned int Statistics::roll_forward = 0;
unsigned int Statistics::user_context_switches = 0;
unsigned int Statistics::kernel_thread_yields = 0, Statistics::kernel_thread_pause = 0;
unsigned int Statistics::wake_processor = 0;
unsigned int Statistics::events = 0, Statistics::setitimer = 0;

// Print statistics
bool Statistics::prtStatTerm_ = false;

void UPP::Statistics::print() {
    uStatistics();					// user specified statistics

    char helpText[512];
    int len;

    len = snprintf( helpText, 512,
		    "\nKernel statistics:\n"
		    "  locks:"
		    " spinlocks %d"
		    " / spins %d"
		    " / schedules %d"
		    " / uLocks %d"
		    " / uOwnerLocks %d"
		    " / uCondLocks %d"
		    " / uSemaphores %d"
		    " / uSerials %d\n"
		    "  signal:"
		    " alarm %d"
		    " / usr1 %d\n",
		    Statistics::uSpinLocks,
		    Statistics::spins,
		    Statistics::spin_sched,
		    Statistics::uLocks,
		    Statistics::uOwnerLocks,
		    Statistics::uCondLocks,
		    Statistics::uSemaphores,
		    Statistics::uSerials,
		    Statistics::signal_alarm,
		    Statistics::signal_usr1 );
    uDebugWrite( STDOUT_FILENO, helpText, len );

    len = snprintf( helpText, 512,
		    "\nI/O statistics:\n"
		    "  select:"
		    " calls %d"
		    " / errors %d"
		    " (EINTR %d)"
		    " / select events %d"
		    " / no events %d"
		    " / events per call %d"
		    " / blocking %d"
		    " / max fd %d\n"
		    "  accept:"
		    " calls %d"
		    " / errors %d\n",
		    Statistics::select_syscalls,
		    Statistics::select_errors,
		    Statistics::select_eintr,
		    Statistics::select_events,
		    Statistics::select_nothing,
		    (Statistics::select_syscalls != 0 ? Statistics::select_events / Statistics::select_syscalls : 0 ),
		    Statistics::select_blocking,
		    Statistics::select_maxFD,
		    Statistics::accept_syscalls,
		    Statistics::accept_errors );
    uDebugWrite( STDOUT_FILENO, helpText, len );

    len = snprintf( helpText, 512,
		    "  read:"
		    " calls %d"
		    " / errors %d"
		    " / eagain %d"
		    " / chunking %d"
		    " / bytes %d\n"
		    "  write:"
		    " calls %d"
		    " / errors %d"
		    " / eagain %d"
		    " / bytes %d\n",
		    Statistics::read_syscalls,
		    Statistics::read_errors,
		    Statistics::read_eagain,
		    Statistics::read_chunking,
		    Statistics::read_bytes,
		    Statistics::write_syscalls,
		    Statistics::write_errors,
		    Statistics::write_eagain,
		    Statistics::write_bytes );
    uDebugWrite( STDOUT_FILENO, helpText, len );

    len = snprintf( helpText, 512,
		    "  sendfile:"
		    " calls %d"
		    " / errors %d"
		    " / eagain %d"
		    " / yields %d"
		    " / first call completion %d\n"
		    "  iopoller:"
		    " exchanges %d"
		    " / spins %d\n",
		    Statistics::sendfile_syscalls,
		    Statistics::sendfile_errors,
		    Statistics::sendfile_eagain,
		    Statistics::sendfile_yields,
		    Statistics::first_sendfile,
		    Statistics::iopoller_exchange,
		    Statistics::iopoller_spin );
    uDebugWrite( STDOUT_FILENO, helpText, len );

    len = snprintf( helpText, 512,
		    "\nScheduler statistics:\n"
		    "  roll forward: %d\n"
		    "  user context switches: %d\n"
		    "  kernel thread: yields %d"
		    " / pause %d"
		    " / processor wake %d\n"
		    "  events %d"
		    " / setitimer %d\n",
		    Statistics::roll_forward,
		    Statistics::user_context_switches,
		    Statistics::kernel_thread_yields,
		    Statistics::kernel_thread_pause,
		    Statistics::wake_processor,
		    Statistics::events,
		    Statistics::setitimer );
    uDebugWrite( STDOUT_FILENO, helpText, len );
} // UPP::Statistics::print
#endif // __U_STATISTICS__


bool uKernelModule::kernelModuleInitialized = false;
uDEBUG( bool uKernelModule::initialized = false; )
bool uKernelModule::coreDumped = false;
#ifndef __U_MULTI__
bool uKernelModule::deadlock = false;
#endif // ! __U_MULTI__
bool uKernelModule::globalAbort = false;
bool uKernelModule::globalSpinAbort = false;
uSpinLock *uKernelModule::globalAbortLock = nullptr;
uSpinLock *uKernelModule::globalProcessorLock = nullptr;
uSpinLock *uKernelModule::globalClusterLock = nullptr;
uDefaultScheduler *uKernelModule::systemScheduler = nullptr;
uCluster *uKernelModule::systemCluster = nullptr;
uProcessor *uKernelModule::systemProcessor = nullptr;
UPP::uBootTask *uKernelModule::bootTask = (uBootTask *)&bootTaskStorage;
uSystemTask *uKernelModule::systemTask = nullptr;
uCluster *uKernelModule::userCluster = nullptr;
uProcessor **uKernelModule::userProcessors = nullptr;
unsigned int uKernelModule::numUserProcessors = 0;

unsigned int uKernelModule::attaching = 0; // debugging

char uKernelModule::systemClusterStorage[sizeof(uCluster)] __attribute__(( aligned (128) ));
char uKernelModule::systemProcessorStorage[sizeof(uProcessor)] __attribute__(( aligned (16) ));
char uKernelModule::bootTaskStorage[sizeof(uBootTask)] __attribute__(( aligned (16) ));

std::filebuf *uKernelModule::cerrFilebuf = nullptr, *uKernelModule::clogFilebuf = nullptr, *uKernelModule::coutFilebuf = nullptr, *uKernelModule::cinFilebuf = nullptr;

// Fake uKernelModule used before uKernelBoot::startup.
volatile __U_THREAD__ uKernelModule::uKernelModuleData uKernelModule::uKernelModuleBoot;

uProcessorSeq *uKernelModule::globalProcessors = nullptr;
uClusterSeq *uKernelModule::globalClusters = nullptr;

bool uKernelModule::afterMain = false;
int uKernelModule::retCode = 0;

size_t uMachContext::pageSize;				// architecture pagesize
#if defined( __i386__ ) || defined( __x86_64__ )
uint16_t uMachContext::fncw;				// floating/MMX control masks
uint32_t uMachContext::mxcsr;
#endif // __i386__ || __x86_64__

#ifdef __U_FLOATINGPOINTDATASIZE__
int uFloatingPointContext::uniqueKey = 0;
#endif // __U_FLOATINGPOINTDATASIZE__

bool uHeapControl::traceHeap_ = false;
bool uHeapControl::prtHeapTerm_ = false;
bool uHeapControl::prtFree_ = false;

#define __U_TIMEOUTPOSN__ 0				// bit 0 is reserved for timeout
#define __U_DESTRUCTORPOSN__ 1				// bit 1 is reserved for destructor

#ifndef __U_MULTI__
uNBIO * uCluster::NBIO = nullptr;
#endif // ! __U_MULTI__

int UPP::uKernelBoot::count = 0;
int UPP::uInitProcessorsBoot::count = 0;


extern "C" void pthread_deletespecific_( void * );	// see pthread simulation
extern "C" void pthread_delete_kernel_threads_();
extern "C" void pthread_pid_destroy_( void );


//######################### main #########################


// The main routine that gets the first task started with the OS supplied arguments, waits for its completion, and
// returns the result code to the OS.  Define in this translation unit so it cannot be replaced by a user.

int uCpp_real_main( int argc, char *argv[], char *env[] ) {
    {
	uMain uUserMain( argc, argv, env, uKernelModule::retCode ); // created on user cluster
    }

    uKernelModule::afterMain = true;

    // Return the program return code to the operating system.

    return uKernelModule::retCode;
} // uCpp_real_main


//######################### Abnormal Event Handling #########################


uKernelFailure::uKernelFailure( const char *const msg ) { setMsg( msg ); }

uKernelFailure::~uKernelFailure() {}

void uKernelFailure::defaultTerminate() const {
#   define uKernelFailureSuffixMsg "Unhandled exception of type "
    char msg[sizeof(uKernelFailureSuffixMsg) - 1 + uEHMMaxName];
    strcpy( msg, uKernelFailureSuffixMsg );
    uEHM::getCurrentEventName( uBaseEvent::ThrowRaise, msg + sizeof(uKernelFailureSuffixMsg) - 1, uEHMMaxName );
    abort( "%s%s%s", msg, strlen( message() ) == 0 ? "" : " : ", message() );
} // uKernelFailure::defaultTerminate


uMutexFailure::uMutexFailure( const UPP::uSerial *const serial, const char *const msg ) : uKernelFailure( msg ), serial( serial ) {}

uMutexFailure::uMutexFailure( const char *const msg ) : uKernelFailure( msg ), serial( nullptr ) {}

const UPP::uSerial *uMutexFailure::serialId() const { return serial; }


uMutexFailure::EntryFailure::EntryFailure( const UPP::uSerial *const serial, const char *const msg ) : uMutexFailure( serial, msg ) {}

uMutexFailure::EntryFailure::EntryFailure( const char *const msg ) : uMutexFailure( msg ) {}

uMutexFailure::EntryFailure::~EntryFailure() {}

void uMutexFailure::EntryFailure::defaultTerminate() const {
    if ( src == nullptr ) {				// raised synchronously ?
	abort( "(uSerial &)%p : Entry failure while executing mutex destructor: %.256s.",
		serialId(), message() );
    } else {
	abort( "(uSerial &)%p : Entry failure while executing mutex destructor: task %.256s (%p) found %.256s.",
		serialId(), sourceName(), &source(), message() );
    } // if
} // uMutexFailure::EntryFailure::defaultTerminate


uMutexFailure::RendezvousFailure::RendezvousFailure( const UPP::uSerial *const serial, const char *const msg ) : uMutexFailure( serial, msg ), caller_( &uThisCoroutine() ) {}

uMutexFailure::RendezvousFailure::~RendezvousFailure() {}

const uBaseCoroutine *uMutexFailure::RendezvousFailure::caller() const { return caller_; }

void uMutexFailure::RendezvousFailure::defaultTerminate() const {
    abort( "(uSerial &)%p : Rendezvous failure in %.256s from task %.256s (%p) to mutex member of task %.256s (%p).",
	    serialId(), message(), sourceName(), &source(), uThisTask().getName(), &uThisTask() );
} // uMutexFailure::RendezvousFailure::defaultTerminate


uIOFailure::uIOFailure( int errno__, const char *const msg ) {
    errno_ = errno__;
    setMsg( msg );
} // uIOFailure::uIOFailure

uIOFailure::~uIOFailure() {}

int uIOFailure::errNo() const {
    return errno_;
} // uIOFailure::errNo


//######################### uSpinLock #########################


void uBaseSpinLock::acquire_( bool rollforward __attribute__(( unused )) ) {
    // No race condition exists for accessing disableIntSpin in the multiprocessor case because this variable is private
    // to each UNIX process. Also, the spin lock must be acquired after adjusting disableIntSpin because the time
    // slicing must see the attempt to access the lock first to prevent live-lock on the same processor.  For example,
    // one task acquires the ready queue lock, a time slice occurs, and it does not appear that the current task is in
    // the kernel because disableIntSpin is not set so the signal handler tries to yield.  However, the ready queue lock
    // is held so the yield live-locks. There is a similar situation on releasing the lock.

    enum { SPIN_START = 4,
#if defined( __i386__ ) || defined( __x86_64__ )
	   SPIN_END = 4 * 1024,				// fewer iterations due to "pause" instruction throttling to memory speed
#else
	   SPIN_END = 64 * 1024,
#endif
    };

#if defined( __U_DEBUG__ ) && ! defined( __U_MULTI__ )
    if ( value != 0 ) {					// locked ?
	abort( "(uSpinLock &)%p.acquire() : internal error, attempt to multiply acquire spin lock by same task.", this );
    } // if
#endif // __U_DEBUG__ && ! __U_MULTI__

    THREAD_GETMEM( This )->disableIntSpinLock();

#ifdef __U_MULTI__
    int spin = SPIN_START;
    for ( ;; ) {					// poll for lock
      if ( value == 0 && uTestSet( value ) == 0 ) break;
	if ( rollforward ) {				// allow timeslicing during spinning
	    THREAD_GETMEM( This )->enableIntSpinLockNoRF();
	} else {
	    THREAD_GETMEM( This )->enableIntSpinLock();
	} // if

	for ( int i = 0; i < spin; i += 1 ) {		// exponential spin
#if defined( __i386__ ) || defined( __x86_64__ )
	    asm volatile( "pause" );
#endif
	    if ( uKernelModule::globalSpinAbort ) _exit( EXIT_FAILURE ); // close down in progress, shutdown immediately!
#ifdef __U_STATISTICS__
	    uFetchAdd( Statistics::spins, 1 );
#endif // __U_STATISTICS__
	} // for
	spin += spin;					// powers of 2
	if ( spin > SPIN_END ) {
	    spin = SPIN_START;				// prevent overflow
//	    sched_yield();				// release CPU so someone else can execute
#ifdef __U_STATISTICS__
	    uFetchAdd( Statistics::spin_sched, 1 );
#endif // __U_STATISTICS__
	} // if
	THREAD_GETMEM( This )->disableIntSpinLock();
    } // for

#if defined( __sparc__ )
    asm volatile ( "membar #LoadLoad" );		// flush the cache
#endif // __sparc__

#else
    value = 1;						// lock
#endif // __U_MULTI__
} // uBaseSpinLock::acquire_


bool uBaseSpinLock::tryacquire() {
#if defined( __U_DEBUG__ ) && ! defined( __U_MULTI__ )
    if ( value != 0 ) {					// locked ?
	abort( "(uSpinLock &)%p.tryacquire() : internal error, attempt to multiply acquire spin lock by same task.", this );
    } // if
#endif // __U_DEBUG__ && ! __U_MULTI__

    THREAD_GETMEM( This )->disableIntSpinLock();

#ifdef __U_MULTI__
    if ( uTestSet( value ) == 0 ) {			// get the lock ?
#if defined( __sparc__ )
	asm volatile ( "membar #LoadLoad" );		// flush the cache
#endif // __sparc__
	return true;
    } else {
	THREAD_GETMEM( This )->enableIntSpinLock();
	return false;
    } // if
#else
    value = 1;						// lock
    return true;
#endif // __U_MULTI__
} // uBaseSpinLock::tryacquire


//######################### uLock #########################


void uLock::acquire() {
    for ( ;; ) {
	spinLock.acquire();
      if ( value == 1 ) break;
	spinLock.release();
	uThisTask().yield();
    } // for
    value = 0;
    spinLock.release();
} // uLock::acquire


bool uLock::tryacquire() {
  if ( value == 0 ) return false;
    spinLock.acquire();
    if ( value == 0 ) {
	spinLock.release();
	return false;
    } else {
	value = 0;
	spinLock.release();
	return true;
    } // if
} // uLock::tryacquire


//######################### uOwnerLock #########################


void uOwnerLock::add_( uBaseTask &task ) {		// used by uCondLock::signal
    spinLock.acquire();
    if ( owner_ != nullptr ) {				// lock in use ?
	waiting.addTail( &(task.entryRef) );		// move task to owner lock list
    } else {
	owner_ = &task;					// become owner
	count = 1;
	task.wake();					// restart new owner
    } // if
    spinLock.release();
} // uOwnerLock::add_


void uOwnerLock::release_() {				// used by uCondLock::wait
    spinLock.acquire();
    if ( ! waiting.empty() ) {				// waiting tasks ?
	owner_ = &(waiting.dropHead()->task());		// remove task at head of waiting list and make new owner
	count = 1;
	owner_->wake();					// restart new owner
    } else {
	owner_ = nullptr;					// release, no owner
	count = 0;
    } // if
    spinLock.release();
} // uOwnerLock::release_


uDEBUG(
    uOwnerLock::~uOwnerLock() {
	spinLock.acquire();
	if ( ! waiting.empty() ) {
	    uBaseTask *task = &(waiting.head()->task());	// waiting list could change as soon as spin lock released
	    spinLock.release();
	    abort( "Attempt to delete owner lock with task %.256s (%p) still on it.", task->getName(), task );
	} // if
	spinLock.release();
    } // uOwnerLock::uOwnerLock
)


void uOwnerLock::acquire() {
    assert( uKernelModule::initialized ? ! THREAD_GETMEM( disableInt ) && THREAD_GETMEM( disableIntCnt ) == 0 : true );

    uBaseTask &task = uThisTask();			// optimization
    spinLock.acquire();
#ifdef KNOT
    task.setActivePriority( task.getActivePriorityValue() + 1 );
#endif // KNOT
    if ( owner_ != &task ) {				// don't own lock yet
	if ( owner_ != nullptr ) {				// but if lock in use
	    waiting.addTail( &(task.entryRef) );	// suspend current task
#ifdef __U_STATISTICS__
	    uFetchAdd( Statistics::owner_lock_queue, 1 );
#endif // __U_STATISTICS__
	    uProcessorKernel::schedule( &spinLock );	// atomically release owner spin lock and block
#ifdef __U_STATISTICS__
	    uFetchAdd( Statistics::owner_lock_queue, -1 );
#endif // __U_STATISTICS__
	    // owner_ and count set in release
	    return;
	} // if
	owner_ = &task;					// become owner
	count = 1;
    } else {
	count += 1;					// remember how often
    } // if
    spinLock.release();
} // uOwnerLock::acquire


bool uOwnerLock::tryacquire() {
    assert( uKernelModule::initialized ? ! THREAD_GETMEM( disableInt ) && THREAD_GETMEM( disableIntCnt ) == 0 : true );

    uBaseTask &task = uThisTask();			// optimization

  if ( owner_ != nullptr && owner_ != &task ) return false; // don't own lock yet

    spinLock.acquire();
#ifdef KNOT
    task.setActivePriority( task.getActivePriorityValue() + 1 );
#endif // KNOT
    if ( owner_ != &task ) {				// don't own lock yet
	if ( owner_ != nullptr ) {				// but if lock in use
	    spinLock.release();
	    return false;				// don't wait for the lock
	} // if
	owner_ = &task;					// become owner
	count = 1;
    } else {
	count += 1;					// remember how often
    } // if
    spinLock.release();
    return true;
} // uOwnerLock::tryacquire


void uOwnerLock::release() {
    assert( uKernelModule::initialized ? ! THREAD_GETMEM( disableInt ) && THREAD_GETMEM( disableIntCnt ) == 0 : true );

    spinLock.acquire();
    uDEBUG(
	if ( owner_ == nullptr ) {
	    spinLock.release();
	    abort( "Attempt to release owner lock (%p) that is not locked.", this );
	} // if
	if ( owner_ == (uBaseTask *)-1 ) {
	    spinLock.release();
	    abort( "Attempt to release owner lock (%p) that is in an invalid state. Possible cause is the lock has been freed.", this );
	} // if
	if ( owner_ != &uThisTask() ) {
	    uBaseTask *prev = owner_;			// owner could change as soon as spin lock released
	    spinLock.release();
	    abort( "Attempt to release owner lock (%p) that is currently owned by task %.256s (%p).", this, prev->getName(), prev );
	} // if
    )
    count -= 1;						// release the lock
    if ( count == 0 ) {					// if this is the last
	if ( ! waiting.empty() ) {			// waiting tasks ?
	    owner_ = &(waiting.dropHead()->task());	// remove task at head of waiting list and make new owner
	    count = 1;
	    owner_->wake();				// restart new owner
	} else {
	    owner_ = nullptr;				// release, no owner
	} // if
    } // if
#ifdef KNOT
    uThisTask().setActivePriority( uThisTask().getActivePriorityValue() - 1 );
#endif // KNOT
    spinLock.release();
} // uOwnerLock::release


//######################### uCondLock #########################


uDEBUG(
    uCondLock::~uCondLock() {
	spinLock.acquire();
	if ( ! waiting.empty() ) {
	    uBaseTask *task = &(waiting.head()->task());
	    spinLock.release();
	    abort( "Attempt to delete condition lock with task %.256s (%p) still blocked on it.", task->getName(), task );
	} // if
	spinLock.release();
    } // uCondLock::uCondLock
)


void uCondLock::wait( uOwnerLock &lock ) {
    uBaseTask &task = uThisTask();			// optimization
    uDEBUG(
	uBaseTask *owner = lock.owner();		// owner could change
	if ( owner != &task ) {
	    abort( "Attempt by waiting task %.256s (%p) to release condition lock currently owned by task %.256s (%p).",
		   task.getName(), &task, owner == nullptr ? "no-owner" : owner->getName(), owner );
	} // if
    )
    task.ownerLock = &lock;				// task remembers this lock before blocking for use in signal
    unsigned int prevcnt = lock.count;			// remember this lock's recursive count before blocking
    spinLock.acquire();
    waiting.addTail( &(task.entryRef) );		// queue current task
    // Must add to the condition queue first before releasing the owner lock because testing for empty condition can
    // occur immediately after the owner lock is released.
    lock.release_();					// release owner lock
    uProcessorKernel::schedule( &spinLock );		// atomically release condition spin lock and block
    // spin released by schedule, owner lock is acquired when task restarts
//    assert( &task == lock.owner() );
    lock.count = prevcnt;				// reestablish lock's recursive count after blocking
} // uCondLock::wait

void uCondLock::wait( uOwnerLock &lock, uintptr_t info ) {
    uThisTask().info = info;				// store the information with this task
    wait( lock );
} // uCondLock::wait

bool uCondLock::wait( uOwnerLock &lock, uDuration duration ) {
    return wait( lock, activeProcessorKernel->kernelClock.getTime() + duration );
} // uCondLock::wait

bool uCondLock::wait( uOwnerLock &lock, uintptr_t info, uDuration duration ) {
    uThisTask().info = info;				// store the information with this task
    return wait( lock, duration );
} // uCondLock::wait

bool uCondLock::wait( uOwnerLock &lock, uTime time ) {
    uBaseTask &task = uThisTask();			// optimization
    uDEBUG(
	uBaseTask *owner = lock.owner();		// owner could change
	if ( owner != &task ) {
	    abort( "Attempt by waiting task %.256s (%p) to release condition lock currently owned by task %.256s (%p).",
		   task.getName(), &task, owner == nullptr ? "no-owner" : owner->getName(), owner );
	} // if
    )
    task.ownerLock = &lock;				// task remembers this lock before blocking for use in signal
    unsigned int prevcnt = lock.count;			// remember this lock's recursive count before blocking
    spinLock.acquire();

    uDEBUGPRT( uDebugPrt( "(uCondLock &)%p.wait, task:%p\n", this, &task ); )

    TimedWaitHandler handler( task, *this );		// handler to wake up blocking task
    uEventNode timeoutEvent( task, handler, time, 0 );
    timeoutEvent.executeLocked = true;
    timeoutEvent.add();

    waiting.addTail( &(task.entryRef) );		// queue current task
    // Must add to the condition queue first before releasing the owner lock because testing for empty condition can
    // occur immediately after the owner lock is released.
    lock.release_();					// release owner lock
    uProcessorKernel::schedule( &spinLock );		// atomically release owner spin lock and block
    // spin released by schedule, owner lock is acquired when task restarts
    assert( &task == lock.owner() );
    lock.count = prevcnt;				// reestablish lock's recursive count after blocking

    timeoutEvent.remove();

    return ! handler.timedout;
} // uCondLock::wait

bool uCondLock::wait( uOwnerLock &lock, uintptr_t info, uTime time ) {
    uThisTask().info = info;			// store the information with this task
    return wait( lock, time );
} // uCondLock::wait

uintptr_t uCondLock::front() const {			// return task information
    uDEBUG(
	if ( waiting.empty() ) {			// condition queue must not be empty
	    abort( "Attempt to access user data on an empty condition lock.\n"
		   "Possible cause is not checking if the condition lock is empty before reading stored data." );
	} // if
    )
    return waiting.head()->task().info;			// return condition information stored with blocked task
} // uCondition::front


void uCondLock::waitTimeout( TimedWaitHandler &h ) {
    // This uCondLock member is called from the kernel, and therefore, cannot block, but it can spin.

    spinLock.acquire();
    uDEBUGPRT( uDebugPrt( "(uCondLock &)%p.waitTimeout, task:%p\n", this, &task ); )
    uBaseTask &task = *h.getThis();			// optimization
    if ( task.entryRef.listed() ) {			// is task on queue
	waiting.remove( &(task.entryRef) );		// remove this task O(1)
	h.timedout = true;
	spinLock.release();
	task.ownerLock->add_( task );			// restart it or chain to its owner lock
    } else {
	spinLock.release();
    } // if
} // uCondLock::waitTimeout


bool uCondLock::signal() {
    spinLock.acquire();
    if ( waiting.empty() ) {				// signal on empty condition is no-op
	spinLock.release();
	return false;
    } // if
    uBaseTask &task = waiting.dropHead()->task();	// remove task at head of waiting list
    spinLock.release();
    task.ownerLock->add_( task );			// restart it or chain to its owner lock
    return true;
} // uCondLock::signal


bool uCondLock::broadcast() {
    // It is impossible to chain the entire waiting list to the associated owner lock because each wait can be on a
    // different owner lock. Hence, each task has to be individually processed to move it onto the correct owner lock.

    uSequence<uBaseTaskDL> temp;
    spinLock.acquire();
    temp.transfer( waiting );
    spinLock.release();
  if ( temp.empty() ) return false;
    do {
	uBaseTask &task = temp.dropHead()->task();	// remove task at head of waiting list
	task.ownerLock->add_( task );			// restart it or chain to its owner lock
    } while ( ! temp.empty() );
    return true;
} // uCondLock::broadcast


void uCxtSwtchHndlr::handler() {
    // Do not use yield here because it polls for async events. Async events cannot be delivered because there is a
    // signal handler stack frame on the current stack, and it is unclear what the semantics are for abnormally
    // terminating that frame.

    uDEBUGPRT( uDebugPrt( "(uCxtSwtchHndlr &)%p.handler yield task:%p\n", this, &uThisTask() ); )

#if defined( __U_MULTI__ )
// freebsd has hidden locks in pthread routines. To prevent deadlock, disable interrupts so a context switch cannot
// occur to another user thread that makes the same call. As well, freebsd returns and undocumented EINTR from
// pthread_kill.
#if defined( __freebsd__ )
    int retcode;

    THREAD_GETMEM( This )->disableInterrupts();
    for ( ;; ) {
	retcode =
#endif // __freebsd__

	    RealRtn::pthread_kill( processor.getPid(), SIGUSR1 );

#if defined( __freebsd__ )
      if ( retcode != -1 || errno != EINTR ) break;	// not a timer interrupt ?
    } // if
    if ( retcode == -1 ) abort( "(uCxtSwtchHndlr &)%p.handler() : internal error, pthread_kill failed, error(%d) %s.", this, errno, strerror( errno ) );
    THREAD_GETMEM( This )->enableInterrupts();
#endif // __freebsd__

#else // UNIPROCESSOR
    uThisTask().uYieldInvoluntary();
#endif // __U_MULTI__
} // uCxtSwtchHndlr::handler


uCondLock::TimedWaitHandler::TimedWaitHandler( uBaseTask &task, uCondLock &condlock ) : condlock( condlock ) {
    This = &task;
    timedout = false;
} // uCondLock::TimedWaitHandler::TimedWaitHandler

uCondLock::TimedWaitHandler::TimedWaitHandler( uCondLock &condlock ) : condlock( condlock ) {
    This = nullptr;
    timedout = false;
} // uCondLock::TimedWaitHandler::TimedWaitHandler

void uCondLock::TimedWaitHandler::handler() {
    condlock.waitTimeout( *this );
} // uCondLock::TimedWaitHandler::handler


//######################### Real-Time #########################


uBaseTask &uBaseScheduleFriend::getInheritTask( uBaseTask &task ) const {
    return task.getInheritTask();
} // uBaseScheduleFriend::getInheritTask

int uBaseScheduleFriend::getActivePriority( uBaseTask &task ) const {
    // special case for base of active priority stack
    return task.getActivePriority();
} // uBaseScheduleFriend::getActivePriority

int uBaseScheduleFriend::getActivePriorityValue( uBaseTask &task ) const {
    return task.getActivePriorityValue();
} // uBaseScheduleFriend::getActivePriorityValue

int uBaseScheduleFriend::setActivePriority( uBaseTask &task1, int priority ) {
    return task1.setActivePriority( priority );
} // uBaseScheduleFriend::setActivePriority

int uBaseScheduleFriend::setActivePriority( uBaseTask &task1, uBaseTask &task2 ) {
    return task1.setActivePriority( task2 );
} // uBaseScheduleFriend::setActivePriority

int uBaseScheduleFriend::getBasePriority( uBaseTask &task ) const {
    return task.getBasePriority();
} // uBaseScheduleFriend::getBasePriority

int uBaseScheduleFriend::setBasePriority( uBaseTask &task, int priority ) {
    return task.setBasePriority( priority );
} // uBaseScheduleFriend::setBasePriority

int uBaseScheduleFriend::getActiveQueueValue( uBaseTask &task ) const {
    return task.getActiveQueueValue();
} // uBaseScheduleFriend::getActiveQueueValue

int uBaseScheduleFriend::setActiveQueue( uBaseTask &task1, int q ) {
    return task1.setActiveQueue( q );
} // uBaseScheduleFriend::setActiveQueue

int uBaseScheduleFriend::getBaseQueue( uBaseTask &task ) const {
    return task.getBaseQueue();
} // uBaseScheduleFriend::getBaseQueue

int uBaseScheduleFriend::setBaseQueue( uBaseTask &task, int q ) {
    return task.setBaseQueue( q );
} // uBaseScheduleFriend::setBaseQueue

bool uBaseScheduleFriend::isEntryBlocked( uBaseTask &task ) const {
    return task.entryRef.listed();
} // uBaseScheduleFriend::isEntryBlocked

bool uBaseScheduleFriend::checkHookConditions( uBaseTask &task1, uBaseTask &task2 ) const {
    return task2.getSerial().checkHookConditions( &task1 );
} // uBaseScheduleFriend::checkHookConditions


//######################### uBasePrioritySeq #########################


#ifdef KNOT
void uDefaultScheduler::add( uBaseTaskDL *taskNode ) {
    uFetchAdd( Statistics::ready_queue, 1 );
    if ( taskNode->task().getActivePriorityValue() == 0 ) {
	list.addTail( taskNode );
    } else {
	list.addHead( taskNode );
    } // if
} // uDefaultScheduler::add
#endif // KNOT


int uBasePrioritySeq::reposition( uBaseTask &task, UPP::uSerial &serial ) {
    remove( &(task.entryRef) );				// remove from entry queue
    task.calledEntryMem->remove( &(task.mutexRef) );	// remove from mutex queue
    
    // Call cluster routine to adjust ready queue and active priority as owner is not on entry queue, it can be updated
    // based on its uPIQ.

    uThisCluster().taskSetPriority( task, task );

    task.calledEntryMem->add( &(task.mutexRef), serial.mutexOwner ); // add to mutex queue
    return add( &(task.entryRef), serial.mutexOwner );	// add to entry queue, automatically does transitivity
} // uBasePrioritySeq::reposition


//######################### uRepositionEntry #########################


uRepositionEntry::uRepositionEntry( uBaseTask &blocked, uBaseTask &calling ) :
	blocked( blocked ), bSerial( blocked.getSerial() ), cSerial( calling.getSerial() ) {
} // uRepositionEntry::uRepositionEntry

int uRepositionEntry::uReposition( bool relCallingLock ) {
    int relPrevLock = 0;
    
    bSerial.spinLock.acquire();
    
    // If owner's current mutex object changes, then owner fixes its own active priority. Recheck if inheritance is
    // necessary as only owner can lower its priority => updated.

    if ( &bSerial != &blocked.getSerial() || ! blocked.entryRef.listed() ||
	 blocked.uPIQ->getHighestPriority() >= blocked.getActivePriorityValue() ) {
	// As owner restarted, the end of the blocking chain has been reached.
	bSerial.spinLock.release();
	return relPrevLock;
    } // if

    if ( relCallingLock ) {
	// release the old lock as correct current lock is acquired
	cSerial.spinLock.release();
	relPrevLock = 1;
    } // if

    if ( bSerial.entryList.reposition( blocked, bSerial ) == 0 ) {
	// only last call does not release lock, so reacquire first entry lock
	if ( relCallingLock == true ) uThisTask().getSerial().spinLock.acquire();
	bSerial.spinLock.release();
    } // if

    // The return value is based on the release of cSerial.lock not bSerial.lock.  The return value from bSerial is
    // processed in the if statement above, so it does not need to be propagated.

    return relPrevLock;
} // uRepositionEntry::uReposition


//######################### InterposeSymbol #########################


#define INIT_REALRTN( x, ver ) x = (__typeof__(x))interposeSymbol( #x, ver )

namespace UPP {
    void *RealRtn::interposeSymbol( const char *symbolName, const char *version ) {
	static void *library;
	const char *error;
	void *originalFunc;
	if ( library == nullptr ) {
#if defined( RTLD_NEXT )
	    library = RTLD_NEXT;
#else
	    // missing RTLD_NEXT => must hard-code library name, assuming libstdc++
	    library = dlopen( "libstdc++.so", RTLD_LAZY );
	    error = dlerror();
	    if ( error != nullptr ) {
		abort( "RealRtn::interposeSymbol : internal error, %s\n", error );
	    } // if
#endif // RTLD_NEXT
	} // if
#if defined( __linux__ )
	if ( version == nullptr ) {
#endif // __linux__
	    originalFunc = dlsym( library, symbolName );
#if defined( __linux__ )
	} else {
	    originalFunc = dlvsym( library, symbolName, version );
	} // if
#endif // __linux__
	error = dlerror();
	if ( error != nullptr ) {
	    ::abort( "RealRtn::interposeSymbol : internal error, %s\n", error );
	} // if
	return originalFunc;
    } // RealRtn::interposeSymbol
    
    // cannot use typeof here because of overloading => explicitly select builtin routine
    void (*RealRtn::exit)(int) __THROW __attribute__(( noreturn ));
    void (*RealRtn::abort)(void) __THROW __attribute__(( noreturn ));

    __typeof__( ::pselect ) *RealRtn::pselect;
    __typeof__( std::set_terminate ) *RealRtn::set_terminate;
    __typeof__( std::set_unexpected ) *RealRtn::set_unexpected;
#if defined( __linux__ ) || defined( __freebsd__ )
    __typeof__( ::dl_iterate_phdr ) *RealRtn::dl_iterate_phdr;
#endif // __linux__ || __freebsd__
#if defined( __U_MULTI__ )
    __typeof__( ::pthread_create ) *RealRtn::pthread_create;
//    __typeof__( ::pthread_exit ) *RealRtn::pthread_exit;
    __typeof__( ::pthread_attr_init ) *RealRtn::pthread_attr_init;
    __typeof__( ::pthread_attr_setstack ) *RealRtn::pthread_attr_setstack;
    __typeof__( ::pthread_kill ) *RealRtn::pthread_kill;
    __typeof__( ::pthread_join ) *RealRtn::pthread_join;
    __typeof__( ::pthread_self ) *RealRtn::pthread_self;
#if defined( __linux__ ) || defined( __freebsd__ )
    __typeof__( ::pthread_setaffinity_np ) *RealRtn::pthread_setaffinity_np;
    __typeof__( ::pthread_getaffinity_np ) *RealRtn::pthread_getaffinity_np;
#endif // __linux__ || __freebsd__
#endif // __U_MULTI__


    void RealRtn::startup() {
	const char *version = nullptr;
	INIT_REALRTN( exit, version );
	INIT_REALRTN( abort, version );
#if defined( __solaris__ ) && FD_SETSIZE > 1024 && __U_WORDSIZE__ == 32
	pselect = (__typeof__(pselect))interposeSymbol( "select_large_fdset", version );
#else
	INIT_REALRTN( pselect, version );
#endif // __solaris__
	set_terminate = (__typeof__(std::set_terminate)*)interposeSymbol( "_ZSt13set_terminatePFvvE", version );
	set_unexpected = (__typeof__(std::set_unexpected)*)interposeSymbol( "_ZSt14set_unexpectedPFvvE", version );
#if defined( __linux__ ) || defined( __freebsd__ )
	INIT_REALRTN( dl_iterate_phdr, version );
#endif // __linux__ || __freebsd__
#if defined( __U_MULTI__ )
	INIT_REALRTN( pthread_attr_setstack, version );
	INIT_REALRTN( pthread_self, version );
	INIT_REALRTN( pthread_kill, version );
	INIT_REALRTN( pthread_join, version );
#if defined( __linux__ ) || defined( __freebsd__ )
	INIT_REALRTN( pthread_setaffinity_np, version );
	INIT_REALRTN( pthread_getaffinity_np, version );
#endif // __linux__ || __freebsd__
//	INIT_REALRTN( pthread_exit, version );
#if defined( __linux__ ) && defined( __i386__ )
	version = "GLIBC_2.1";
#endif // __linux__&& __i386__
	INIT_REALRTN( pthread_create, version );
	INIT_REALRTN( pthread_attr_init, version );
#endif // __U_MULTI__
    } // RealRtn::startup
} // UPP


#if defined( __linux__ ) || defined( __freebsd__ )
extern "C" int dl_iterate_phdr( int (*callback)( dl_phdr_info *, size_t, void * ), void *data ) {
    assert( RealRtn::dl_iterate_phdr != nullptr );
    THREAD_GETMEM( This )->disableInterrupts();
    int ret = RealRtn::dl_iterate_phdr( callback, data );
    THREAD_GETMEM( This )->enableInterrupts();
    return ret;
} // dl_iterate_phdr
#endif // __linux__


//######################### uKernelModule #########################


void uKernelModule::startup() {
    uKernelModule::kernelModuleInitialized = true;

    // No concurrency, so safe to make direct call through TLS pointer.
    uKernelModule::uKernelModuleBoot.ctor();

    // SKULLDUGGERY: This ensures that calls to uThisCoroutine work before the kernel is initialized.

    ((uBaseTask *)&uKernelModule::bootTaskStorage)->currCoroutine = (uBaseTask *)&uKernelModule::bootTaskStorage;
    ((uBaseTask *)&uKernelModule::bootTaskStorage)->inheritTask = (uBaseTask *)&uKernelModule::bootTaskStorage;
} // uKernelModule::startup


void uKernelModule::uKernelModuleData::ctor() volatile {
    This = this;
    kernelModuleInitialized = true;

    activeProcessor = (uProcessor *)&uKernelModule::systemProcessorStorage;
    activeCluster = (uCluster *)&uKernelModule::systemClusterStorage;
    activeTask = (uBaseTask *)&bootTaskStorage;

    disableInt = true;
    disableIntCnt = 1;

    disableIntSpin = false;
    disableIntSpinCnt = 0;

    RFpending = RFinprogress = false;

#if defined( __ia64__ ) && ( defined( __linux__ ) || defined( __freebsd__ ) ) && defined( __U_MULTI__ )
    // set private memory pointer
    register volatile uKernelModule *thread_self asm( "r13" );
    threadPointer = (unsigned long)thread_self;
#endif // __U_MULTI__
} // uKernelModule::ctor


void uKernelModule::rollForward( bool inKernel ) {
    uDEBUGPRT( char buffer[256];
	      uDebugPrtBuf( buffer, "rollForward( %d ), disableInt:%d, disableIntCnt:%d, disableIntSpin:%d, disableIntSpinCnt:%d, RFpending:%d, RFinprogress:%d\n",
			    inKernel, THREAD_GETMEM(disableInt), THREAD_GETMEM(disableIntCnt), THREAD_GETMEM(disableIntSpin), THREAD_GETMEM(disableIntSpinCnt),
			    THREAD_GETMEM(RFpending), THREAD_GETMEM(RFinprogress) ); )

#ifdef __U_STATISTICS__
    uFetchAdd( UPP::Statistics::roll_forward, 1 );
#endif // __U_STATISTICS__

#if defined( __U_MULTI__ )
    if ( &uThisProcessor() == uKernelModule::systemProcessor ) { // process events on event list
#endif // __U_MULTI__
	uEventNode *event;
	for ( uEventListPop iter( *uKernelModule::systemProcessor->events, inKernel ); iter >> event; );
#if defined( __U_MULTI__ )
    } else {						// other processors only deal with context-switch
	THREAD_SETMEM( RFinprogress, false );
	if ( ! inKernel ) {				// not in kernel ?
	    THREAD_SETMEM( RFpending, false );
	    uThisTask().uYieldInvoluntary();
	} // if
    } // if
#endif // __U_MULTI__

    uDEBUGPRT( uDebugPrtBuf( buffer, "rollForward, leaving, RFinprogress:%d\n", THREAD_GETMEM( RFinprogress ) ); )
} // uKernelModule::rollForward


//######################### Translator Generated Definitions #########################


namespace UPP {
    uSerial::uSerial( uBasePrioritySeq &entryList ) : entryList( entryList ) {
#ifdef __U_STATISTICS__
	uFetchAdd( UPP::Statistics::uSerials, 1 );
#endif // __U_STATISTICS__
	mask.clrAll();					// mutex members start closed
	mutexOwner = &uThisTask();			// set the current mutex owner to the creating task

	// Make creating task the owner of the mutex.
	prevSerial = &mutexOwner->getSerial();		// save previous serial
	mutexOwner->setSerial( *this );			// set new serial
	mr = mutexOwner->mutexRecursion;		// save previous recursive count
	mutexOwner->mutexRecursion = 0;			// reset recursive count

	acceptMask = false;
	mutexMaskLocn = nullptr;

	destructorTask = nullptr;
	destructorStatus = NoDestructor;
	constructorTask = mutexOwner;

	// real-time

	timeoutEvent.executeLocked = true;
	events = nullptr;

	// exception handling

	lastAcceptor = nullptr;
	notAlive = false;

#ifdef __U_PROFILER__
	// profiling

	profileSerialSamplerInstance = nullptr;
#endif // __U_PROFILER__
    } // uSerial::uSerial


    uSerial::~uSerial() {
	notAlive = true;				// no more entry calls can be accepted
	uBaseTask &task = uThisTask();			// optimization
	task.setSerial( *prevSerial );			// reset previous serial

	for ( ;; ) {
	    uBaseTaskDL *p = acceptSignalled.drop();
	  if ( p == nullptr ) break;
	    mutexOwner = &(p->task());
	    _Resume uMutexFailure::EntryFailure( this, "blocked on acceptor/signalled stack" ) _At *(mutexOwner->currCoroutine);
	    acceptSignalled.add( &(task.mutexRef) );	// suspend current task on top of accept/signalled stack
	    uProcessorKernel::schedule( mutexOwner );
	} // for

	if ( ! entryList.empty() ) {			// no need to acquire the lock if the queue is empty
	    for ( ;; ) {
		spinLock.acquire();
		uBaseTaskDL *p = entryList.drop();
	      if ( p == nullptr ) break;
		mutexOwner = &(p->task());
		mutexOwner->calledEntryMem->remove( &(mutexOwner->mutexRef) );
		_Resume uMutexFailure::EntryFailure( this, "blocked on entry queue" ) _At *(mutexOwner->currCoroutine);
		acceptSignalled.add( &(task.mutexRef) ); // suspend current task on top of accept/signalled stack
		uProcessorKernel::schedule( &spinLock, mutexOwner );
	    } // for
	    spinLock.release();
	} // if
    } // uSerial::~uSerial


    void uSerial::resetDestructorStatus() {
	destructorStatus = NoDestructor;
	destructorTask = nullptr;
    } // uSerial::rresetDestructorStatus


    void uSerial::enter( unsigned int &mr, uBasePrioritySeq &ml, int mp ) {
	uBaseTask &task = uThisTask();			// optimization
	spinLock.acquire();

	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.enter enter, mask:0x%x,0x%x,0x%x,0x%x, owner:%p, maskposn:%p, ml:%p, mp:%d\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner, mutexMaskLocn, &ml, mp ); )

	if ( mask.isSet( mp ) ) {			// member acceptable ?
	    mask.clrAll();				// clear the mask
	    mr = task.mutexRecursion;			// save previous recursive count
	    task.mutexRecursion = 0;			// reset recursive count
	    mutexOwner = &task;				// set the current mutex owner
	    if ( entryList.executeHooks ) {
		// always execute hook as calling task cannot be constructor or destructor
		entryList.onAcquire( *mutexOwner );	// perform any priority inheritance
	    } // if
	    spinLock.release();
	} else if ( mutexOwner == &task ) {		// already hold mutex ?
	    task.mutexRecursion += 1;			// another recursive call at the mutex object level
	    spinLock.release();
	} else {					// otherwise block the calling task
	    ml.add( &(task.mutexRef), mutexOwner );	// add to end of mutex queue
	    task.calledEntryMem = &ml;			// remember which mutex member called
	    entryList.add( &(task.entryRef), mutexOwner ); // add mutex object to end of entry queue
	    uProcessorKernel::schedule( &spinLock );	// find someone else to execute; release lock on kernel stack
	    mr = task.mutexRecursion;			// save previous recursive count
	    task.mutexRecursion = 0;			// reset recursive count
	    _Enable <uMutexFailure>;			// implicit poll
	} // if
	if ( mutexMaskLocn != nullptr ) {			// part of a rendezvous ? (i.e., member accepted)
	    *mutexMaskLocn = mp;			// set accepted mutex member
	    mutexMaskLocn = nullptr;			// mark as further mutex calls are not part of rendezvous
	} // fi
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.enter exit, mask:0x%x,0x%x,0x%x,0x%x, owner:%p, maskposn:%p, ml:%p, mp:%d\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner, mutexMaskLocn, &ml, mp ); )
    } // uSerial::enter


    // enter routine for destructor, does not poll
    void uSerial::enterDestructor( unsigned int &mr, uBasePrioritySeq &ml, int mp ) {
	uBaseTask &task = uThisTask();			// optimization

	if ( destructorStatus != NoDestructor ) {		// only one task is allowed to call destructor
	    abort( "Attempt by task %.256s (%p) to call the destructor for uSerial %p, but this destructor was already called by task %.256s (%p).\n"
		    "Possible cause is multiple tasks simultaneously deleting a mutex object.",
		    task.getName(), &task, this, destructorTask->getName(), destructorTask );
	} // if

	spinLock.acquire();

	destructorStatus = DestrCalled;
	destructorTask = &task;

	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.enterDestructor enter, mask:0x%x,0x%x,0x%x,0x%x, owner:%p, maskposn:%p, ml:%p, mp:%d\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner, mutexMaskLocn, &ml, mp ); )
	if ( mask.isSet( mp ) ) {			// member acceptable ?
	    mask.clrAll();				// clear the mask
	    mr = task.mutexRecursion;			// save previous recursive count
	    task.mutexRecursion = 0;			// reset recursive count
	    mutexOwner = &task;				// set the current mutex owner
	    destructorStatus = DestrScheduled;
	    // hook is not executed for destructor
	    spinLock.release();
	} else if ( mutexOwner == &task ) {		// already hold mutex ?
	    abort( "Attempt by task %.256s (%p) to call the destructor for uSerial %p, but this task has outstanding nested calls to this mutex object.\n"
		    "Possible cause is deleting a mutex object with outstanding nested calls to one of its members.",
		    task.getName(), &task, this );
	} else {					// otherwise block the calling task
	    task.calledEntryMem = &ml;			// remember which mutex member was called
	    uProcessorKernel::schedule( &spinLock );	// find someone else to execute; release lock on kernel stack
	    mr = task.mutexRecursion;			// save previous recursive count
	    task.mutexRecursion = 0;			// reset recursive count
	} // if
	if ( mutexMaskLocn != nullptr ) {			// part of a rendezvous ? (i.e., member accepted)
	    *mutexMaskLocn = mp;			// set accepted mutex member
	    mutexMaskLocn = nullptr;			// mark as further mutex calls are not part of rendezvous
	} // fi
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.exitDestructor exit, mask:0x%x,0x%x,0x%x,0x%x, owner:%p, maskposn:%p, ml:%p, mp:%d\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner, mutexMaskLocn, &ml, mp ); )
    } // uSerial::enterDestructor


    void uSerial::enterTimeout() {
	// This monitor member is called from the kernel, and therefore, cannot block, but it can spin.

	spinLock.acquire();

	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.enterTimeout enter, mask:0x%x,0x%x,0x%x,0x%x, owner:%p, maskposn:%p\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner, mutexMaskLocn ); )
	if ( mask.isSet( __U_TIMEOUTPOSN__ ) ) {	// timeout member acceptable ?  0 => timeout mask bit
	    mask.clrAll();				// clear the mask
	    *mutexMaskLocn = 0;				// set timeout mutex member  0 => timeout mask bit
	    mutexOwner = &(acceptSignalled.drop()->task()); // next task to gain control of the mutex object
	
	    // priority-inheritance, bump up priority of mutexowner from head of prioritized entry queue (NOT leaving
	    // task), because suspended stack is not prioritized.
	
	    if ( entryList.executeHooks && checkHookConditions( mutexOwner ) ) {
		entryList.onAcquire( *mutexOwner );
	    } // if

	    uDEBUGPRT( uDebugPrt( "(uSerial &)%p.enterTimeout, waking task %.256s (%p) \n", this, mutexOwner->getName(), mutexOwner ); )
	    mutexOwner->wake();				// wake up next task to use this mutex object
	} // if

	spinLock.release();
    } // uSerial::enterTimeout


    // leave and leave2 do not poll for concurrent exceptions because they are called in some critical destructors.
    // Throwing an exception out of these destructors causes problems.

    void uSerial::leave( unsigned int mr ) {		// used when a task is leaving a mutex and has not queued itself before calling
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave enter, mask:0x%x,0x%x,0x%x,0x%x, owner:%p, mr:%u\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner, mr ); )
	uBaseTask &task = uThisTask();			// optimization

	if ( task.mutexRecursion != 0 ) {		// already hold mutex ?
	    if ( acceptMask ) {
		// lock is acquired and mask set by accept statement
		acceptMask = false;
		spinLock.release();
	    } // if
	    task.mutexRecursion -= 1;
	} else {
	    if ( acceptMask ) {
		// lock is acquired and mask set by accept statement
		acceptMask = false;
		mutexOwner = nullptr;			// reset no task in mutex object
		if ( entryList.executeHooks && checkHookConditions( &task ) ) {
		    entryList.onRelease( task );
		} // if
		if ( &task == destructorTask ) resetDestructorStatus();
		spinLock.release();
	    } else if ( acceptSignalled.empty() ) {	// no tasks waiting re-entry to mutex object ?
		spinLock.acquire();
		if ( destructorStatus != DestrCalled ) {
		    if ( entryList.empty() ) {		// no tasks waiting entry to mutex object ?
			mask.setAll();			// accept all members
			mask.clr( 0 );			// except timeout
			mutexOwner = nullptr;		// reset no task in mutex object
			if ( entryList.executeHooks && checkHookConditions( &task ) ) {
			    entryList.onRelease( task );
			} // if
			if ( &task == destructorTask ) resetDestructorStatus();
			spinLock.release();
		    } else {				// tasks wating entry to mutex object
			mutexOwner = &(entryList.drop()->task()); // next task to gain control of the mutex object
			mutexOwner->calledEntryMem->remove( &(mutexOwner->mutexRef) ); // also remove task from mutex queue
			if ( entryList.executeHooks ) {
			    if ( checkHookConditions( &task ) ) entryList.onRelease( task );
			    if ( checkHookConditions( mutexOwner ) ) entryList.onAcquire( *mutexOwner );
			} // if
			if ( &task == destructorTask ) resetDestructorStatus();
			spinLock.release();
			uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave, waking task %.256s (%p)\n", this, mutexOwner->getName(), mutexOwner ); )
			mutexOwner->wake();		// wake up next task to use this mutex object
		    } // if
		} else {
		    mutexOwner = destructorTask;
		    destructorStatus = DestrScheduled;
		    if ( entryList.executeHooks ) {
			if ( checkHookConditions( &task ) ) entryList.onRelease( task );
			// do not call acquire the hook for the destructor
		    } // if
		    spinLock.release();
		    uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave, waking task %.256s (%p)\n", this, mutexOwner->getName(), mutexOwner ); )
		    mutexOwner->wake();			// wake up next task to use this mutex object
		} // if
	    } else {
		// priority-inheritance, bump up priority of mutexowner from head of prioritized entry queue (NOT
		// leaving task), because suspended stack is not prioritized.

		if ( entryList.executeHooks ) {
		    spinLock.acquire();			// acquire entry lock to prevent inversion during transfer 
		    mutexOwner = &(acceptSignalled.drop()->task()); // next task to gain control of the mutex object
		    if ( checkHookConditions( &task ) ) entryList.onRelease( task );
		    if ( checkHookConditions( mutexOwner ) ) entryList.onAcquire( *mutexOwner );
		    uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave, waking task %.256s (%p)\n", this, mutexOwner->getName(), mutexOwner ); )
		    mutexOwner->wake();			// wake up next task to use this mutex object
		    if ( &task == destructorTask ) resetDestructorStatus();
		    spinLock.release();
		} else {
		    uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave, waking task %.256s (%p)\n", this, mutexOwner->getName(), mutexOwner ); )
		    mutexOwner = &(acceptSignalled.drop()->task()); // next task to gain control of the mutex object
		    if ( &task == destructorTask ) {
			spinLock.acquire();
			resetDestructorStatus();
			spinLock.release();
		    } // if
		    mutexOwner->wake();			// wake up next task to use this mutex object
		} // if
	    } // if
	    task.mutexRecursion = mr;			// restore previous recursive count
	} // if

	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave, mask:0x%x,0x%x,0x%x,0x%x, owner:%p\n", this, mask[0], mask[1], mask[2], mask[3], mutexOwner ); )
    } // uSerial::leave


    void uSerial::leave2() {				// used when a task is leaving a mutex and has queued itself before calling
	uBaseTask &task = uThisTask();			// optimization

	if ( acceptMask ) {
	    // lock is acquired and mask set by accept statement
	    acceptMask = false;
	    mutexOwner = nullptr;				// reset no task in mutex object
	    if ( entryList.executeHooks && checkHookConditions( &task ) ) {
		entryList.onRelease( task );
	    } // if
	    if ( &task == destructorTask ) resetDestructorStatus();
	    uProcessorKernel::schedule( &spinLock );	// find someone else to execute; release lock on kernel stack
	} else if ( acceptSignalled.empty() ) {		// no tasks waiting re-entry to mutex object ?
	    spinLock.acquire();
	    if ( destructorStatus != DestrCalled ) {
		if ( entryList.empty() ) {		// no tasks waiting entry to mutex object ?
		    mask.setAll();			// accept all members
		    mask.clr( 0 );			// except timeout
		    mutexOwner = nullptr;
		    if ( entryList.executeHooks && checkHookConditions( &task ) ) {
			entryList.onRelease( task );
		    } // if
		    uProcessorKernel::schedule( &spinLock ); // find someone else to execute; release lock on kernel stack
		} else {
		    mutexOwner = &(entryList.drop()->task()); // next task to gain control of the mutex object
		    mutexOwner->calledEntryMem->remove( &(mutexOwner->mutexRef) ); // also remove task from mutex queue
		    if ( entryList.executeHooks ) {
			if ( checkHookConditions( &task ) ) entryList.onRelease( task );
			if ( checkHookConditions( mutexOwner ) ) entryList.onAcquire( *mutexOwner );
		    } // if
		    uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave2, waking task %.256s (%p)\n", this, mutexOwner->getName(), mutexOwner ); )
		    uProcessorKernel::schedule( &spinLock, mutexOwner ); // find someone else to execute; release lock and wake on kernel stack
		} // if
	    } else {
		mutexOwner = destructorTask;
		destructorStatus = DestrScheduled;
		if ( entryList.executeHooks ) {
		    if ( checkHookConditions( &task ) ) entryList.onRelease( task );
		    // do not call acquire the hook for the destructor
		} // if
		uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave2, waking task %.256s (%p)\n", this, mutexOwner->getName(), mutexOwner ); )
		uProcessorKernel::schedule( &spinLock, mutexOwner ); // find someone else to execute; release lock and wake on kernel stack
	    } // if
	} else {
	    // priority-inheritance, bump up priority of mutexowner from head of prioritized entry queue (NOT leaving
	    // task), because suspended stack is not prioritized.

	    uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave2, waking task %.256s (%p)\n", this, mutexOwner->getName(), mutexOwner ); )
	    if ( entryList.executeHooks ) {
		spinLock.acquire();
		mutexOwner = &(acceptSignalled.drop()->task()); // next task to gain control of the mutex object
		if ( checkHookConditions( &task ) ) entryList.onRelease( task );
		if ( checkHookConditions( mutexOwner ) ) entryList.onAcquire( *mutexOwner );
		uProcessorKernel::schedule( &spinLock, mutexOwner ); // find someone else to execute; release lock and wake on kernel stack
	    } else {
		mutexOwner = &(acceptSignalled.drop()->task()); // next task to gain control of the mutex object
		uProcessorKernel::schedule( mutexOwner ); // find someone else to execute; wake on kernel stack
	    } // if
	} // if
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.leave2, mask:0x%x,0x%x,0x%x,0x%x, owner:%p\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner ); )
    } // uSerial::leave2


    void uSerial::removeTimeout() {
	if ( events != nullptr ) {
	    timeoutEvent.remove();
	} // if
    } // uSerial::removeTimeout


    bool uSerial::checkHookConditions( uBaseTask *task ) {
	return task != constructorTask && task != destructorTask;
    } // uSerial::checkHookConditions


    // The field uSerial::lastAcceptor is set in acceptTry and acceptPause and reset to null in
    // uSerialMember::uSerialMember, so an exception can be thrown when all the guards in the accept statement fail.
    // This ensures that lastAcceptor is set only when a task rendezouvs with another task.

    void uSerial::acceptStart( unsigned int &mutexMaskLocn ) {
#if defined( __U_DEBUG__ ) || defined( __U_PROFILER__ )
	uBaseTask &task = uThisTask();			// optimization
#endif // __U_DEBUG__ || __U_PROFILER__
	uDEBUG(
	    if ( &task != mutexOwner ) {		// must have mutex lock to wait
		abort( "Attempt to accept in a mutex object not locked by this task.\n"
		       "Possible cause is accepting in a nomutex member routine." );
	    } // if
	)
#ifdef __U_PROFILER__
	if ( task.profileActive && uProfiler::uProfiler_registerAcceptStart ) { // task registered for profiling ?
	    (*uProfiler::uProfiler_registerAcceptStart)( uProfiler::profilerInstance, *this, task );
	} // if
#endif // __U_PROFILER__

	uSerial::mutexMaskLocn = &mutexMaskLocn;
	acceptLocked = false;
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.acceptStart, mask:0x%x,0x%x,0x%x,0x%x, owner:%p, maskposn:%p\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner, &mutexMaskLocn ); )
    } // uSerial::acceptStart


    bool uSerial::acceptTry( uBasePrioritySeq &ml, int mp ) {
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.acceptTry enter, mask:0x%x,0x%x,0x%x,0x%x, ml:%p, mp:%d\n",
			      this, mask[0], mask[1], mask[2], mask[3], &ml, mp ); )
	if ( ! acceptLocked ) {				// lock is acquired on demand
	    spinLock.acquire();
	    mask.clrAll();
	    acceptLocked = true;
	} // if
	if ( mp == __U_DESTRUCTORPOSN__ ) {		// ? destructor accepted
	    // Handles the case where destructor has not been called or the destructor has been scheduled.  If the
	    // destructor has been scheduled, there is potential for synchronization deadlock when only the destructor
	    // is accepted.
	    if ( destructorStatus != DestrCalled ) {
		mask.set( mp );				// add this mutex member to the mask
		return false;				// the accept failed
	    } else {
		uBaseTask &task = uThisTask();		// optimization
		mutexOwner = destructorTask;		// next task to use this mutex object
		destructorStatus = DestrScheduled;	// change status of destructor to scheduled
		lastAcceptor = &task;			// saving the acceptor thread of a rendezvous
		mask.clrAll();				// clear the mask
		acceptSignalled.add( &(task.mutexRef) ); // suspend current task on top of accept/signalled stack
		if ( entryList.executeHooks ) {
		    // no check for destructor because it cannot accept itself
		    if ( checkHookConditions( &task ) ) entryList.onRelease( task );  
		    // do not call the acquire hook for the destructor
		} // if
		uProcessorKernel::schedule( &spinLock, mutexOwner ); // find someone else to execute; release lock and wake on kernel stack
		if ( task.acceptedCall ) {		// accepted entry is suspended if true
		    task.acceptedCall->acceptorSuspended = false; // acceptor resumes
		    task.acceptedCall = nullptr;
		} // if
		_Enable <uMutexFailure::RendezvousFailure>; // implicit poll
		return true;
	    } // if
	} else {
	    if ( ml.empty() ) {
		mask.set( mp );				// add this mutex member to the mask
		return false;				// the accept failed
	    } else {
		uBaseTask &task = uThisTask();		// optimization
		mutexOwner = &(ml.drop()->task());	// next task to use this mutex object
		lastAcceptor = &task;			// saving the acceptor thread of a rendezvous
		entryList.remove( &(mutexOwner->entryRef) ); // also remove task from entry queue
		mask.clrAll();				// clear the mask
		acceptSignalled.add( &(task.mutexRef) ); // suspend current task on top of accept/signalled stack
		if ( entryList.executeHooks ) {
		    if ( checkHookConditions( &task ) ) entryList.onRelease( task );  
		    if ( checkHookConditions( mutexOwner ) ) entryList.onAcquire( *mutexOwner );
		} // if
		uProcessorKernel::schedule( &spinLock, mutexOwner ); // find someone else to execute; release lock and wake on kernel stack
		if ( task.acceptedCall ) {		// accepted entry is suspended if true
		    task.acceptedCall->acceptorSuspended = false; // acceptor resumes
		    task.acceptedCall = nullptr;
		} // if
		_Enable <uMutexFailure::RendezvousFailure>; // implicit poll
		return true;
	    } // if
	} // if
    } // uSerial::acceptTry


    void uSerial::acceptTry() {
	if ( ! acceptLocked ) {				// lock is acquired on demand
	    spinLock.acquire();
	    mask.clrAll();
	    acceptLocked = true;
	} // if
	mask.set( 0 );					// add this mutex member to the mask, 0 => timeout mask bit
    } // uSerial::acceptTry


    bool uSerial::acceptTry2( uBasePrioritySeq &ml, int mp ) {
	if ( ! acceptLocked ) {				// lock is acquired on demand
	    spinLock.acquire();
	    mask.clrAll();
	    acceptLocked = true;
	} // if
	if ( mp == __U_DESTRUCTORPOSN__ ) {		// ? destructor accepted
	    // Handles the case where destructor has not been called or the destructor has been scheduled.  If the
	    // destructor has been scheduled, there is potential for synchronization deadlock when only the destructor
	    // is accepted.
	    if ( destructorStatus != DestrCalled ) {
		mask.set( mp );				// add this mutex member to the mask
		return false;				// the accept failed
	    } else {
		uBaseTask *acceptedTask = destructorTask; // next task to use this mutex object
		destructorStatus = DestrScheduled;	// change status of destructor to scheduled
		mask.clrAll();				// clear the mask
		spinLock.release();
		acceptSignalled.add( &(acceptedTask->mutexRef) ); // move accepted task on top of accept/signalled stack
		return true;
	    } // if
	} else {
	    if ( ml.empty() ) {
		mask.set( mp );				// add this mutex member to the mask
		return false;				// the accept failed
	    } else {
		uBaseTask *acceptedTask = &(ml.drop()->task()); // next task to use this mutex object
		entryList.remove( &(acceptedTask->entryRef) ); // also remove task from entry queue
		mask.clrAll();				// clear the mask
		spinLock.release();
		acceptSignalled.add( &(acceptedTask->mutexRef) ); // move accepted task on top of accept/signalled stack
		return true;
	    } // if
	} // if
    } // uSerial::acceptTry2


    void uSerial::acceptPause() {
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.acceptPause enter, mask:0x%x,0x%x,0x%x,0x%x, owner:%p\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner ); )
	// lock is acquired at beginning of accept statement
	uBaseTask &task = uThisTask();			// optimization
	lastAcceptor = &task;				// saving the acceptor thread of a rendezvous
	acceptSignalled.add( &(task.mutexRef) );	// suspend current task on top of accept/signalled stack

	mutexOwner = nullptr;
	if ( entryList.executeHooks && checkHookConditions( &task ) ) {
	    entryList.onRelease( task );
	} // if
	uProcessorKernel::schedule( &spinLock );	// find someone else to execute; release lock on kernel stack

	if ( task.acceptedCall ) {			// accepted entry is suspended if true
	    task.acceptedCall->acceptorSuspended = false; // acceptor resumes
	    task.acceptedCall = nullptr;
	} // if
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.acceptPause restart, mask:0x%x,0x%x,0x%x,0x%x, owner:%p, maskposn:%p\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner, mutexMaskLocn ); )
	_Enable <uMutexFailure>;			// implicit poll
    } // uSerial::acceptPause


    void uSerial::acceptPause( uDuration duration ) {
	acceptPause( activeProcessorKernel->kernelClock.getTime() + duration );
    } // uSerial::acceptPause


    void uSerial::acceptPause( uTime time ) {
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.acceptPause( time ) enter, mask:0x%x,0x%x,0x%x,0x%x, owner:%p\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner ); )
	// lock is acquired at beginning of accept statement
	uBaseTask &task = uThisTask();			// optimization
	uTimeoutHndlr handler( task, *this );		// handler to wake up blocking task

	timeoutEvent.alarm = time;
	timeoutEvent.task = &task;
	timeoutEvent.sigHandler = &handler;
	timeoutEvent.executeLocked = true;

	timeoutEvent.add();

	lastAcceptor = &task;				// saving the acceptor thread of a rendezvous
	acceptSignalled.add( &(task.mutexRef) );	// suspend current task on top of accept/signalled stack

	mutexOwner = nullptr;
	if ( entryList.executeHooks && checkHookConditions( &task ) ) {
	    entryList.onRelease( task );
	} // if
	uProcessorKernel::schedule( &spinLock );	// find someone else to execute; release lock on kernel stack

	timeoutEvent.remove();

	if ( task.acceptedCall ) {			// accepted entry is suspended if true
	    task.acceptedCall->acceptorSuspended = false; // acceptor resumes
	    task.acceptedCall = nullptr;
	} // if
	uDEBUGPRT( uDebugPrt( "(uSerial &)%p.acceptPause( time ) restart, mask:0x%x,0x%x,0x%x,0x%x, owner:%p, maskposn:%p\n",
			      this, mask[0], mask[1], mask[2], mask[3], mutexOwner, mutexMaskLocn ); )
	_Enable <uMutexFailure>;			// implicit poll
    } // uSerial::acceptPause


    void uSerial::acceptEnd() {
	mutexMaskLocn = nullptr;				// not reset after timeout

#ifdef __U_PROFILER__
	if ( uThisTask().profileActive && uProfiler::uProfiler_registerAcceptEnd ) { // task registered for profiling ?              
	    (*uProfiler::uProfiler_registerAcceptEnd)( uProfiler::profilerInstance, *this, uThisTask() );
	} // if
#endif // __U_PROFILER__
    } // uSerial::acceptEnd


    uSerialConstructor::uSerialConstructor( uAction f, uSerial &serial ) : f( f ), serial( serial ) {
	uDEBUGPRT( uDebugPrt( "(uSerialConstructor &)%p.uSerialConstructor, f:%d, s:%p\n", this, f, &serial ); )
    } // uSerialConstructor::uSerialConstructor


#ifdef __U_PROFILER__
    uSerialConstructor::uSerialConstructor( uAction f, uSerial &serial, const char *n ) : f( f ), serial( serial ) {
	uDEBUGPRT( uDebugPrt( "(uSerialConstructor &)%p.uSerialConstructor, f:%d, s:%p, n:%s\n", this, f, &serial, n ); )

	if ( f == uYes ) {
	    if ( uThisTask().profileActive && uProfiler::uProfiler_registerMonitor ) { // task registered for profiling ?
		(*uProfiler::uProfiler_registerMonitor)( uProfiler::profilerInstance, serial, n, uThisTask() );
	    } // if
	} // if
    } // uSerialConstructor::uSerialConstructor
#endif // __U_PROFILER__


    uSerialConstructor::~uSerialConstructor() {
	uDEBUGPRT( uDebugPrt( "(uSerialConstructor &)%p.~uSerialConstructor\n", this ); )
	if ( f == uYes && ! std::__U_UNCAUGHT_EXCEPTION__() ) {
	    uBaseTask &task = uThisTask();		// optimization

	    task.setSerial( *serial.prevSerial );	// reset previous serial
	    serial.leave( serial.mr );
	} // if
    } // uSerialConstructor::~uSerialConstructor


    uSerialDestructor::uSerialDestructor( uAction f, uSerial &serial, uBasePrioritySeq &ml, int mp ) : f( f ) {
	if ( f == uYes ) {
	    uBaseTask &task = uThisTask();		// optimization
	    uDEBUG( nlevel = task.currSerialLevel += 1; )
	    serial.prevSerial = &task.getSerial();	// save previous serial
	    task.setSerial( serial );			// set new serial

	    serial.enterDestructor( mr, ml, mp );
	    if ( ! serial.acceptSignalled.empty() ) {
		serial.lastAcceptor = nullptr;
		uBaseTask &uCallingTask = task;		// optimization
		serial.mutexOwner = &(serial.acceptSignalled.drop()->task());
		serial.acceptSignalled.add( &(uCallingTask.mutexRef) ); // suspend terminating task on top of accept/signalled stack
		uProcessorKernel::schedule( serial.mutexOwner ); // find someone else to execute; wake on kernel stack
	    } // if
	} // if
    } // uSerialDestructor::uSerialDestructor


    uSerialDestructor::~uSerialDestructor() {
	if ( f == uYes ) {
	    uBaseTask &task = uThisTask();		// optimization
	    uSerial &serial = task.getSerial();		// get current serial
	    // Useful for dynamic allocation if an exception is thrown in the destructor so the object can continue to
	    // be used and deleted again.
	    uDEBUG(
		if ( nlevel != task.currSerialLevel ) {
		    abort( "Attempt to perform a non-nested entry and exit from multiple accessed mutex objects." );
		} // if
		task.currSerialLevel -= 1;
	    )
	    if ( std::__U_UNCAUGHT_EXCEPTION__() ) {
		serial.leave( mr );
	    } else {
#ifdef __U_PROFILER__
		if ( task.profileActive && uProfiler::uProfiler_deregisterMonitor ) { // task registered for profiling ?
		    (*uProfiler::uProfiler_deregisterMonitor)( uProfiler::profilerInstance, serial, task );
		} // if
#endif // __U_PROFILER__

		task.mutexRecursion = mr;		// restore previous recursive count
	    } // if
	} // if
    } // uSerialDestructor::~uSerialDestructor


    void uSerialMember::finalize( uBaseTask &task ) {
	uDEBUG(
	    if ( nlevel != task.currSerialLevel ) {
		abort( "Attempt to perform a non-nested entry and exit from multiple accessed mutex objects." );
	    } // if
	    task.currSerialLevel -= 1;
	)
	task.setSerial( *prevSerial );			// reset previous serial
    } // uSerialMember::finalize

    uSerialMember::uSerialMember( uSerial &serial, uBasePrioritySeq &ml, int mp ) {
	uBaseTask &task = uThisTask();			// optimization

	// There is a race condition between setting and testing this flag.  However, it is the best that can be
	// expected because the mutex storage is being deleted.

	if ( serial.notAlive ) {			// check against improper memory management
	    _Throw uMutexFailure::EntryFailure( &serial, "mutex object has been destroyed" );
	} // if

#ifdef __U_PROFILER__
	if ( task.profileActive && uProfiler::uProfiler_registerMutexFunctionEntryTry ) { // task registered for profiling ?
	    (*uProfiler::uProfiler_registerMutexFunctionEntryTry)( uProfiler::profilerInstance, serial, task );
	} // if
#endif // __U_PROFILER__

	try {
	    // Polling in enter happens after properly setting values of mr and therefore, in the catch clause, it can
	    // be used to retore the mr value in the uSerial object.

	    prevSerial = &task.getSerial();		// save previous serial
	    task.setSerial( serial );			// set new serial
	    uDEBUG( nlevel = task.currSerialLevel += 1; )
	    serial.enter( mr, ml, mp );
	    acceptor = serial.lastAcceptor;
	    acceptorSuspended = acceptor != nullptr;
	    if ( acceptorSuspended ) {
		acceptor->acceptedCall = this;
	    } // if
	    serial.lastAcceptor = nullptr;			// avoid messing up subsequent mutex method invocation
	} catch( ... ) {				// UNDO EFFECTS OF FAILED SERIAL ENTRY
	    finalize( task );
	    if ( serial.lastAcceptor ) {
		serial.lastAcceptor->acceptedCall = nullptr; // the rendezvous did not materialize
		_Resume uMutexFailure::RendezvousFailure( &serial, "accepted call" ) _At *serial.lastAcceptor->currCoroutine; // acceptor is not initialized
		serial.lastAcceptor = nullptr;
	    } // if
	    serial.leave( mr );				// look at ~uSerialMember()
	    _Throw;
	} // try

	noUserOverride = true;

#ifdef __U_PROFILER__
	if ( task.profileActive && uProfiler::uProfiler_registerMutexFunctionEntryDone ) { // task registered for profiling ?
	    (*uProfiler::uProfiler_registerMutexFunctionEntryDone )( uProfiler::profilerInstance, serial, task );
	} // if
#endif // __U_PROFILER__
    } // uSerialMember::uSerialMember

    // Used in conjunction with macro uRendezvousAcceptor inside mutex types to determine if a rendezvous has ended.
    // null means rendezvous is ended; otherwise address of the rendezvous partner is returned.  In addition, calling
    // uRendezvousAcceptor has the side effect of cancelling the implicit resume of uMutexFailure::RendezvousFailure at
    // the acceptor, which allows a mutex member to terminate with an exception without informing the acceptor.

    uBaseTask *uSerialMember::uAcceptor() {
	uBaseTask *temp = acceptor;
	acceptor->acceptedCall = nullptr;
	acceptor = nullptr;
	return temp;
    } // uSerialMember::uAcceptor

    uSerialMember::~uSerialMember() {
	uBaseTask &task = uThisTask();			// optimization
	uSerial &serial = task.getSerial();		// get current serial

	finalize( task );
	if ( acceptor ) {				// not cancelled by uRendezvousAcceptor ?
	    acceptor->acceptedCall = nullptr;		// accepted mutex member terminates
	    // raise a concurrent exception at the acceptor
	    if ( std::__U_UNCAUGHT_EXCEPTION__() && noUserOverride && ! serial.notAlive && acceptorSuspended ) {
		// Return the acceptor only when the acceptor remains suspended and the mutex object has yet to be
		// destroyed, otherwise return a null reference.  Side-effect: prevent the kernel from resuming
		// (_Resume) a concurrent exception at the suspended acceptor if the rendezvous terminates with an
		// exception.

		noUserOverride = false;
		_Resume uMutexFailure::RendezvousFailure( &serial, "accepted call" ) _At *(( ! serial.notAlive && acceptorSuspended ) ? acceptor->currCoroutine : nullptr);
	    } // if
	} // if

#ifdef __U_PROFILER__
	if ( task.profileActive && uProfiler::uProfiler_registerMutexFunctionExit ) { // task registered for profiling ?
	    (*uProfiler::uProfiler_registerMutexFunctionExit)( uProfiler::profilerInstance, serial, task );
	} // if
#endif // __U_PROFILER__

	serial.leave( mr );
    } // uSerialMember::~uSerialMember
} // UPP


uCondition::~uCondition() {
    // A uCondition object must be destroyed before its owner.  Concurrent execution of the destructor for a uCondition
    // and its owner is unacceptable.  The flag owner->notAlive tells if a mutex object is destroyed or not but it
    // cannot protect against concurrent execution.  As long as uCondition objects are declared inside its owner mutex
    // object, the proper order of destruction is guaranteed.

    if ( ! waiting.empty() ) {
	// wake each task blocked on the condition with an async event
	for ( ;; ) {
	    uBaseTaskDL *p = waiting.head();		// get the task blocked at the start of the condition
	  if ( p == nullptr ) break;			// list empty ?
	    uEHM::uDeliverEStack dummy( false );	// block all async exceptions in destructor
	    uBaseTask &task = p->task();
	    _Resume WaitingFailure( *this, "found blocked task on condition variable during deletion" ) _At *(task.currCoroutine); // throw async event at blocked task
	    signalBlock();				// restart (signal) the blocked task
	} // for
    } // if
} // uCondition::~uCondition


uDEBUG(
#define uConditionMsg( operation ) \
    "Attempt to " operation " a condition variable for a mutex object not locked by this task.\n" \
    "Possible cause is accessing the condition variable outside of a mutex member for the mutex object owning the variable."
)


void uCondition::wait() {				// wait on a condition
    uBaseTask &task = uThisTask();			// optimization
    UPP::uSerial &serial = task.getSerial();

    uDEBUG(
	if ( owner != nullptr && &task != owner->mutexOwner ) { // must have mutex lock to wait
	    abort( uConditionMsg( "wait on" ) );
	} // if
    )
    if ( owner != &serial ) {				// only owner can use condition
	if ( owner == nullptr ) {				// owner exist ?
	    owner = &serial;				// set condition owner
	} // if
    } // if

#ifdef __U_PROFILER__
    if ( task.profileActive && uProfiler::uProfiler_registerWait ) { // task registered for profiling ?
	(*uProfiler::uProfiler_registerWait)( uProfiler::profilerInstance, *this, task, serial );
    } // if
#endif // __U_PROFILER__

    waiting.add( &(task.mutexRef) );			// add to end of condition queue

    serial.leave2();					// release mutex and let it schedule another task

    _Enable <uMutexFailure><WaitingFailure>;		// implicit poll

#ifdef __U_PROFILER__
    if ( task.profileActive && uProfiler::uProfiler_registerReady ) { // task registered for profiling ?
	(*uProfiler::uProfiler_registerReady)( uProfiler::profilerInstance, *this, task, serial );
    } // if
#endif // __U_PROFILER__
} // uCondition::wait


uDEBUG(
#define uSignalCheck() \
    /* must have mutex lock to signal */ \
    if ( owner != nullptr && &task != owner->mutexOwner ) abort( uConditionMsg( "signal" ) );
)


bool uCondition::signal() {				// signal a condition
  if ( waiting.empty() ) return false;			// signal on empty condition is no-op

    uBaseTask &task = uThisTask();			// optimization
    UPP::uSerial &serial = task.getSerial();
    uDEBUG( uSignalCheck(); )

#ifdef __U_PROFILER__
    if ( task.profileActive && uProfiler::uProfiler_registerSignal ) { // task registered for profiling ?
	(*uProfiler::uProfiler_registerSignal)( uProfiler::profilerInstance, *this, task, serial );
    } // if
#endif // __U_PROFILER__

    serial.acceptSignalled.add( waiting.drop() );	// move signalled task on top of accept/signalled stack
    return true;
} // uCondition::signal


bool uCondition::signalBlock() {			// signal a condition
    if ( waiting.empty() ) return false;		// signal on empty condition is no-op

    uBaseTask &task = uThisTask();			// optimization
    UPP::uSerial &serial = task.getSerial();
    uDEBUG( uSignalCheck(); )

#ifdef __U_PROFILER__
    if ( task.profileActive && uProfiler::uProfiler_registerSignal ) { // task registered for profiling ?
	(*uProfiler::uProfiler_registerSignal)( uProfiler::profilerInstance, *this, task, serial );
    } // if
    if ( task.profileActive && uProfiler::uProfiler_registerWait ) { // task registered for profiling ?
	(*uProfiler::uProfiler_registerWait)( uProfiler::profilerInstance, *this, task, serial );
    } // if
#endif // __U_PROFILER__

    serial.acceptSignalled.add( &(task.mutexRef) );	// suspend signaller task on accept/signalled stack
    serial.acceptSignalled.addHead( waiting.drop() );	// move signalled task on head of accept/signalled stack
    serial.leave2();					// release mutex and let it schedule the signalled task

#ifdef __U_PROFILER__
    if ( task.profileActive && uProfiler::uProfiler_registerReady ) { // task registered for profiling ?
	(*uProfiler::uProfiler_registerReady)( uProfiler::profilerInstance, *this, task, serial );
    } // if
#endif // __U_PROFILER__
    return true;
} // uCondition::signalBlock


uCondition::WaitingFailure::WaitingFailure( const uCondition &cond, const char *const msg ) : uKernelFailure( msg ), cond( cond ) {}

uCondition::WaitingFailure::~WaitingFailure() {}

const uCondition &uCondition::WaitingFailure::conditionId() const { return cond; }

void uCondition::WaitingFailure::defaultTerminate() const {
    abort( "(uCondition &)%p : Waiting failure as task %.256s (%p) found blocked task %.256s (%p) on condition variable during deletion.",
	    &conditionId(), sourceName(), &source(), uThisTask().getName(), &uThisTask() );
} // uCondition::WaitingFailure::defaultTerminate


//######################### uMain #########################


// Needed for friendship with uKernelModule and uBaseTask because using uMain is too broad.
// This routine is still accessible in UPP, but I can't seem to hide it further.

inline void UPP::umainProfile() {
#ifdef __U_PROFILER__
    // task uMain is always profiled when the profiler is active
    uThisTask().profileActivate( *uKernelModule::bootTask ); // make boot task the parent
#endif // __U_PROFILER__
} // umainProfile

uMain::uMain( int argc, char *argv[], char *env[], int &retcode ) : uPthreadable( uMainStackSize() ), argc( argc ), argv( argv ), env( env ), uRetCode( retcode ) {
    setName( "main" );					// pretend to be user's uCpp_main
    umainProfile();
} // uMain::uMain


uMain::~uMain() {
    pthread_delete_kernel_threads_();
} // uMain::~uMain


//######################### Kernel Boot #########################


void UPP::uKernelBoot::startup() {
    if ( ! uKernelModule::kernelModuleInitialized ) {
	uKernelModule::startup();
    } // if

    RealRtn::startup();					// must come before any call to uDebugPrt

    uHeapControl::startup();

    // Force dynamic loader to (pre)load and initialize all the code to use C printf I/O. The stack depth to do this
    // initialization is significant as it includes all the calls to locale and can easily overflow the small stack of a
    // task.

    char dummy[16];
    sprintf( dummy, "dummy%d\n", 6 );			// force dynamic loading for this and associated routines

    uMachContext::pageSize = sysconf( _SC_PAGESIZE );

    // Cause floating-point errors to signal SIGFPE, which is handled by uC++ signal handler.

#if defined( __solaris__ )
    fpsetmask( FP_X_INV | FP_X_OFL | FP_X_UFL | FP_X_DZ ); // raise floating-point signal
    // TEMPORARY, do not add FP_X_IMP, there is a bug in "sin" (maybe other trig routines)
    // fpsetmask( FP_X_IMP );
#else	// Linux
    feenableexcept( FE_ALL_EXCEPT );			// raise floating-point signal
    // TEMPORARY removal, there is a bug in "sin" (maybe other trig routines)
    fedisableexcept( FE_INEXACT );

    // Store modified control registers as default values for register virtualization.
#if defined( __i386__ ) || defined( __x86_64__ )
    asm volatile ( "fnstcw %0" : "=m" (uMachContext::fncw) );
    asm volatile ( "stmxcsr %0" : "=m" (uMachContext::mxcsr) );
#endif // __i386__ || __x86_64__

#if defined( __ia64__ )
    // TEMPORARY removal, there is a bug in "__builtin_clz"
    fedisableexcept( FE_UPWARD );
    fedisableexcept( FE_DOWNWARD );
#endif // __ia64__
#endif // __solaris__

    // create kernel locks

    uKernelModule::globalAbortLock = new uSpinLock;
    uKernelModule::globalProcessorLock = new uSpinLock;
    uKernelModule::globalClusterLock = new uSpinLock;

    uDEBUGPRT( uDebugPrt( "uKernelBoot::startup1, disableInt:%d, disableIntCnt:%d\n",
			  THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ) ); )

    // initialize kernel signal handlers

    UPP::uSigHandlerModule();

    // create global lists

    uKernelModule::globalProcessors = new uProcessorSeq;
    uKernelModule::globalClusters = new uClusterSeq;

    // SKULLDUGGERY: Initialize the global pointers with the appropriate memory locations and then "new" the cluster and
    // processor. Because the global pointers are initialized, all the references through them in the cluster and
    // processor constructors work out. (HA!)

    uKernelModule::systemProcessor = (uProcessor *)&uKernelModule::systemProcessorStorage;
    uKernelModule::systemCluster = (uCluster *)&uKernelModule::systemClusterStorage;
    uKernelModule::systemScheduler = new uDefaultScheduler;

#if ! defined( __U_MULTI__ )
    uProcessor::contextSwitchHandler = new uCxtSwtchHndlr;
    uProcessor::contextEvent = new uEventNode( *uProcessor::contextSwitchHandler );
    uCluster::NBIO = new uNBIO;
#endif // ! __U_MULTI__

    uProcessor::events = new uEventList;

    // create system cluster: it is at a fixed address so storing the result is unnecessary.

    new( &uKernelModule::systemClusterStorage ) uCluster( *uKernelModule::systemScheduler, uDefaultStackSize(), "systemCluster" );
    new( &uKernelModule::systemProcessorStorage ) uProcessor( *uKernelModule::systemCluster, 1.0 );

    // create processor kernel

    uProcessorKernel *pk = new uProcessorKernel;
    THREAD_SETMEM( processorKernelStorage, pk );

    // set thread register in the new context

#if defined( __U_MULTI__ ) && defined( __U_SWAPCONTEXT__ )
#   if defined( __linux__ ) && defined( __ia64__ )
	((ucontext_t *)THREAD_GETMEM( processorKernelStorage )->context)->uc_mcontext.sc_gr[13] = THREAD_GETMEM( threadPointer );
#   else
	#error uC++ : internal error, unsupported architecture
#   endif
#endif // __U_MULTI__ && __U_SWAPCONTEXT__

    // start boot task, which executes the global constructors and destructors

    uKernelModule::bootTask = new( &uKernelModule::bootTaskStorage ) uBootTask();

    // SKULLDUGGERY: Set the processor's last resumer to the boot task so it returns to it when the processor coroutine
    // terminates. This has to be done explicitly because the kernel coroutine is never resumed only context switched
    // to. Therefore, uLast is never set by resume, and subsequently copied to uStart.

    activeProcessorKernel->last = uKernelModule::bootTask;

    // SKULLDUGGERY: Force a context switch to the system processor to set the boot task's context to the current UNIX
    // context. Hence, the boot task does not begin through uInvoke, like all other tasks. It also starts the system
    // processor's companion task so that it is ready to receive processor specific requests. The trick here is that
    // uBootTask is on the ready queue when this call is made. Normally, a task is not on a ready queue when it is
    // running. As result, there has to be a special check in schedule to not assert that this task is NOT on a ready
    // queue when it starts the context switch.

    uThisTask().uYieldNoPoll();

    // create system task

    uKernelModule::systemTask = new uSystemTask();


    // THE SYSTEM IS NOW COMPLETELY RUNNING


    // Obtain the addresses of the original set_terminate and set_unexpected using the dynamic loader. Use the original
    // versions to initialize the hidden variables holding the terminate and unexpected routines. All other references
    // to set_terminate and set_unexpected refer to the uC++ ones.

    RealRtn::set_terminate( uEHM::terminateHandler );
    RealRtn::set_unexpected( uEHM::unexpectedHandler );

    // create user cluster

    uKernelModule::userCluster = new uCluster( "userCluster" );

    // create user processors

    uKernelModule::numUserProcessors = uDefaultProcessors();
    uDEBUG(
	if ( uKernelModule::numUserProcessors == 0 ) {
	    abort( "uDefaultProcessors must return a value 1 or greater" );
	} // if
    )
    uKernelModule::userProcessors = new uProcessor*[ uKernelModule::numUserProcessors ];
    uKernelModule::userProcessors[0] = new uProcessor( *uKernelModule::userCluster );

    // uOwnerLock has a runtime check testing if locking is attempted from inside the kernel. This check only applies
    // once the system becomes concurrent. During the previous boot-strapping code, some locks may be invoked (and hence
    // a runtime check would occur) but the system is not concurrent. Hence, these locks are always open and no blocking
    // can occur. This flag enables uOwnerLock checking after this point.
    uDEBUG( uKernelModule::initialized = true; )

    uDEBUGPRT( uDebugPrt( "uKernelBoot::startup2, disableInt:%d, disableIntCnt:%d, uPreemption:%d\n",
			  THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ), uThisProcessor().getPreemption() ); )

    uKernelModule::bootTask->migrate( *uKernelModule::userCluster );

    // reset filebuf for default streams

    uKernelModule::cerrFilebuf = new std::filebuf( 2, 0 ); // unbufferred
    std::cerr.rdbuf( uKernelModule::cerrFilebuf );
    uKernelModule::clogFilebuf = new std::filebuf( 2 );
    std::clog.rdbuf( uKernelModule::clogFilebuf );
    uKernelModule::coutFilebuf = new std::filebuf( 1 );
    std::cout.rdbuf( uKernelModule::coutFilebuf );
    uKernelModule::cinFilebuf  = new std::filebuf( 0 );
    std::cin.rdbuf( uKernelModule::cinFilebuf );

    uDEBUGPRT( uDebugPrt( "uKernelBoot::startup3, disableInt:%d, disableIntCnt:%d, uPreemption:%d\n",
			  THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ), uThisProcessor().getPreemption() ); )
} // uKernelBoot::startup


void UPP::uKernelBoot::finishup() {
    uDEBUGPRT( uDebugPrt( "uKernelBoot::finishup1, disableInt:%d, disableIntCnt:%d, uPreemption:%d\n",
			  THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ), uThisProcessor().getPreemption() ); )

    // Flush standard output streams as required by 27.4.2.1.6

    delete uKernelModule::cinFilebuf;
    std::cout.flush();
    std::cout.rdbuf( nullptr );
    delete uKernelModule::coutFilebuf;
    std::clog.rdbuf( nullptr );
    delete uKernelModule::clogFilebuf;
    std::cerr.flush();
    std::cerr.rdbuf( nullptr );
    delete uKernelModule::cerrFilebuf;

    // If afterMain is false, control has reached here due to an "exit" call before the end of uMain::main. In this
    // case, all user destructors have been executed (which is potentially dangerous as external user-tasks may be
    // running). To prevent triggering errors, shutdown is now terminated, and any remaining destructors are executed.
    if ( ! uKernelModule::afterMain ) return;

    uKernelModule::bootTask->migrate( *uKernelModule::systemCluster );

    uDEBUGPRT( uDebugPrt( "uKernelBoot::finishup2, disableInt:%d, disableIntCnt:%d, uPreemption:%d\n",
			  THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ), uThisProcessor().getPreemption() ); )

    delete uKernelModule::userProcessors[0];
    delete [] uKernelModule::userProcessors;
    delete uKernelModule::userCluster;

    delete uKernelModule::systemTask;
    uKernelModule::systemTask = nullptr;

    // Turn off uOwnerLock checking.
    uDEBUG( uKernelModule::initialized = false; )

    THREAD_GETMEM( This )->disableInterrupts();
    // Block all signals as system can no longer handle them, and there may be pending timer values.
    if ( sigprocmask( SIG_BLOCK, &UPP::uSigHandlerModule::block_mask, nullptr ) == -1 ) {
	abort( "internal error, sigprocmask" );
    } // if
    THREAD_GETMEM( This )->enableInterrupts();

    // SKULLDUGGERY: The termination order for the boot task is different from the starting order. This results from the
    // fact that the boot task must have a processor before it can start. However, the system processor must have the
    // thread from the boot task to terminate. Deleting the cluster first requires that the boot task be removed first
    // from the list of tasks on the cluster or the cluster complains about unfinished tasks. The boot task must be
    // added again before its deletion because it removes itself from the list. Also, when the boot task is being
    // deleted it is using the *already* deleted system cluster, which works only because the system cluster storage is
    // not dynamically allocated so the storage is not scrubbed; therefore, it still has the necessary state values to
    // allow the boot task to be deleted. As well, the ready queue has to be allocated spearately from the system
    // cluster so it can be deleted *after* the boot task is deleted because the boot task access the ready queue during
    // its deletion.

    // remove the boot task so the cluster does not complain
    uKernelModule::systemCluster->taskRemove( *(uBaseTask *)uKernelModule::bootTask );

    // remove system processor, processor task and cluster
    uKernelModule::systemProcessor->uProcessor::~uProcessor();
    uKernelModule::systemCluster->uCluster::~uCluster();

#ifndef __U_MULTI__
    delete uCluster::NBIO;
    delete uProcessor::contextEvent;
    delete uProcessor::contextSwitchHandler;
#endif // ! __U_MULTI__

    delete uProcessor::events;

    // remove processor kernal coroutine with execution still pending
    delete THREAD_GETMEM( processorKernelStorage );

    // add the boot task back so it can remove itself from the list
    uKernelModule::systemCluster->taskAdd( *(uBaseTask *)uKernelModule::bootTask );
    ((uBootTask *)uKernelModule::bootTask)->uBootTask::~uBootTask();

    // Clean up storage associated with the boot task by pthread-like thread-specific data (see ~uTaskMain). Must occur
    // *after* all calls that might call std::uncaught_exceptions, otherwise the exception data-structures are created
    // again for the task. (Note: ~uSerialX members call std::uncaught_exceptions).

    if ( ((uBootTask *)uKernelModule::bootTask)->pthreadData != nullptr ) {
	pthread_deletespecific_( ((uBootTask *)uKernelModule::bootTask)->pthreadData );
    } // if

    // Clean up storage associated with pthread pid table (if any).

    pthread_pid_destroy_();

    // no tasks on the ready queue so it can be deleted
    delete uKernelModule::systemScheduler;

    uDEBUGPRT( uDebugPrt( "uKernelBoot::finishup3, disableInt:%d, disableIntCnt:%d, uPreemption:%d\n",
			  THREAD_GETMEM( disableInt ), THREAD_GETMEM( disableIntCnt ), uThisProcessor().getPreemption() ); )

    delete uKernelModule::globalClusters;
    delete uKernelModule::globalProcessors;

    delete uKernelModule::globalClusterLock;
    delete uKernelModule::globalProcessorLock;
    delete uKernelModule::globalAbortLock;

    uHeapControl::finishup();
} // uKernelBoot::finishup


void uInitProcessorsBoot::startup() {
    for ( unsigned int i = 1; i < uKernelModule::numUserProcessors; i += 1 ) {
	uKernelModule::userProcessors[i] = new uProcessor( *uKernelModule::userCluster );
    } // for
} // uInitProcessorsBoot::startup


void uInitProcessorsBoot::finishup() {
    for ( unsigned int i = 1; i < uKernelModule::numUserProcessors; i += 1 ) {
	delete uKernelModule::userProcessors[i];
    } // for
} // uInitProcessorsBoot::finishup


// Local Variables: //
// compile-command: "make install" //
// End: //
