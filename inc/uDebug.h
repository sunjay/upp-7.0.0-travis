//                              -*- Mode: C++ -*- 
// 
// uC++ Version 7.0.0, Copyright (C) Peter A. Buhr 1994
// 
// uDebug.h -- 
// 
// Author           : Peter A. Buhr
// Created On       : Sat Dec 18 12:56:10 1993
// Last Modified By : Peter A. Buhr
// Last Modified On : Sat Feb 18 18:45:35 2017
// Update Count     : 51
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


#ifndef __U_DEBUG_H__
#define __U_DEBUG_H__

extern "C" void uDebugWrite( int fd, const char *buffer, int len );
extern "C" void uDebugAcquire();
extern "C" void uDebugRelease();
extern "C" void uDebugPrt( const char fmt[], ... ) __attribute__(( format (printf, 1, 2) ));
extern "C" void uDebugPrt2( const char fmt[], ... ) __attribute__(( format (printf, 1, 2) ));
extern "C" void uDebugPrtBuf( char buffer[], const char fmt[], ... ) __attribute__(( format (printf, 2, 3) ));

#endif // __U_DEBUG_H__


// Local Variables: //
// compile-command: "make install" //
// End: //
