//cf_au.c:

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

#if defined(MUROARD_FEATURE_CODECFILTER) && defined(MUROARD_FEATURE_CODECFILTER_AU)
static void muroard_cf_au_read_extra_header (int id, struct muroard_stream * stream) {
 char buf[128];
 ssize_t ret;
 size_t size;

 while ( stream->cf_data.au.dataoffset ) {
  size = (size_t)stream->cf_data.au.dataoffset > sizeof(buf) ? sizeof(buf) : (size_t)stream->cf_data.au.dataoffset;
  ret = muroard_stream_network_read(id, buf, size);
  if ( ret < 1 )
   return;
  stream->cf_data.au.dataoffset -= ret;
 }
}

ssize_t muroard_cf_au_read(int id, struct muroard_stream * stream, void * buf, size_t len) {
 uint32_t data[6];
 unsigned int i;
 int processed_header = 0;

 if ( !stream->cf_data.au.headeroffset ) {
  processed_header = 1;
  stream->cf_data.au.dataoffset = 24;
  stream->cf_data.au.headeroffset = muroard_stream_network_read(id, data, sizeof(data));
  if ( stream->cf_data.au.headeroffset != stream->cf_data.au.dataoffset )
   return -1;

  for (i = 0; i < (sizeof(data)/sizeof(*data)); i++)
   data[i] = ntohl(data[i]);

  if ( data[0] != 0x2e736e64 ) /* magic, ".snd" */
   return -1;

  if ( data[1] < (uint32_t)stream->cf_data.au.dataoffset )
   return -1;
  stream->cf_data.au.dataoffset  = data[1];
  stream->cf_data.au.dataoffset -= stream->cf_data.au.headeroffset;

  if ( data[4] != (uint32_t)muroard_state_member(sa_rate) )
   return -1;
  stream->info.channels = data[5];

  switch (data[3]) {
   case  1: stream->info.bits =  8; stream->info.codec = MUROAR_CODEC_MULAW;    break;
   case  2: stream->info.bits =  8; stream->info.codec = MUROAR_CODEC_PCM_S;    break;
   case  3: stream->info.bits = 16; stream->info.codec = MUROAR_CODEC_PCM_S_BE; break;
   case  4: stream->info.bits = 24; stream->info.codec = MUROAR_CODEC_PCM_S_BE; break;
   case  5: stream->info.bits = 32; stream->info.codec = MUROAR_CODEC_PCM_S_BE; break;
   case 27: stream->info.bits =  8; stream->info.codec = MUROAR_CODEC_ALAW;     break;
   default: return -1; break;
  }

  if ( muroard_stream_cksupport(&(stream->info), STREAM_DATADIR_IN) != 1 )
   return -1;
 }

 if ( stream->cf_data.au.dataoffset ) {
  processed_header = 1;
  muroard_cf_au_read_extra_header(id, stream);
 }

 if ( stream->cf_data.au.dataoffset || processed_header ) {
  *(char*)buf = 0;
  return 1;
 }

 return muroard_stream_network_read(id, buf, len);
}
#endif

//ll
