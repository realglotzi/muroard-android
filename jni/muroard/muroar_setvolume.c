//muroar_setvolume.c:

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
// for memset():
#include <string.h>

int muroar_setvolume   (muroar_t fh, int stream, long unsigned int left, long unsigned int right) {
 unsigned char buf[20];
 unsigned char ans[10];
 int channels;
 int mode;
 size_t len;

 if ( fh == MUROAR_HANDLE_INVALID ) {
  _SET_ERRNO(EBADF);
  return -1;
 }

 if ( stream < 0 ) {
  _SET_ERRNO(EINVAL);
  return -1;
 }

 memset(buf, 0, sizeof(buf));

 // header:
 buf[1] = MUROAR_CMD_SET_VOL;
 buf[2] = (stream & 0xFF00) >> 8;
 buf[3] = (stream & 0x00FF);

 // body:
 buf[10] = 0;    // Version MSB
 buf[11] = 1;    // Version LSB
 buf[12] = 0xFF; // scale MSB
 buf[13] = 0xFF; // scale LSB
 buf[14] = 0;    // mode MSB

 for (channels = 2; channels > 0; channels--) {
  len = 10 + 6 + 2*channels; // total request length.
  buf[9] = len - 10; // body length LSB, the 10 is thhe header length

  for (mode = 4; mode > 0; mode -= 3) {
   buf[15] = mode; // mode LSB
   buf[16] = (left  & 0xFF00) >> 8; // channel 0 MSB
   buf[17] = (left  & 0x00FF);      // channel 0 LSB
   buf[18] = (right & 0xFF00) >> 8; // channel 1 MSB
   buf[19] = (right & 0x00FF);      // channel 1 LSB

   // send request:
   if ( muroar_write(fh, buf, len) != (ssize_t)len ) {
    return -1;
   }

   if ( muroar_read(fh, ans, 10) != 10 ) {
    return -1;
   }

   if ( ans[1] == MUROAR_CMD_OK )
    return 0;
  }

  left = left/2 + right/2;
 }

#ifdef ENOTSUP
 _SET_ERRNO(ENOTSUP);
#else
 _SET_ERRNO(ENOSYS);
#endif
 return -1;
}

//ll
