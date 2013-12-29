//private.h:

/*
 *      Copyright (C) Philipp 'ph3-der-loewe' Schafft - 2009-2013
 *
 *  This file is part of µRoar,
 *  a minimalist library to access a RoarAudio Sound Server.
 *
 *  µRoar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  µRoar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with µRoar.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#include "muroar.h"

#ifdef __YIFF__
#include <YIFFc/string.h>
#include <YIFFc/proc.h>
#include <YIFFc/network.h>
#include <YIFFc/stdio.h>
#include <YIFFc/identifier.h>
#elif defined(__WIN32)
#include <windows.h>
#include <winsock2.h>
#else
#include <errno.h> /* for errno keeping in _CLOSE() */
#define _HAVE_ERRNO
#include <sys/socket.h>
#include <netinet/in.h>
#if !defined(__WIN32) && defined(AF_UNIX) && !defined(NO_AF_UNIX)
#define HAVE_AF_UNIX
#include <sys/un.h>
#endif
#include <netdb.h>
#endif

#ifdef HAVE_LIB_DNET
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#endif


// Win32 needs different close function for sockets because of
// it's broken design.
#ifdef __WIN32
#define __CLOSE(x) closesocket((x))
#elif MUROAR_HDLTYPE == MUROAR_HDLTYPE_YIFF_VIO
#define __CLOSE(x) { yiffc_error_t __olderr = yiffc_error; yiffc_id_unref((x)); yiffc_error = __olderr; }
#else
#define __CLOSE(x) { int __olderr = errno; close((x)); errno = __olderr; }
#endif

#if   MUROAR_HDLTYPE == MUROAR_HDLTYPE_POSIX_SYSIO
#define _CLOSE(x) __CLOSE((x))
#elif MUROAR_HDLTYPE == MUROAR_HDLTYPE_POSIX_STDIO
#define _CLOSE(x) { int __olderr = errno; fclose((x)); errno = __olderr; }
#elif MUROAR_HDLTYPE == MUROAR_HDLTYPE_WIN32_SOCKET
#define _CLOSE(x) __CLOSE((x))
#elif MUROAR_HDLTYPE == MUROAR_HDLTYPE_YIFF_VIO
#define _CLOSE(x) { yiffc_error_t __olderr = yiffc_error; yiffc_close((x)); yiffc_error = __olderr; }
#else
#error Unsupported value for MUROAR_HDLTYPE
#endif

// Set errno in case we have it
#ifdef _HAVE_ERRNO
#define _SET_ERRNO(x) (errno = (x))
#else
#define _SET_ERRNO(x)
#endif

#endif

//ll
