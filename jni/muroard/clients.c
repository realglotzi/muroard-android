//clients.c:

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
#ifdef MUROARD_FEATURE_CMD_GETTIMEOFDAY
#include <time.h>
#endif

#define _FILTER_ANY 0

int muroard_client_init(void) {
 memset(muroard_state_member(client), 0, sizeof(muroard_state_member(client))); // state unused = 0!

#ifdef MUROARD_FEATURE_INTERNAL_CLIENT
 muroard_state_member(client)[0].state  = CLIENT_STATE_INTERNAL;
 muroard_state_member(client)[0].sock   = MUROARD_HANDLE_INVALID;
 muroard_state_member(client)[0].stream = -1;
#endif

 return 0;
}

int muroard_client_free(void) {
 int i;
 int ret = 0;

 for (i = 0; i < MUROARD_MAX_CLIENTS; i++) {
  if ( muroard_state_member(client)[i].state != CLIENT_STATE_UNUSED ) {
   if ( muroard_client_delete(i) == -1 )
    ret = -1;
  }
 }

 return ret;
}

int muroard_client_new(muroard_handle_t sock) {
 int i;

 if ( muroard_network_nonblock(sock, 0) == -1 )
  return -1;

 for (i = 0; i < MUROARD_MAX_CLIENTS; i++) {
  if ( muroard_state_member(client)[i].state == CLIENT_STATE_UNUSED ) {
   muroard_state_member(client)[i].state  = CLIENT_STATE_NEW;
   muroard_state_member(client)[i].sock   = sock;
   muroard_state_member(client)[i].stream = -1;
   return i;
  }
 }

 return -1;
}

int muroard_client_delete(int id) {
 int ret = 0;

 if ( id >= MUROARD_MAX_CLIENTS || id < 0 )
  return -1;

#ifdef MUROARD_FEATURE_INTERNAL_CLIENT
 if ( muroard_state_member(client)[id].state == CLIENT_STATE_INTERNAL  )
  return 0;
#endif

 if ( muroard_state_member(client)[id].state == CLIENT_STATE_UNUSED   ||
      muroard_state_member(client)[id].state == CLIENT_STATE_CLOSING   )
  return 0;

 muroard_state_member(client)[id].state = CLIENT_STATE_CLOSING;

 if ( muroard_state_member(client)[id].stream != -1 ) {
  if ( muroard_stream_delete(muroard_state_member(client)[id].stream) == -1 )
   ret = -1;
 }

 if ( muroard_state_member(client)[id].sock != MUROARD_HANDLE_INVALID ) {
  muroard_network_close(muroard_state_member(client)[id].sock);
 }

 muroard_state_member(client)[id].state = CLIENT_STATE_UNUSED;

 return ret;
}

muroard_handle_t muroard_client_exec(int id) {
 muroard_handle_t sock = muroard_state_member(client)[id].sock;

 muroard_state_member(client)[id].sock  = MUROARD_HANDLE_INVALID;
 muroard_state_member(client)[id].state = CLIENT_STATE_EXECED;

 return sock;
}

