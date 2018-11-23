//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Richard C. Bilson 2004
// 
// uAtomic.h -- atomic routines for various processors
// 
// Author           : Richard C. Bilson
// Created On       : Thu Sep 16 13:57:26 2004
// Last Modified By : Peter A. Buhr
// Last Modified On : Thu Mar 23 21:08:59 2017
// Update Count     : 144
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

#ifndef __U_ATOMIC_H__
#define __U_ATOMIC_H__

#if defined( __ia64__ )
#include <ia64intrin.h>
#endif // __ia64__


#if defined( __i386__ ) || defined( __x86_64__ )
static inline unsigned long long int uRdtsc() {
    unsigned long long int result;
#if defined( __x86_64__ )
    asm volatile ( "rdtsc\n"
		   "salq $32, %%rdx\n"
		   "orq %%rdx, %%rax"
		   : "=a" (result) : : "rdx"
	);
#elif defined( __i386__ )
    asm volatile ( "rdtsc" : "=A" (result) );
#endif
    return result;
} // uRdtsc
#endif


static inline void uPause() {				// pause to prevent excess processor bus usage
#if defined( __sparc )
    __asm__ __volatile__ ( "rd %ccr,%g0" );
#elif defined( __i386 ) || defined( __x86_64 )
    __asm__ __volatile__ ( "pause" : : : );
#else
    // do nothing
#endif
} // uPause


template< typename T > static inline bool uTestSet( volatile T &lock ) {
    //return __sync_lock_test_and_set( &lock, 1 );
    return __atomic_test_and_set( &lock, __ATOMIC_ACQUIRE );
} // uTestSet

template< typename T > static inline void uTestReset( volatile T &lock ) {
    //__sync_lock_release( &lock );
    __atomic_clear( &lock, __ATOMIC_RELEASE );
} // uTestReset


template< typename T > static inline T uFetchAssign( volatile T &loc, T replacement ) {
    //return __sync_lock_test_and_set( &loc, replacement );
    return __atomic_exchange_n( &loc, replacement, __ATOMIC_ACQUIRE );
} // uFetchAssign


template< typename T > static inline T uFetchAdd( volatile T &counter, int increment ) {
    //return __sync_fetch_and_add( &counter, increment );
    return __atomic_fetch_add( &counter, increment, __ATOMIC_SEQ_CST );
} // uFetchAdd


template< typename T > static inline bool uCompareAssign( volatile T &loc, T comp, T replacement ) {
#if ( defined( GLIBCXX_ENABLE_ATOMIC_BUILTINS ) || defined( __ia64__ ) ) && defined( __sync_bool_compare_and_swap ) // broken macro, replace it
#if __U_WORDSIZE__ == 32
    return __sync_bool_compare_and_swap_si((int *)(void *)(&loc),(int)(comp),(int)(replacement));
#else
    return __sync_bool_compare_and_swap_di((long *)(void *)(&loc),(long)(comp),(long)(replacement));
#endif // __U_WORDSIZE__ == 32
#else
    //return __sync_bool_compare_and_swap( &loc, comp, replacement );
    return __atomic_compare_exchange_n( &loc, &comp, replacement, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
#endif
} // uCompareAssign


template< typename T > static inline bool uCompareAssignValue( volatile T &loc, T &comp, T replacement ) {
    return __atomic_compare_exchange_n( &loc, &comp, replacement, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST );
} // uCompareAssignValue


#endif // __U_ATOMIC_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
