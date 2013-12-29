//cf_riff_wave.c:

/*
 *      Copyright (C) Philipp 'ph3-der-loewe' Schafft - 2011-2013
 *
 *  This file is part of µRoarD,
 *  a sound server daemon for using the RoarAudio protocol.
 *  See README for details.
 *
 *  This file is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3
 *  or (at your option) any later version as published by
 *  the Free Software Foundation.
 *
 *  RoarAudio is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this software; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "muroard.h"

#if defined(MUROARD_FEATURE_CODECFILTER) && defined(MUROARD_FEATURE_CODECFILTER_RIFF_WAVE)
ssize_t muroard_cf_riff_wave_read(int id, struct muroard_stream * stream, void * buf, size_t len) {
 unsigned char header[44];
 uint32_t tmp;

 (void)len;

 if ( muroard_stream_network_read(id, header, 44) != 44 )
  return -1;

 if ( strncmp((char*)header+ 0, "RIFF", 4) != 0 ||
      strncmp((char*)header+ 8, "WAVE", 4) != 0 ||
      strncmp((char*)header+12, "fmt ", 4) != 0 ||
      strncmp((char*)header+36, "data", 4) != 0 )
  return -1;

 tmp = (uint32_t)header[20] + ((uint32_t)header[21] << 8);
 switch (tmp) {
  case 0x0001: stream->info.codec = MUROAR_CODEC_PCM_S_LE; break;
  case 0x0006: stream->info.codec = MUROAR_CODEC_ALAW;     break;
  case 0x0007: stream->info.codec = MUROAR_CODEC_MULAW;    break;
  default:
    return -1;
   break;
 }


 tmp = (uint32_t)header[22] + ((uint32_t)header[23] << 8);
 stream->info.channels = tmp;
 tmp = (uint32_t)header[24] + ((uint32_t)header[25] << 8) +
       ((uint32_t)header[26] << 16) + ((uint32_t)header[27] << 24);
 if ( tmp != (uint32_t)muroard_state_member(sa_rate) )
  return -1;

 tmp = (uint32_t)header[34] + ((uint32_t)header[35] << 8);
 stream->info.bits = tmp;

 if ( muroard_stream_cksupport(&(stream->info), STREAM_DATADIR_IN) != 1 )
  return -1;

 stream->cf_read = NULL;

 *(char*)buf = 0;
 return 1;
}
#endif

//ll
