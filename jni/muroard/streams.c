//streams.c:

/*
 *      Copyright (C) Philipp 'ph3-der-loewe' Schafft - 2010-2013
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

int muroard_stream_init(void) {
 muroard_memzero(muroard_state_member(stream), sizeof(muroard_state_member(stream))); // unused = 0
 return 0;
}

int muroard_stream_free(void) {
 int i;
 int ret = 0;

 for (i = 0; i < MUROARD_MAX_STREAMS; i++) {
  if ( muroard_state_member(stream)[i].state != STREAM_STATE_UNUSED ) {
   if ( muroard_stream_delete(i) == -1 )
    ret = -1;
  }
 }

 return ret;
}

int muroard_stream_new(int client, int dir, struct muroard_audio_info * info) {
#if defined(MUROARD_FEATURE_UPSCALE_INPUT) || defined(MUROARD_FEATURE_DOWNSCALE_OUTPUT)
 int need_scale = 0;
#define _NEED_SCALE need_scale
#endif
#if defined(MUROARD_FEATURE_UPMIX_INPUT) || defined(MUROARD_FEATURE_DOWNMIX_OUTPUT)
 int need_mix   = 0;
#define _NEED_MIX need_mix
#endif
#ifdef MUROARD_FEATURE_CODECFILTER
 int need_cf    = 0;
#define _NEED_CF need_cf
#endif
 register struct muroard_stream * stream;
 int i;

 //fprintf(stderr, "%s:%i: dir=%i\n", __FILE__, __LINE__, dir);

#ifdef _NEED_SCALE
 if ( info->bits == 8 ) {
  need_scale = 1;
  switch (info->codec) {
   case MUROAR_CODEC_PCM_S_LE:
   case MUROAR_CODEC_PCM_S_BE:
   case MUROAR_CODEC_PCM_S_PDP:
     info->codec = MUROAR_CODEC_PCM_S;
    break;
#ifdef MUROARD_FEATURE_UNSIGNED_8BIT
   case MUROAR_CODEC_PCM_U_LE:
   case MUROAR_CODEC_PCM_U_BE:
   case MUROAR_CODEC_PCM_U_PDP:
     info->codec = MUROAR_CODEC_PCM_U;
    break;
#endif
  }
 } else if ( info->bits != 16 ) {
  return -1;
 }
#else
 if ( info->bits != 16 )
  return -1;
#endif

#ifdef _NEED_MIX
 if ( info->channels == 1 ) {
  need_mix = 1;
 } else if ( info->channels != muroard_state_member(sa_channels) ) {
  return -1;
 }
#else
 if ( info->channels != muroard_state_member(sa_channels) )
  return -1;
#endif

#ifdef _NEED_CF
 switch (info->codec) {
#ifdef MUROARD_FEATURE_CODECFILTER_AU
  case MUROAR_CODEC_AU:
#endif
#ifdef MUROARD_FEATURE_CODECFILTER_RIFF_WAVE
  case MUROAR_CODEC_RIFF_WAVE:
#endif
#ifdef MUROARD_FEATURE_CODECFILTER_OGG_VORBIS
  case MUROAR_CODEC_OGG_VORBIS:
#endif
    _NEED_CF = info->codec;
    info->codec = MUROAR_CODEC_PCM;
   break;
 }
#endif

#ifdef MUROARD_FEATURE_BYTE_SWAP
 if ( info->bits > 16 && info->codec != MUROAR_CODEC_PCM )
  return -1;

 switch (info->codec) {
  case MUROAR_CODEC_PCM_S_BE:
  case MUROAR_CODEC_PCM_S_LE:
  case MUROAR_CODEC_PCM_S_PDP:
#ifdef MUROARD_FEATURE_UNSIGNED_8BIT
  case MUROAR_CODEC_PCM_U_LE:
  case MUROAR_CODEC_PCM_U_BE:
  case MUROAR_CODEC_PCM_U_PDP:
#endif
   break;
  default:
    return -1;
   break;
 }
#else
 if ( info->codec != MUROAR_CODEC_PCM )
  return -1;
#endif

 if ( client_get_stream(client) != -1 )
  return -1;

 for (i = 0; i < MUROARD_MAX_STREAMS; i++) {
  if ( muroard_state_member(stream)[i].state == STREAM_STATE_UNUSED ) {
   stream = &(muroard_state_member(stream)[i]);
   muroard_memzero(stream, sizeof(*stream));
   stream->state    = STREAM_STATE_UNUSED;
   stream->iobuf    = NULL;

#ifdef _NEED_CF
   stream->cf_read  = NULL;
   stream->cf_close = NULL;

   switch (_NEED_CF) {
    case 0: break; // no need to do anything.
#ifdef MUROARD_FEATURE_CODECFILTER_AU
    case MUROAR_CODEC_AU:
      stream->cf_read = muroard_cf_au_read;
     break;
#endif
#ifdef MUROARD_FEATURE_CODECFILTER_RIFF_WAVE
    case MUROAR_CODEC_RIFF_WAVE:
      stream->cf_read = muroard_cf_riff_wave_read;
     break;
#endif
#ifdef MUROARD_FEATURE_CODECFILTER_OGG_VORBIS
    case MUROAR_CODEC_OGG_VORBIS:
      stream->cf_read  = muroard_cf_ogg_vorbis_read;
      stream->cf_close = muroard_cf_ogg_vorbis_close;
     break;
#endif
    default: return -1;
   }
   if ( _NEED_CF ) {
#ifdef _NEED_SCALE
    _NEED_SCALE = 1;
    info->bits = 8;
#endif
#ifdef _NEED_MIX
    _NEED_MIX = 1;
    info->channels = 1;
#endif
   }
#endif

   switch (dir) {
    case MUROAR_PLAY_WAVE:
      stream->datadir = STREAM_DATADIR_IN;
      if ( (stream->iobuf = muroard_malloc(muroard_state_member(abuffer_size))) == NULL ) {
       return -1;
      }
      muroard_memzero(stream->iobuf, muroard_state_member(abuffer_size)); // clean memory to avoid noise
     break;
#ifdef MUROARD_FEATURE_MONITOR
    case MUROAR_MONITOR_WAVE:
       stream->datadir = STREAM_DATADIR_OUT;
      // no neet to set up anything
     break;
#endif
    default:
      return -1;
     break;
   }

#ifdef _NEED_SCALE
   if ( _NEED_SCALE ) {
    switch (stream->datadir) {
#ifdef MUROARD_FEATURE_UPSCALE_INPUT
     case STREAM_DATADIR_IN: break;
#endif
#ifdef MUROARD_FEATURE_DOWNSCALE_OUTPUT
     case STREAM_DATADIR_OUT: break;
#endif
     default:
       if ( stream->iobuf != NULL )
        muroard_free(stream->iobuf);
       return -1;
    }
   }
#endif

#ifdef _NEED_MIX
   if ( _NEED_MIX ) {
    switch (stream->datadir) {
#ifdef MUROARD_FEATURE_UPMIX_INPUT
     case STREAM_DATADIR_IN: break;
#endif
#ifdef MUROARD_FEATURE_DOWNMIX_OUTPUT
     case STREAM_DATADIR_OUT: break;
#endif
     default:
       if ( stream->iobuf != NULL )
        muroard_free(stream->iobuf);
       return -1;
    }
   }
#endif

   //fprintf(stderr, "%s:%i: dir=%i\n", __FILE__, __LINE__, dir);

   stream->sock   = MUROARD_HANDLE_INVALID;
   stream->client = client;
   stream->dir    = dir;

   // stream volume:
#ifdef MUROARD_FEATURE_VOLUME_CONTROL
   stream->volume = 65535;
#endif

   memcpy(&(stream->info), info, sizeof(stream->info));

   stream->state = STREAM_STATE_NEW;
   client_set_stream(client, i); // TODO: check errors here
   return i;
  }
 }

 return -1;
}

int muroard_stream_delete(int id) {
 register struct muroard_stream * stream;
 int state;
 int ret   = 0;

 if ( id >= MUROARD_MAX_STREAMS || id < 0 )
  return -1;

 stream = &(muroard_state_member(stream)[id]);
 state = stream->state;

 if ( state == STREAM_STATE_UNUSED  ||
      state == STREAM_STATE_CLOSING )
  return 0;

 stream->state = STREAM_STATE_CLOSING;

#ifdef MUROARD_FEATURE_CODECFILTER
 if ( stream->cf_close != NULL )
  stream->cf_close(id, stream);
#endif

 if ( state == STREAM_STATE_EXECED ) {
  if ( stream->client != -1 )
   if ( muroard_client_delete(stream->client) == -1 )
    ret = -1;
 } else {
  muroard_state_member(client)[stream->client].stream = -1;
 }

 if ( stream->sock != MUROARD_HANDLE_INVALID ) {
  muroard_network_close(stream->sock);
 }

 if ( stream->iobuf != NULL )
  muroard_free(stream->iobuf);

 stream->state = STREAM_STATE_UNUSED;

 return ret;
}

int muroard_stream_exec(int id) {
 muroard_handle_t sock;

 if ( (sock = muroard_client_exec(muroard_state_member(stream)[id].client)) == MUROARD_HANDLE_INVALID )
  return -1;

 muroard_state_member(stream)[id].sock  = sock;
 muroard_state_member(stream)[id].state = STREAM_STATE_EXECED;

 return 0;
}

#ifdef MUROARD_FEATURE_REREADWRITE
ssize_t muroard_stream_network_read(int id, void * buf, size_t len) {
 muroard_handle_t fh = muroard_stream_get_sock(id);
 ssize_t done = 0;
 ssize_t ret;

 if ( fh == MUROARD_HANDLE_INVALID || buf == NULL )
  return -1;

 while (len) {
  if ( (ret = muroard_network_read(fh, buf, len)) < 1 )
   return done;

  done += ret;
  buf   = (char *)buf + ret;
  len  -= ret;
 }

 return done;
}
ssize_t muroard_stream_network_write(int id, const void * buf, size_t len) {
 muroard_handle_t fh = muroard_stream_get_sock(id);
 ssize_t done = 0;
 ssize_t ret;

 if ( fh == MUROARD_HANDLE_INVALID || buf == NULL )
  return -1;

 while (len) {
  if ( (ret = muroard_network_write(fh, buf, len)) < 1 )
   return done;

  done += ret;
  buf   = (const char *)buf + ret;
  len  -= ret;
 }

 return done;
}
#endif

int muroard_stream_read(int id) {
 register struct muroard_stream * stream = &(muroard_state_member(stream)[id]);
 ssize_t len;
 size_t readsize = muroard_state_member(abuffer_size);

#ifdef MUROARD_FEATURE_UPSCALE_INPUT
 if ( stream->info.bits == 8 )
  readsize /= 2;
#endif

#ifdef MUROARD_FEATURE_UPMIX_INPUT
 if ( stream->info.channels == 1 && muroard_state_member(sa_channels) != 1 )
  readsize /= muroard_state_member(sa_channels);
#endif

#ifdef MUROARD_FEATURE_CODECFILTER
 if ( stream->cf_read != NULL ) {
  len = stream->cf_read(id, stream, stream->iobuf, readsize);
 } else {
#endif
  len = muroard_stream_network_read(id, stream->iobuf, readsize);
#ifdef MUROARD_FEATURE_CODECFILTER
 }
#endif

 if ( len == (ssize_t) readsize ) {
  // no op here.
 } else if ( len == 0 ) {
  return muroard_stream_delete(id);
 } else if ( len == -1 ) {
  muroard_stream_delete(id);
  return -1;
 } else {
  muroard_memzero(((void*)stream->iobuf)+len, muroard_state_member(abuffer_size) - len);
 }

#ifdef MUROARD_FEATURE_POSITION
 stream->pos += len / (stream->info.bits / 8);
#endif

#ifdef MUROARD_FEATURE_UPSCALE_INPUT
 if ( stream->info.bits == 8 ) {
  muroard_dsp_upscale(stream->iobuf, len);

  len *= 2;
 }
#endif

#ifdef MUROARD_FEATURE_BYTE_SWAP
 if ( muroard_dsp_need_swap_s(stream->info.codec) )
  muroard_dsp_swap16(stream->iobuf, len/2);
#endif

#ifdef MUROARD_FEATURE_UPMIX_INPUT
 if ( stream->info.channels == 1 && muroard_state_member(sa_channels) != 1 )
  muroard_dsp_upmix(stream->iobuf, len/2);
#endif

 return 0;
}

#ifdef MUROARD_FEATURE_MONITOR
int muroard_stream_write(int id, int16_t * buf) {
 register struct muroard_stream * stream = &(muroard_state_member(stream)[id]);
 ssize_t len;
 size_t  writesize = muroard_state_member(abuffer_size);
#if defined(MUROARD_FEATURE_DOWNSCALE_OUTPUT) || defined(MUROARD_FEATURE_DOWNMIX_OUTPUT) || defined(MUROARD_FEATURE_BYTE_SWAP)
 size_t   need_buf = 0;
#define _NEED_BUF need_buf
#endif

 if ( muroard_stream_get_sock(id) == MUROARD_HANDLE_INVALID )
  return -1;

#ifdef MUROARD_FEATURE_DOWNSCALE_OUTPUT
 if ( stream->info.bits == 8 ) {
  writesize /= 2;
  need_buf   = 1;
 }
#endif

#ifdef MUROARD_FEATURE_DOWNMIX_OUTPUT
 if ( stream->info.channels == 1 && muroard_state_member(sa_channels) != 1 ) {
  writesize /= muroard_state_member(sa_channels);
  need_buf   = 1;
 }
#endif

#ifdef MUROARD_FEATURE_BYTE_SWAP
 if ( muroard_dsp_need_swap_s(stream->info.codec) )
  need_buf   = 1;
#endif

#ifdef _NEED_BUF
 if ( _NEED_BUF ) {
  if ( stream->iobuf == NULL ) {
   muroard_state_member(stream)[id].iobuf = muroard_malloc(muroard_state_member(abuffer_size));
   if ( stream->iobuf == NULL )
    return -1;
  }

  memcpy(stream->iobuf, buf, muroard_state_member(abuffer_size));
  buf = muroard_state_member(stream)[id].iobuf;

#ifdef MUROARD_FEATURE_DOWNMIX_OUTPUT
  if ( stream->info.channels == 1 && muroard_state_member(sa_channels) != 1 ) {
   muroard_dsp_downmix(buf, muroard_state_member(abuffer_size)/(2*muroard_state_member(sa_channels)));
  }
#endif

#ifdef MUROARD_FEATURE_BYTE_SWAP
 if ( muroard_dsp_need_swap_s(stream->info.codec) )
  muroard_dsp_swap16(buf, (muroard_state_member(abuffer_size)*stream->info.channels)/(2*muroard_state_member(sa_channels)));
#endif

#ifdef MUROARD_FEATURE_DOWNSCALE_OUTPUT
  if ( stream->info.bits == 8 ) {
   muroard_dsp_downscale(buf, writesize);
  }
#endif
 }
#endif

 len = muroard_stream_network_write(id, buf, writesize);

 if ( len < 1 ) {
  muroard_stream_delete(id);
  return -1;
 }

#ifdef MUROARD_FEATURE_POSITION
 stream->pos += len / (stream->info.bits / 8);
#endif

 return 0;
}
#endif

#ifdef MUROARD_FEATURE_CMD_ATTACH
int muroard_stream_move_client(int id, int client) {
 int old_client;

 if ( muroard_state_member(stream)[id].state == STREAM_STATE_UNUSED )
  return -1;

 old_client = muroard_state_member(stream)[id].client;

 // if we are execed we need to close the client, too.
 // we use a little trick:
 // we tell the stream it is closing and close the client.
 // after the client has been closed we reset the stream to NEW.
 if ( muroard_state_member(stream)[id].state == STREAM_STATE_EXECED ) {
  muroard_state_member(stream)[id].state = STREAM_STATE_CLOSING;
  if ( muroard_client_delete(old_client) == -1 ) {
   muroard_state_member(stream)[id].state = STREAM_STATE_NEW;
   muroard_stream_delete(id);
   return -1;
  }
  old_client = -1;
  muroard_state_member(stream)[id].state = STREAM_STATE_NEW;
 }

 if ( old_client != -1 )
  muroard_state_member(client)[old_client].stream = -1;

 // now we have a free client we need to attach to the client like muroard_stream_new() does:
#ifdef MUROARD_FEATURE_INTERNAL_CLIENT
 if ( muroard_state_member(client)[client].state == CLIENT_STATE_INTERNAL ) {
  muroard_state_member(stream)[id].client = -1;
  return 0;
 }
#endif

 muroard_state_member(stream)[id].client = client;
 client_set_stream(client, id); // TODO: check errors here

 return 0;
}
#endif

#ifdef MUROARD_FEATURE_CODECFILTER
int muroard_stream_cksupport(struct muroard_audio_info * info, int datadir) {
 if ( info->bits == 8 ) {
  switch (info->codec) {
   case MUROAR_CODEC_PCM_S_LE:
   case MUROAR_CODEC_PCM_S_BE:
   case MUROAR_CODEC_PCM_S_PDP:
#ifdef MUROARD_FEATURE_UNSIGNED_8BIT
   case MUROAR_CODEC_PCM_U_LE:
   case MUROAR_CODEC_PCM_U_BE:
   case MUROAR_CODEC_PCM_U_PDP:
#endif
    break;
   default:
     return 0;
    break;
  }
 } else if ( info->bits == 16 ) {
  switch (info->codec) {
#if MUROAR_CODEC_PCM_S_LE == MUROAR_CODEC_PCM_S
#ifdef MUROARD_FEATURE_BYTE_SWAP
   case MUROAR_CODEC_PCM_S_BE:
#endif
   case MUROAR_CODEC_PCM_S_LE:
   case MUROAR_CODEC_PCM_S_PDP:
    break;
#else
#ifdef MUROARD_FEATURE_BYTE_SWAP
   case MUROAR_CODEC_PCM_S_LE:
   case MUROAR_CODEC_PCM_S_PDP:
#endif
   case MUROAR_CODEC_PCM_S_BE:
    break;
#endif
#ifdef MUROARD_FEATURE_UNSIGNED_16BIT
#if MUROAR_CODEC_PCM_S_LE == MUROAR_CODEC_PCM_S
#ifdef MUROARD_FEATURE_BYTE_SWAP
   case MUROAR_CODEC_PCM_U_BE:
#endif
   case MUROAR_CODEC_PCM_U_LE:
   case MUROAR_CODEC_PCM_U_PDP:
    break;
#else
#ifdef MUROARD_FEATURE_BYTE_SWAP
   case MUROAR_CODEC_PCM_U_LE:
   case MUROAR_CODEC_PCM_U_PDP:
#endif
   case MUROAR_CODEC_PCM_U_BE:
    break;
#endif
#endif
   default:
     return 0;
    break;
  }
 } else {
  return 0;
 }

 if ( info->bits != 16 ) {
  if ( info->bits == 8 ) {
   if ( datadir == STREAM_DATADIR_IN ) {
#ifndef MUROARD_FEATURE_UPSCALE_INPUT
    return 0;
#endif
   } else if ( datadir == STREAM_DATADIR_OUT ) {
#ifndef MUROARD_FEATURE_DOWNSCALE_OUTPUT
    return 0;
#endif
   } else {
    return 0;
   }
  }
 }

 if ( info->channels != muroard_state_member(sa_channels) ) {
  if ( info->channels == 1 ) {
   if ( datadir == STREAM_DATADIR_IN ) {
#ifndef MUROARD_FEATURE_UPMIX_INPUT
    return 0;
#endif
   } else if ( datadir == STREAM_DATADIR_OUT ) {
#ifndef MUROARD_FEATURE_DOWNMIX_OUTPUT
    return 0;
#endif
   } else {
    return 0;
   }
  } else {
   return 0;
  } 
 }

 return 1;
}
#endif

#ifdef MUROARD_FEATURE_SOURCES
int muroard_stream_new_source(const char * filename, int dir, struct muroard_audio_info * info) {
 int stream;
#ifndef __WIN32
 int flags = O_NONBLOCK;
#else
 int flags = 0;
#endif
 int fh;

 if ( dir == STREAM_DATADIR_IN ) {
  flags |= O_RDONLY;
 } else if ( dir == STREAM_DATADIR_OUT ) {
#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif
  flags |= O_WRONLY|O_CREAT|O_LARGEFILE|O_APPEND;
 } else {
 }

 fh = open(filename, flags, 0666);
 if ( fh == -1 )
  return -1;

 stream = muroard_stream_new(0, dir, info);
 if ( stream == -1 ) {
  close(fh);
  return -1;
 }

 if ( muroard_stream_move_client(stream, 0) == -1 ) {
  muroard_stream_delete(stream);
  return -1;
 }

 if ( muroard_stream_set_sock(stream, fh) == -1 ) {
  muroard_stream_delete(stream);
  return -1;
 }

 return stream;
}
#endif

//ll