int muroard_client_handle(int id) {
 struct muroard_message m;
 muroard_handle_t sock = muroard_state_member(client)[id].sock;
 int ret  = 0;
#ifdef MUROARD_FEATURE_VOLUME_CONTROL
 uint16_t volume;
 size_t   channels;
 uint16_t tmp;
#endif

 if ( muroard_proto_recv(sock, &m) == -1 ) {
  muroard_client_delete(id);
  return -1;
 }

 if ( m.stream > MUROARD_MAX_STREAMS && m.stream != (uint16_t)-1 ) {
  // FIXME: this code is redundant :(
  m.cmd     = MUROAR_CMD_ERROR;
  m.stream  = -1;
  m.pos     =  0;
  m.datalen =  0;

  if ( muroard_proto_send(sock, &m) == -1 ) {
   muroard_client_delete(id);
   return -1;
  }
  return 0;
 }

 switch (m.cmd) {
  case MUROAR_CMD_NOOP:
  case MUROAR_CMD_IDENTIFY:
  case MUROAR_CMD_AUTH:
#ifdef MUROARD_FEATURE_NOOP_SIUCMDS
  case MUROAR_CMD_SET_META:
#endif
    // no need to do anything
    m.datalen =  0;
   break;
  case MUROAR_CMD_QUIT:
    m.cmd = MUROAR_CMD_OK;
    m.datalen =  0;
    muroard_proto_send(sock, &m);
    muroard_client_delete(id);
    return 0;
   break;
  case MUROAR_CMD_EXEC_STREAM:
    m.datalen =  0;
    if ( m.stream != (uint16_t)-1 && m.stream == muroard_state_member(client)[id].stream ) {
     if ( muroard_stream_exec(m.stream) == -1 )
      ret = -1;
    } else {
     ret = -1;
    }
   break;
#ifdef MUROARD_FEATURE_CMD_WHOAMI
  case MUROAR_CMD_WHOAMI:
    m.datalen =  1;
    m.data[0] =  id;
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_EXIT
  case MUROAR_CMD_EXIT:
    // we simply set muroard_state_member(alive) to zero so we will die with the next cycle.
    if ( m.datalen == 0 ) { // default: exit
     muroard_state_member(alive)  = 0;
    } else if ( m.datalen == 1 && m.data[0] == 0 ) { // explicit: exit
     muroard_state_member(alive)  = 0;
#ifndef MUROARD_FEATURE_TERMINATE
    } else if ( m.datalen == 1 && m.data[0] == 1 ) { // explicit: terminate
     ret = muroard_network_prefree();
#endif
    } else { // terminate or unsupported command version
     ret      = -1;
    }
    m.datalen = 0;
   break;
#endif
#ifdef MUROARD_FEATURE_STANDBY
  case MUROAR_CMD_SET_STANDBY:
    if ( m.datalen != 2 ) {
     ret = -1;
    } else {
     if ( m.data[0] != 0 ) {
      ret = -1;
     } else {
      switch (m.data[1]) {
       case 0: muroard_waveform_standby(0, -1); break;
       case 1: muroard_waveform_standby(1, -1); break;
       default: ret = -1;
      }
     }
    }
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_GET_STANDBY
  case MUROAR_CMD_GET_STANDBY:
    m.datalen = 2;
    m.data[0] = 0;
#ifdef MUROARD_FEATURE_STANDBY
    m.data[1] = muroard_waveform_standby(-1, -1);
#else
    m.data[1] = 0;
#endif
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_GET_VOL
  case MUROAR_CMD_GET_VOL:
    // we support only the most up to date version of the command:
    if ( m.datalen < 2 || m.data[0] != 0 || m.data[1] != 1 || m.stream == (uint16_t)-1 ) {
     ret = -1;
    } else {
     m.datalen = 6*2;
     // version: 1
     m.data[ 0] = 0;
     m.data[ 1] = 1;
     // channels: 1
     m.data[ 2] = 0;
     m.data[ 3] = 1;
     // scale:
     m.data[ 4] = 0xFF;
     m.data[ 5] = 0xFF;
     // rpg mul:
     m.data[ 6] = 0;
     m.data[ 7] = 1;
     // rpg div:
     m.data[ 8] = 0;
     m.data[ 9] = 1;
     // our only channel:
#ifdef MUROARD_FEATURE_VOLUME_CONTROL
     volume = muroard_stream_get_volume(m.stream);
     m.data[10] = (volume & 0xFF00) >> 8;
     m.data[11] =  volume & 0x00FF;
#else
     m.data[10] = 0xFF;
     m.data[11] = 0xFF;
#endif
    }
   break;
#endif
#ifdef MUROARD_FEATURE_VOLUME_CONTROL
  case MUROAR_CMD_SET_VOL:
    if ( m.datalen < (4*2) || m.data[0] != 0 || m.data[1] != 1 || !muroard_stream_exist(m.stream) ) {
     ret = -1;
    } else {
     channels = (m.datalen - 3*2)/2;
     volume = 0;
     tmp = ntohs(((uint16_t*)m.data)[2]);
     ret = 0;
     switch (tmp) {
      case 1: // ROAR_SET_VOL_ALL
      case 4: // ROAR_SET_VOL_UNMAPPED
        switch (channels) {
         case 1: // mono
           volume = ntohs(((uint16_t*)m.data)[3]);
          break;
         case 2: // stereo
           volume = ((uint_least32_t)ntohs(((uint16_t*)m.data)[3]) + (uint_least32_t)ntohs(((uint16_t*)m.data)[4]))/2;
//           printf("l=%u, r=%u, avg=%u\n", (unsigned)ntohs(((uint16_t*)m.data)[3]), (unsigned)ntohs(((uint16_t*)m.data)[4]), (unsigned)volume);
          break;
         default:
           ret = -1;
          break;
        }
       break;
      case 3: // ROAR_SET_VOL_MS
        if ( channels == 1 ) {
         volume = ntohs(((uint16_t*)m.data)[3]);
        } else {
         ret = -1;
        }
       break;
      default:
        ret = -1;
       break;
     }

//     printf("ret=%i, volume=%u\n", ret, (unsigned)volume);
     tmp    = ntohs(((uint16_t*)m.data)[1]);
     volume = (float)volume*65535.f/(float)tmp;
//     printf("tmp=%u, volume=%u\n", (unsigned)tmp, (unsigned)volume);
     muroard_state_member(stream)[m.stream].volume = volume;
    }
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_KICK
  case MUROAR_CMD_KICK:
    if ( m.datalen != 4 ) {
     ret = -1;
    } else {
     if ( m.data[0] != 0 ) { // type >= 256
      ret = -1;
     } else {
      if ( m.data[2] != 0 ) { // id >= 256
       ret = -1;
      } else if ( m.data[1] == 1 ) { // CLIENT
       ret = muroard_client_delete(m.data[3]);
      } else if ( m.data[1] == 2 ) { // STREAM
       ret = muroard_stream_delete(m.data[3]);
      } else { // unknown type
       ret = -1;
      }
     }
    }
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_ATTACH
  case MUROAR_CMD_ATTACH:
    ret = muroard_client_handle_attach(id, &m);
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_SERVER_OINFO
  case MUROAR_CMD_SERVER_OINFO:
    ret = muroard_client_handle_server_oinfo(id, &m);
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_PASSFH
  case MUROAR_CMD_PASSFH:
    ret = muroard_client_handle_passfh(id, &m);
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_LIST_CLIENTS
  case MUROAR_CMD_LIST_CLIENTS:
    ret = muroard_client_handle_list_clients(id, &m);
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_LIST_STREAMS
  case MUROAR_CMD_LIST_STREAMS:
    ret = muroard_client_handle_list_streams(id, &m);
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_GET_CLIENT
  case MUROAR_CMD_GET_CLIENT:
    ret = muroard_client_handle_get_client(id, &m);
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_GET_STREAM
  case MUROAR_CMD_GET_STREAM:
    ret = muroard_client_handle_get_stream(id, &m);
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_CAPS
  case MUROAR_CMD_CAPS:
    ret = muroard_client_handle_caps(id, &m);
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_SERVER_INFO
  case MUROAR_CMD_SERVER_INFO:
    ret = muroard_client_handle_server_info(id, &m);
   break;
#endif
#ifdef MUROARD_FEATURE_CMD_GETTIMEOFDAY
  case MUROAR_CMD_GETTIMEOFDAY:
    ret = muroard_client_handle_gettimeofday(id, &m);
   break;
#endif
  case MUROAR_CMD_NEW_STREAM:
    ret = muroard_client_handle_new_stream(id, &m);
   break;
  default:
    ret = -1; // we do not know this command
 }

 if ( ret == 0 ) {
  m.cmd     = MUROAR_CMD_OK;
 } else {
  m.cmd     = MUROAR_CMD_ERROR;
  m.stream  = -1;
  m.pos     =  0;
  m.datalen =  0;
 }

 if ( muroard_proto_send(sock, &m) == -1 ) {
  muroard_client_delete(id);
  return -1;
 }

 return 0;
}

