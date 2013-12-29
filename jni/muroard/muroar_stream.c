//muroar_stream.c:

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

// Open a Stream
muroar_t muroar_stream(muroar_t fh, int dir, int * stream, int codec, int rate, int channels, int bits) {
 unsigned char buf[24];
 uint16_t sid;
 uint32_t * data = (uint32_t*)buf;
 int i;

 if ( fh == MUROAR_HANDLE_INVALID ) {
  _SET_ERRNO(EBADF);
  return MUROAR_HANDLE_INVALID;
 }

 // Send NEW_STREAM command:
 memset(buf, 0, 10);
 buf[1] = MUROAR_CMD_NEW_STREAM;
 buf[2] = 0xFF; // byte 2 and 3 are mixer ID (must be -1)
 buf[3] = 0xFF;
 buf[9] = 24;   // 6 * int32 = 24 Byte

 if ( muroar_write(fh, buf, 10) != 10 ) {
  _CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 data[0] = dir;      // Stream Direction
 data[1] = -1;       // Rel Pos ID
 data[2] = rate;     // Sample Rate
 data[3] = bits;     // Bits per Sample
 data[4] = channels; // Number of Channels
 data[5] = codec;    // Used Codec

 for (i = 0; i < 6; i++)
  data[i] = htonl(data[i]);

 if ( muroar_write(fh, buf, 24) != 24 ) {
  _CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 if ( muroar_read(fh, buf, 10) != 10 ) {
  _CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 if ( buf[1] != MUROAR_CMD_OK ) {
  _CLOSE(fh);
  _SET_ERRNO(EINVAL);
  return MUROAR_HANDLE_INVALID;
 }

 // Stream ID of new stream is in byte 2 and 3 of header,
 // encoded in network byte order.
 sid = (buf[2] << 8) | buf[3];

 // Send EXEC_STREAM command:
 memset(buf, 0, 10);
 buf[1] = MUROAR_CMD_EXEC_STREAM;

 // set Stream ID:
 buf[2] = (sid & 0xFF00) >> 8;
 buf[3] = (sid & 0x00FF);

 if ( muroar_write(fh, buf, 10) != 10 ) {
  _CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 if ( muroar_read(fh, buf, 10) != 10 ) {
  _CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 if ( buf[1] != MUROAR_CMD_OK ) {
  _CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 // Set Stream ID in case caller want to know (passed non-NULL):
 if ( stream != NULL )
  *stream = sid;

 // we converted the control connection to a stream connection,
 // return it.
 return fh;
}

//ll
