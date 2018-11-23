//                              -*- Mode: C++ -*- 
// 
// Copyright (C) Peter A. Buhr 2016
// 
// ActorInherit.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Fri Dec 23 17:05:06 2016
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Jan  6 09:36:47 2018
// Update Count     : 7
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

_Actor B {
    Allocation receive( Message & ) { return Delete; }
    int i;
  protected:
    void preStart() {
	osacquire( cout ) << "B" << endl;
    } // B::preStart
  public:
    B() {}
    B( int i ) { B::i = i; }
}; // B

_Actor D : public B {
    void preStart() {
	B::preStart();					// call base member
	osacquire( cout ) << "D" << endl;
    } // D::preStart
    Allocation receive( Message & ) { return Delete; }
  public:
    D() {}
    D( int i ) : B( i ) {}
}; // D

int main() {
    B *b = new B;
    D *d = new D;
    // Output is non-deterministic because actor b or d may may run first.
    *b | uActor::stopMsg;
    *d | uActor::stopMsg;
    uActor::stop();
} // main

// Local Variables: //
// compile-command: "u++-work -g -O2 -multi ActorInherit.cc" //
// End: //