int muroard_client_handle_new_stream(int id, struct muroard_message * mes) {
 struct muroard_audio_info info;
 uint32_t * data = (uint32_t*)mes->data;
 int ret;
 int i;

 if ( mes->datalen != (6*4) )
  return -1;

 mes->datalen = 0;

 for (i = 0; i < 6; i++) {
  data[i] = ntohl(data[i]);
 }

 if ( data[1] != (uint32_t)-1 )
  return -1;

 if ( data[2] != (uint32_t)muroard_state_member(sa_rate) )
  return -1;

 info.bits     = data[3];
 info.channels = data[4];
 info.codec    = data[5];

 ret = muroard_stream_new(id, data[0], &info);

 if ( ret == -1 )
  return -1;

 mes->stream = ret;

 return 0;
}

#ifdef MUROARD_FEATURE_CMD_ATTACH
int muroard_client_handle_attach(int id, struct muroard_message * mes) {
 int client;

 (void)id; // we do not need tis parameter

 if ( mes->datalen != 6 )
  return -1;

 // we do a quick and dirty check:
 // 0) 16 bit version must be zero
 // 1) we only support SIMPLE attach, const 16 bit, too.
 // 2) upper 8 bit of client must be zero (we do not support > 256 clients!)

 if ( mes->data[0] != 0 || mes->data[1] != 0 ||
      mes->data[2] != 0 || mes->data[3] != 1 ||
      mes->data[4] != 0                       )
  return -1;

 client = mes->data[5];

 if ( client < 0 )
  return -1;

 if ( (client + 1) > MUROARD_MAX_CLIENTS )
  return -1;

 if ( muroard_state_member(client)[client].state != CLIENT_STATE_NEW      &&
#ifdef MUROARD_FEATURE_INTERNAL_CLIENT
      muroard_state_member(client)[client].state != CLIENT_STATE_INTERNAL &&
#endif
      muroard_state_member(client)[client].state != CLIENT_STATE_OLD       )
  return -1;

 if ( muroard_state_member(client)[client].stream != -1 )
  return -1;

 if ( muroard_stream_move_client(mes->stream, client) == -1 )
  return -1;

#ifdef MUROARD_FEATURE_INTERNAL_CLIENT
 if ( muroard_state_member(client)[client].state != CLIENT_STATE_INTERNAL )
#endif
  muroard_state_member(client)[client].stream = mes->stream;

 mes->datalen = 0;

 return 0;
}
#endif

