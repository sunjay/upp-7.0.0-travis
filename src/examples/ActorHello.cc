//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2016
// 
// ActorHello.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Mon Dec 19 08:23:31 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Tue Dec 27 08:29:22 2016
// Update Count     : 6
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

#include <iostream>
using namespace std;
#include <uActor.h>


struct StrMsg : public uActor::Message {
    string val;
    StrMsg( string val ) : Message( uActor::Delete ), val( val ) {}
}; // StrMsg

_Actor Hello {
    Allocation receive( Message &msg ) {
	Case( StrMsg, msg ) {				// determine message kind
	    osacquire acq( cout );
	    cout << msg_t->val;				// access message value
	    if ( msg_t->val == "hello" ) {
		cout << ", hello back at you!" << endl;
	    } else {
		cout << ", do not understand?" << endl;
	    } // if
	} else Case( StopMsg, msg ) {
	    return Delete;				// delete actor
	} // Case
	return Nodelete;				// reuse actor
    } // Hello::receive
}; // Hello

int main() {
    *(new Hello()) | *new StrMsg( "hello" ) | *new StrMsg( "bonjour" ) | uActor::stopMsg;
    *(new Hello()) | *new StrMsg( "hello" ) | *new StrMsg( "buenos dias" ) | uActor::stopMsg;
    *(new Hello()) | *new StrMsg( "hello" ) | *new StrMsg( "你好" ) | uActor::stopMsg;
    uActor::stop();					// wait for all actors to terminate
} // main

// Local Variables: //
// compile-command: "u++-work -g -O2 -multi ActorHello.cc" //
// End: //
