//streams.h:

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

#ifndef _MUROARD_STREAMS_H_
#define _MUROARD_STREAMS_H_

#define STREAM_STATE_UNUSED    0
#define STREAM_STATE_NEW       1
#define STREAM_STATE_OLD       2
#define STREAM_STATE_EXECED    3
#define STREAM_STATE_CLOSING   4

#define STREAM_DATADIR_IN      1
#define STREAM_DATADIR_OUT     2

struct muroard_audio_info {
 int channels;
 int bits;
 int codec;
};

struct muroard_stream {
 // general data and IO:
 muroard_handle_t sock;
 int state;
 int client;
 int datadir;
 // Stream level data:
 int dir;
 // audio level data:
 struct muroard_audio_info info;
 // raw data:
 int16_t * iobuf;

 // stream volume:
#ifdef MUROARD_FEATURE_VOLUME_CONTROL
 uint16_t volume;
#endif

 // stream position:
#ifdef MUROARD_FEATURE_POSITION
 uint32_t pos;
#endif

#ifdef MUROARD_FEATURE_CODECFILTER
 ssize_t (*cf_read)(int id, struct muroard_stream * stream, void * buf, size_t len);
 int (*cf_close)(int id, struct muroard_stream * stream);
 union {
  void * vp; // we need a dummy to comply the C standard. So we use something some codec filter may find useful.
#ifdef MUROARD_FEATURE_CODECFILTER_AU
  struct {
   ssize_t dataoffset, headeroffset;
  } au;
#endif
#ifdef MUROARD_FEATURE_CODECFILTER_OGG_VORBIS
  struct {
   int opened;
   int stream_id;
   OggVorbis_File vf;
  } ogg_vorbis;
#endif
 } cf_data;
#endif
};

int muroard_stream_init(void);
int muroard_stream_free(void);
int muroard_stream_new(int client, int dir, struct muroard_audio_info * info);
int muroard_stream_delete(int id);
int muroard_stream_exec(int id);

int muroard_stream_read(int id);
#ifdef MUROARD_FEATURE_MONITOR
int muroard_stream_write(int id, int16_t * buf);
#endif

#ifdef MUROARD_FEATURE_CMD_ATTACH
int muroard_stream_move_client(int id, int client);
#endif

#ifdef MUROARD_FEATURE_CODECFILTER
int muroard_stream_cksupport(struct muroard_audio_info * info, int datadir);
#endif

#ifdef MUROARD_FEATURE_SOURCES
int muroard_stream_new_source(const char * filename, int dir, struct muroard_audio_info * info);
#endif

#define muroard_stream_exist(id)       (((uint16_t)(id)) == (uint16_t)-1 || muroard_state_member(stream)[(id)].state == STREAM_STATE_UNUSED ?  0   : 1)
#define muroard_stream_get_sock(id)    (muroard_state_member(stream)[(id)].state == STREAM_STATE_UNUSED ? MUROARD_HANDLE_INVALID : muroard_state_member(stream)[(id)].sock)
#define muroard_stream_set_sock(id, s) (muroard_state_member(stream)[(id)].state == STREAM_STATE_UNUSED || muroard_state_member(stream)[(id)].sock != MUROARD_HANDLE_INVALID ? \
                                -1  : (muroard_state_member(stream)[(id)].sock = (s)) && 0)

#define muroard_stream_get_iobuf(id)   (muroard_state_member(stream)[(id)].state == STREAM_STATE_UNUSED ? NULL : muroard_state_member(stream)[(id)].iobuf)
#define muroard_stream_get_datadir(id) (muroard_state_member(stream)[(id)].state == STREAM_STATE_UNUSED ? -1   : muroard_state_member(stream)[(id)].datadir)

#ifdef MUROARD_FEATURE_VOLUME_CONTROL
#define muroard_stream_get_volume(id)  (muroard_state_member(stream)[(id)].state == STREAM_STATE_UNUSED ? -1   : muroard_state_member(stream)[(id)].volume)
#endif

#ifdef MUROARD_FEATURE_REREADWRITE
ssize_t muroard_stream_network_read(int id, void * buf, size_t len);
ssize_t muroard_stream_network_write(int id, const void * buf, size_t len);
#else
#define muroard_stream_network_read(id,buf,len) muroard_network_read(muroard_stream_get_sock((id)), (buf), (len))
#define muroard_stream_network_write(id,buf,len) muroard_network_write(muroard_stream_get_sock((id)), (buf), (len))
#endif

#endif

//ll
