//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 2016
// 
// ActorProdCons.cc -- 
// 
// Author           : Peter A. Buhr
// Created On       : Wed Jul 25 13:05:45 2018
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Aug 23 07:51:42 2018
// Update Count     : 253
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

//#define NOOUTPUT

#ifdef NOOUTPUT
#define PRT( stmt )
#else
#define PRT( stmt ) stmt
#endif // NOOUTPUT

_Actor Cons;											// forward

template<typename T> _Actor Buffer {
  private:
	const size_t prods, cons;
	size_t cycle = 0, stops = 0;
	Cons ** consumers;									// manage consumers

	uActor::Allocation receive( uActor::Message & msg );
  public:
	struct valueMsg : public uActor::Message { T value; valueMsg( T value ) : Message( uActor::Delete ), value( value ) {} };

	Buffer( size_t prods, size_t cons );
	~Buffer() { delete consumers; }
}; // Buffer


_Actor Prod {
	Buffer<size_t> * buffer;
	const size_t times;									// generate N values

    Allocation receive( uActor::Message & msg ) {
		for ( size_t i = 0; i < times; i += 1 ) {
			//for ( volatile size_t delay = 0; delay < 100; delay += 1 );
			PRT( cout << "prod " << this << " " << i << endl; )
				*buffer | *new Buffer<size_t>::valueMsg{ i }; // insert value and send
		} // for
		*buffer | uActor::stopMsg;						// indicate completion
		return Delete;
	}
  public:
	Prod( Buffer<size_t> *buffer, size_t times ) : buffer( buffer ), times( times ) {}
}; // Prod

_Actor Cons {
	Buffer<size_t> * buffer;

    Buffer<size_t>::Allocation receive( uActor::Message & msg ) {
		Case( Buffer<size_t>::valueMsg, msg ) {					// receive value
			PRT( cout << "cons " << this << " remove " << msg_t->value << endl; )
			//for ( volatile size_t delay = 0; delay < 100; delay += 1 );
		} else Case( StopMsg, msg ) return Delete;		// stop consuming
		else abort( "Cons unknown message" );
		return Nodelete;								// reuse actor
	}
  public:
	Cons( Buffer<size_t> *buffer ) : buffer( buffer ) {}
}; // Cons


template<typename T> Buffer<T>::Buffer( size_t prods, size_t cons ) : prods( prods ), cons( cons ), consumers( new Cons *[cons] )  {
	for ( int i = 0; i < cons; i += 1 )	consumers[i] = new Cons( this ); // create consumers
} // Buffer::Buffer

template<typename T> uActor::Allocation Buffer<T>::receive( uActor::Message & msg ) {
	Case( valueMsg, msg ) {								// producer value ?
		cycle = (cycle + 1) % cons;
		*consumers[cycle] | *new valueMsg( msg_t->value ); // round-robin values to consumers
	} else Case( uActor::StopMsg, msg ) {				// producer stop ?
		stops += 1;
		if ( stops == prods ) {							// all producers finished, stop consumers
			for ( int i = 0; i < cons; i += 1 ) *consumers[i] | uActor::stopMsg;
			return uActor::Delete;
		} // if
	} else abort( "Buffer unknown message" );
	return uActor::Nodelete;							// reuse actor
}

void check( const char *arg, int &value ) {
	if ( strcmp( arg, "d" ) != 0 ) {					// user default ?
		value = stoi( arg );
		if ( value < 1 ) throw 1;
	} // if
} // check

int main( int argc, char *argv[] ) {
	int Prods = 1, Conss = 1, times = 10;
    try {
		switch ( argc ) {
		  case 4:
			check( argv[3], times );
		  case 3:
			check( argv[2], Conss );
		  case 2:
			check( argv[1], Prods );
		  case 1:										// use defaults
			break;
		  default:
			throw 1;
		} // switch
    } catch( ... ) {
		cout << "Usage: " << argv[0] << " [ prods (> 0) | 'd' [ cons (>0) | 'd' [ times (>0) ] | 'd' ] ]" << endl;
		exit( EXIT_SUCCESS );
    } // try
	cout << "producers " << Prods << " consumers " << Conss << " times " << times << endl;

	Buffer<size_t> *buffer = new Buffer<size_t>( Prods, Conss ); // buffer creates consumers
	for ( int i = 0; i < Prods; i += 1 ) {				// create and start producers
		*(new Prod( buffer, times ) ) | uActor::startMsg;
	} // for
    uActor::stop();										// wait for all actors to terminate
	malloc_stats();										// print memory statistics
} // main

// Local Variables: //
// tab-width: 4 //
// compile-command: "u++-work -g -O2 -multi ActorProdCons.cc" //
// End: //