#ifdef MUROARD_FEATURE_CMD_SERVER_OINFO
int muroard_client_handle_server_oinfo(int id, struct muroard_message * mes) {
 uint32_t * data = (uint32_t*) mes->data;
 int        i;

 (void)id; // we do not need this parameter

 if ( mes->datalen != 0 )
  return -1;

 mes->datalen = 6*4;
 mes->stream  =  -1;
 mes->pos     =  -1;

 data[0] =  6; // TODO: do not use magic number. this is ROAR_DIR_MIXING
 data[1] = -1;
 data[2] = muroard_state_member(sa_rate);
 data[3] = 16;
 data[4] = muroard_state_member(sa_channels);
 data[5] = MUROAR_CODEC_PCM;

 for (i = 0; i < 6; i++) {
  data[i] = htonl(data[i]);
 }

 return 0;
}
#endif

#ifdef MUROARD_FEATURE_CMD_PASSFH
int muroard_client_handle_passfh(int id, struct muroard_message * mes) {
 muroard_handle_t fh;

 mes->datalen = 0;

 fh = muroard_network_recvfh(muroard_state_member(client)[id].sock);

 if ( fh == MUROARD_HANDLE_INVALID )
  return -1;

 if ( muroard_network_nonblock(fh, 0) == -1 ) {
  muroard_network_close(fh);
  return -1;
 }

 if ( !muroard_stream_exist(mes->stream) ) {
  muroard_network_close(fh);
  return -1;
 }

 if ( muroard_stream_get_sock(mes->stream) != MUROARD_HANDLE_INVALID ) {
  muroard_network_close(fh);
  return -1;
 }

 if ( muroard_stream_set_sock(mes->stream, fh) == -1 ) {
  muroard_network_close(fh);
  return -1;
 }

 return 0;
}
#endif

