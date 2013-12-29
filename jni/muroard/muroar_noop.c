//muroar_noop.c:

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

#include "muroar.h"

// Set errno in case we have it
#ifndef __WIN32
#include <errno.h>
#define _SET_ERRNO(x) (errno = (x))
#else
#define _SET_ERRNO(x)
#endif

int muroar_noop   (muroar_t fh) {
 unsigned char msg[10] = "\0\0\0\0\0\0\0\0\0\0";

 if ( fh == MUROAR_HANDLE_INVALID ) {
  _SET_ERRNO(EBADF);
  return -1;
 }

 if ( muroar_write(fh, msg, 10) != 10 )
  return -1;

 if ( muroar_read(fh, msg, 10) != 10 )
  return -1;

 if ( msg[1] != MUROAR_CMD_OK )
  return -1;

 return 0;
}

//ll
