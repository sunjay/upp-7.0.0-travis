//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr and Thierry Delisle 2016
// 
// actor.cc -- 
// 
// Author           : Peter A. Buhr and Thierry Delisle
// Created On       : Mon Nov 14 22:41:44 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Fri Dec 22 13:43:53 2017
// Update Count     : 35
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
#include <uActor.h>


uExecutor uActor::executor_( uDefaultActorMailboxes(), uDefaultActorThreads(), uDefaultActorProcessors(), uDefaultActorOffset(), uDefaultActorSepClus() ); // executor for all actors
uSemaphore uActor::wait_( 0 );				// uMain::main waits for all actors to be destroyed
unsigned long int uActor::alive_ = 0;			// number of actor objects in system
uActor::StartMsg uActor::startMsg;			// start actor
uActor::StopMsg uActor::stopMsg;			// terminate actor
uActor::UnhandledMsg uActor::unhandledMsg;		// tell error


// Local Variables: //
// compile-command: "make install" //
// End: //
