//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uBootTask.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Thu Apr 28 13:31:12 1994
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Aug 10 01:07:07 2012
// Update Count     : 143
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
#include <uBootTask.h>


namespace UPP {
    void uBootTask::main() {
	// NEVER INVOKED BECAUSE BOOT TASK IS ALREADY RUNNING USING THE UNIX THREAD.
    } // uBootTask::main


    void *uBootTask::operator new( size_t, void *storage ) {
	return storage;
    } // uBootTask::operator new


    void *uBootTask::operator new( size_t size ) {
	return ::operator new( size );
    } // uBootTask::operator new


    uBootTask::uBootTask() : uBaseTask( *uKernelModule::systemCluster ) {
	// SKULLDUGGERY: Explicitly make this task look like the currently executing task.

	THREAD_SETMEM( activeTask, this );

	// SKULLDUGGERY: Remove "main" from the suspend stack so it is never executed. This trick makes this task into a
	// pseudo-task to execute the global constructor and destructor lists.

	uSerialInstance.acceptSignalled.drop();

	// SKULLDUGGERY: Because this task has been removed from the suspend stack, it is not woken by
	// uSerialConstructor so do this manually by putting the task on the ready queue.

	wake();
    } // uBootTask::uBootTask


    uBootTask::~uBootTask() {
	// SKULLDUGGERY: Since this task does not have a "main" routine, some of its termination code must be performed
	// manually.

	notHalted = false;
    } // uBootTask::~uBootTask
} // UPP


// Local Variables: //
// compile-command: "make install" //
// End: //
