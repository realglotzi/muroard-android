//muroar_quit.c:

/*
 *      Copyright (C) Philipp 'ph3-der-loewe' Schafft - 2012-2013
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
#include "private.h"

// Close a control connection by sending QUIT command.
int muroar_quit   (muroar_t fh) {
 char quit[] = "\0\6\0\0\0\0\0\0\0\0"; // QUIT command
 int ret = 0;

 if ( fh == MUROAR_HANDLE_INVALID ) {
  _SET_ERRNO(EBADF);
  return -1;
 }

 if ( muroar_write(fh, quit, 10) != 10 )
  ret = -1;

 // read in case the server response
 // ignore errors as the server do not necessary
 // response to our request.
 muroar_read(fh, quit, 10);

 if ( muroar_close(fh) == -1 )
  ret = -1;

 return ret;
}

//ll
