//cf_ogg_vorbis.c:

/*
 *      Copyright (C) Philipp 'ph3-der-loewe' Schafft - 2008-2013
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

#if defined(MUROARD_FEATURE_CODECFILTER) && defined(MUROARD_FEATURE_CODECFILTER_OGG_VORBIS)

#if ROAR_CODEC_DEFAULT == ROAR_CODEC_PCM_S_LE
#define _bigendianp 0
#define _codec MUROAR_CODEC_PCM
#else
#define _bigendianp 1
#define _codec MUROAR_CODEC_PCM_S_BE // force because ROAR_CODEC_DEFAULT can be something non-BE, too.
#endif

static int muroard_cf_ogg_vorbis_private_return_err (void) {
 return -1;
}

static size_t muroard_cf_ogg_vorbis_private_read (void *ptr, size_t size, size_t nmemb, void *datasource) {
 ssize_t r;

 r = muroard_stream_network_read(((struct muroard_stream *)datasource)->cf_data.ogg_vorbis.stream_id, ptr, size*nmemb);

 if ( r == -1 ) {
  return 0;
 }

 r /= size;

 return r;
}

static const ov_callbacks _callbacks = {
  .read_func  = muroard_cf_ogg_vorbis_private_read,
  .seek_func  = (int    (*)(void *, ogg_int64_t, int      )) muroard_cf_ogg_vorbis_private_return_err,
  .close_func = (int    (*)(void *                        )) muroard_cf_ogg_vorbis_private_return_err,
  .tell_func  = (long   (*)(void *                        )) muroard_cf_ogg_vorbis_private_return_err
};

ssize_t muroard_cf_ogg_vorbis_read(int id, struct muroard_stream * stream, void * buf, size_t len) {
 vorbis_info * vi;
 int current_section;
 long r;
 long todo = len;
 long done = 0;
 int ret;

 if ( !stream->cf_data.ogg_vorbis.opened ) {
  stream->cf_data.ogg_vorbis.stream_id = id;
  if ( (ret = ov_open_callbacks((void*)stream, &(stream->cf_data.ogg_vorbis.vf), NULL, 0, _callbacks)) < 0 ) {
   return -1;
  }

  vi = ov_info(&(stream->cf_data.ogg_vorbis.vf), -1);

  if ( vi->rate != muroard_state_member(sa_rate) ) {
   ov_clear(&(stream->cf_data.ogg_vorbis.vf));
   return -1;
  }

  stream->info.channels = vi->channels;
  stream->info.bits     = 16;
  stream->info.codec    = _codec;

  if ( muroard_stream_cksupport(&(stream->info), STREAM_DATADIR_IN) != 1 ) {
   ov_clear(&(stream->cf_data.ogg_vorbis.vf));
   return -1;
  }

  stream->cf_data.ogg_vorbis.opened = 1;
 }

 while (todo) {
  r = ov_read(&(stream->cf_data.ogg_vorbis.vf), buf+done, todo, _bigendianp, 2, 1, &current_section);
  if ( r == OV_HOLE ) {
  } else if ( r < 1 ) {
   break;
  } else {
   todo -= r;
   done += r;
  }
 }

 if ( done == 0 ) {
  // do some EOF handling...
  return 0;
 } else {
  return len;
 }
}

int     muroard_cf_ogg_vorbis_close(int id, struct muroard_stream * stream) {
 (void)id;
 if ( !stream->cf_data.ogg_vorbis.opened )
  return 0;

 ov_clear(&(stream->cf_data.ogg_vorbis.vf));
 return 0;
}
#endif

//ll
