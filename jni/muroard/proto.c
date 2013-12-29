//proto.c:

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
#if !defined(__WIN32) && !defined(HAVE_LIB_YIFFC)
#include <sched.h>
#endif

int muroard_proto_recv(muroard_handle_t sock, struct muroard_message * mes) {
 unsigned char header[10];
#ifndef __WIN32
 ssize_t ret = -1;
#endif

 if ( muroard_network_read(sock, header, 10) != 10 )
  return -1;

 if ( header[0] != 0 ) /* version */
  return -1;

 mes->cmd     =  header[1];
 mes->stream  = ((uint16_t)header[2] <<  8) +  (uint16_t)header[3];
 mes->pos     = ((uint32_t)header[4] << 24) + ((uint32_t)header[5] << 16) +
                ((uint32_t)header[6] <<  8) +  (uint32_t)header[7];
 mes->datalen = ((uint16_t)header[8] <<  8) +  (uint16_t)header[9];

// fprintf(stderr, "MSG IN : ver=0, cmd=%i, stream=%i, pos=%u, datalen=%u\n", mes->cmd, mes->stream, mes->pos, mes->datalen);

 if ( mes->datalen == 0 )
  return 0;

 if ( mes->datalen > MUROARD_MAX_MSGSIZE )
  return -1;

#ifndef __WIN32
 ret = muroard_network_read(sock, mes->data, mes->datalen);
 if ( ret == -1 && errno == EAGAIN ) {
  sched_yield();
  ret = muroard_network_read(sock, mes->data, mes->datalen);
  if ( ret == -1 && errno == EAGAIN ) {
   usleep(muroard_state_member(abuffer_size)*25000LLU/(long long unsigned)(muroard_state_member(sa_rate)*muroard_state_member(sa_channels)*2));
   sched_yield();
   ret = muroard_network_read(sock, mes->data, mes->datalen);
  }
 }
 if ( ret != (ssize_t)mes->datalen )
  return -1;
#else
 if ( muroard_network_read(sock, mes->data, mes->datalen) != (ssize_t)mes->datalen )
  return -1;
#endif

 return 0;
}

int muroard_proto_send(muroard_handle_t sock, struct muroard_message * mes) {
 unsigned char header[10];

// fprintf(stderr, "MSG OUT: ver=0, cmd=%i, stream=%i, pos=%u, datalen=%u\n", mes->cmd, mes->stream, mes->pos, mes->datalen);

 header[0] =  0; // version;
 header[1] =  mes->cmd;
 header[2] = (mes->stream  &     0xFF00) >>  8;
 header[3] = (mes->stream  &     0x00FF);
 header[4] = (mes->pos     & 0xFF000000) >> 24;
 header[5] = (mes->pos     & 0x00FF0000) >> 16;
 header[6] = (mes->pos     & 0x0000FF00) >>  8;
 header[7] = (mes->pos     & 0x000000FF);
 header[8] = (mes->datalen &     0xFF00) >>  8;
 header[9] = (mes->datalen &     0x00FF);

 if ( muroard_network_write(sock, header, 10) != 10 )
  return -1;

 if ( mes->datalen == 0 )
  return 0;

 if ( muroard_network_write(sock, mes->data, mes->datalen) != (ssize_t)mes->datalen )
  return -1;

 return 0;
}

//ll