#ifdef MUROARD_FEATURE_CMD_LIST_CLIENTS
int muroard_client_handle_list_clients(int id, struct muroard_message * mes) {
 int len = 0;
 int i;

 (void)id; // we do not need tis parameter

 if ( mes->datalen != 7 )
  return -1;

 if ( mes->data[0] != 0 )
  return -1;

 if ( mes->data[1] != _FILTER_ANY )
  return -1;

 for (i = 0; i < MUROARD_MAX_CLIENTS; i++) {
  if ( muroard_state_member(client)[i].state != CLIENT_STATE_UNUSED ) {
   mes->data[len++] = i;
  }
 }

 mes->datalen = len;

 return 0;
}
#endif
#ifdef MUROARD_FEATURE_CMD_LIST_STREAMS
int muroard_client_handle_list_streams(int id, struct muroard_message * mes) {
 int len = 0;
 int i;

 (void)id; // we do not need tis parameter

 if ( mes->datalen != 7 )
  return -1;

 if ( mes->data[0] != 0 )
  return -1;

 if ( mes->data[1] != _FILTER_ANY )
  return -1;

 for (i = 0; i < MUROARD_MAX_STREAMS; i++) {
  if ( muroard_state_member(stream)[i].state != STREAM_STATE_UNUSED ) {
   mes->data[len++] = i;
  }
 }

 mes->datalen = len;

 return 0;
}
#endif

#ifdef MUROARD_FEATURE_CMD_GET_CLIENT
int muroard_client_handle_get_client(int id, struct muroard_message * mes) {
 struct muroard_client * c;
 int client;
 int len = 0;

 (void)id; // we do not need tis parameter

 if ( mes->datalen != 1 )
  return -1;

 client = (unsigned char)mes->data[0];

 if ( client >= MUROARD_MAX_CLIENTS )
  return -1;

 if ( (c = &(muroard_state_member(client)[client]))->state == CLIENT_STATE_UNUSED )
  return -1;

 mes->data[len++] = 0; // version

 if ( c->stream == -1 ) {
  mes->data[len++] = -1; // not execed because we do not have a stream
 } else {
  if ( muroard_state_member(stream)[c->stream].state == STREAM_STATE_EXECED ) {
   mes->data[len++] = c->stream;
  } else {
   mes->data[len++] = -1; // not execed
  }
 }

 if ( c->stream == -1 ) {
  mes->data[len++] = 0; // no streams
 } else {
  mes->data[len++] = 1; // one stream
  mes->data[len++] = c->stream;
 }

 mes->data[len++] = 1; // name has size of one byte
#ifdef MUROARD_FEATURE_GET_CLIENT_DUMMYNAME
 snprintf(&(mes->data[len]), sizeof(mes->data) - len, "Client %i", client);
 mes->data[len - 1] = strlen(&(mes->data[len])) + 1;
 len += mes->data[len - 1] - 1;
#endif
 mes->data[len++] = 0; // \0

 memset(&(mes->data[len]), 0xFF, 3*4); // PID, UID, GID
 len += 3*4;

 // Protocol: RoarAudio = 1, 32 bit network byte order
 mes->data[len++] = 0;
 mes->data[len++] = 0;
 mes->data[len++] = 0;
 mes->data[len++] = 1;

 // Byte order we use: Network byte order = 0x02, 32 bit network byte order
 mes->data[len++] = 0;
 mes->data[len++] = 0;
 mes->data[len++] = 0;
 mes->data[len++] = 0x02;

 mes->datalen = len;

 return 0;
}
#endif
#ifdef MUROARD_FEATURE_CMD_GET_STREAM
int muroard_client_handle_get_stream(int id, struct muroard_message * mes) {
 struct muroard_stream * s;
 uint32_t * data = (uint32_t*) mes->data;
 int        i;

 (void)id; // we do not need tis parameter

 if ( !muroard_stream_exist(mes->stream) ) {
  return -1;
 }

 s = &(muroard_state_member(stream)[mes->stream]);

 //fprintf(stderr, "%s:%i: dir=%i\n", __FILE__, __LINE__, s->dir);

 mes->datalen = 6*4;
#ifdef MUROARD_FEATURE_POSITION
 mes->pos     = s->pos;
#else
 mes->pos     = -1;
#endif

 data[0] = s->dir;
 data[1] = -1;
 data[2] = muroard_state_member(sa_rate);
 data[3] = s->info.bits;
 data[4] = s->info.channels;
 data[5] = s->info.codec;

 for (i = 0; i < 6; i++) {
  data[i] = htonl(data[i]);
 }

 return 0;
}
#endif

#ifdef MUROARD_FEATURE_CMD_CAPS
#if BYTE_ORDER == LITTLE_ENDIAN || defined(__WIN32)
#define _HTONS(x) ( \
                   (((uint32_t)(x) & 0x000000FF) << 24) | \
                   (((uint32_t)(x) & 0x0000FF00) <<  8) | \
                   (((uint32_t)(x) & 0x00FF0000) >>  8) | \
                   (((uint32_t)(x) & 0xFF000000) >> 24)   \
                  )
#elif BYTE_ORDER == BIG_ENDIAN
#define _HTONS(x) (x)
#else
#error CAPS support not supported for your byte order
#endif
#define _MKSTD(vendor,standard,version) _HTONS(((((uint32_t)(vendor)   & 0x00FF) << 24) | \
                                                (((uint32_t)(standard) & 0xFFFF) <<  8) | \
                                                 ((uint32_t)(version)  & 0x00FF)        ))

#define _RA 0

#define _TERM 0xFFFFFFFF
#define _TERM_SIZE 4
static const uint32_t _caps_standards[] = {
 _MKSTD(_RA, 0, 0), // Version 0 messages
#ifdef MUROARD_FEATURE_SOCKET_INET
 _MKSTD(_RA, 25, 0),
#endif
#ifdef MUROARD_FEATURE_SOCKET_UNIX
 _MKSTD(_RA, 23, 0),
#endif
 _TERM
};

int muroard_client_handle_caps(int id, struct muroard_message * mes) {
 uint16_t flags;

 (void)id;

 // check if header is complet.
 if ( mes->datalen < 4 )
  return -1;

 // check if version is supported.
 if ( mes->data[0] != 0 )
  return -1;

 // check if type is supported.
 if ( mes->data[1] != MUROAR_CT_STANDARDS )
  return -1;

 flags = ntohs(((uint16_t*)mes->data)[1]);

 if ( !(flags & MUROAR_CF_REQUEST) ) {
  mes->datalen = 0;
  return 0;
 }

 mes->datalen = 4 + (sizeof(_caps_standards) - _TERM_SIZE);

 if ( mes->datalen > sizeof(mes->data) )
  return -1;

 // clear flags:
 mes->data[2] = 0;
 mes->data[3] = 0;

 memcpy(mes->data + 4, _caps_standards, sizeof(_caps_standards) - _TERM_SIZE);

 return 0;
}
#endif

#ifdef MUROARD_FEATURE_CMD_SERVER_INFO
int muroard_client_handle_server_info(int id, struct muroard_message * mes) {
 const char * version = MUROARD_VERSION;
 const size_t len = strlen(version);

 (void)id, (void)mes;

 // test length:
 if ( mes->datalen != 4 )
  return -1;

 // test version:
 if ( mes->data[0] != 0 || mes->data[1] != 0 )
  return -1;

 // test type (0x0001 = IT_SERVER):
 if ( mes->data[2] != 0 || mes->data[3] != 1 )
  return -1;

 mes->datalen = 8 + len;
 // version:
 mes->data[0] = 0x00;
 // reserved:
 mes->data[1] = 0x00;
 // indexlen:
 mes->data[2] = 0x00;
 mes->data[3] = 0x01;

 // index 0:
 // - reserved
 mes->data[4] = 0x00;
 // - type (0x00 = ITST_VERSION)
 mes->data[5] = 0x00;
 // - len
 mes->data[6] = (len & 0xFF00) >> 8;
 mes->data[7] = (len & 0x00FF);

 memcpy(&(mes->data[8]), version, len);

 return 0;
}
#endif

#ifdef MUROARD_FEATURE_CMD_GETTIMEOFDAY
int muroard_client_handle_gettimeofday(int id, struct muroard_message * mes) {
 uint32_t * data = (uint32_t *)mes->data;
 int32_t now = time(NULL);

 (void)id;

 mes->datalen = 8*4;

 data[0] = 0;
 data[1] = 0;
 data[2] = now < 0 ? 0xFFFFFFFFLU : 0LU;
 data[3] = htonl((uint32_t)now);
 data[4] = 0;
 data[5] = 0;
 data[6] = 0;
 data[7] = htonl(1000000000UL);

 return 0;
}
#endif

//ll
