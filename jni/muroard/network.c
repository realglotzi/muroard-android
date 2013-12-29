//network.c:

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
#ifndef __WIN32
#include <sched.h>
#endif

int muroard_network_init(void) {
#ifdef __WIN32
 WSADATA wsadata;

 WSAStartup(MAKEWORD(1,1) , &wsadata);
#endif
 return 0;
}

int muroard_network_free(void) {
 return muroard_network_prefree();
}

int muroard_network_prefree(void) {
#ifdef MUROARD_FEATURE_SOCKET_LISTEN
 if ( muroard_state_member(listen_socket) != MUROARD_HANDLE_INVALID )
  muroard_network_close(muroard_state_member(listen_socket));

  muroard_state_member(listen_socket) = MUROARD_HANDLE_INVALID;
#ifdef MUROARD_FEATURE_SOCKET_LISTEN_SOCKTYPE
  muroard_state_member(listen_type) = -1;
#endif
#endif

 return 0;
}

#ifdef MUROARD_FEATURE_SOCKET_UNIX
static inline int muroard_network_test_unix (const char * addr) {
 struct sockaddr_un   un;
 muroard_handle_t fh;
 int ret;

 if ( (fh = socket(AF_UNIX, SOCK_STREAM, 0)) == MUROARD_HANDLE_INVALID )
  return -1;

 un.sun_family = AF_UNIX;
 strncpy(un.sun_path, addr, sizeof(un.sun_path) - 1);

 if ( connect(fh, (struct sockaddr *)&un, sizeof(struct sockaddr_un)) == -1 ) {
  if ( errno == ECONNREFUSED ) {
   ret =  0;
  } else {
   ret = -1;
  }
 } else {
  ret = 1;
 }

 muroard_network_close(fh);

 return ret;
}
#endif

#ifdef MUROARD_FEATURE_SOCKET_LISTEN
int muroard_network_listen(int type, const char * addr, int port) {
#ifdef MUROARD_FEATURE_SOCKET_UNIX
 struct sockaddr_un   un;
#endif
#ifdef MUROARD_FEATURE_SOCKET_INET
 struct hostent     * he;
 struct sockaddr_in   in;
#endif
 muroard_handle_t fh;
#ifdef MUROARD_DEFAULT_LISTEN_CHMOD
 mode_t defmask;
#endif
#ifdef MUROARD_FEATURE_SOCKET_DECNET
 struct sockaddr_dn dn;
 size_t objlen;
#ifdef DSO_ACCEPTMODE
 int acceptmode;
#endif
#endif

 switch (type) {
#ifdef MUROARD_FEATURE_SOCKET_UNIX
  case MUROARD_NETWORK_TYPE_UNIX:
    if ( addr == NULL )
     addr = MUROARD_DEFAULT_LISTEN_ADDR_UNIX;

    if ( (fh = socket(AF_UNIX, SOCK_STREAM, 0)) == MUROARD_HANDLE_INVALID )
     return -1;

    un.sun_family = PF_UNIX;
    strncpy(un.sun_path, addr, sizeof(un.sun_path) - 1);

#ifdef MUROARD_DEFAULT_LISTEN_CHMOD
    defmask = umask(0000);

    if ( fchmod(fh, MUROARD_DEFAULT_LISTEN_CHMOD) == -1 ) {
     muroard_network_close(fh);
     return -1;
    }
#endif

    if ( bind(fh, (struct sockaddr *)&un, sizeof(struct sockaddr_un)) == -1 ) {
     if ( muroard_network_test_unix(addr) != 0 ) {
      muroard_network_close(fh);
      return -1;
     }

     if ( unlink(addr) == -1 ) {
      muroard_network_close(fh);
      return -1;
     }

     if ( bind(fh, (struct sockaddr *)&un, sizeof(struct sockaddr_un)) == -1 ) {
      muroard_network_close(fh);
      return -1;
     }
    }

#ifdef MUROARD_DEFAULT_LISTEN_CHMOD
    umask(defmask);
#endif

   break;
#endif
#ifdef MUROARD_FEATURE_SOCKET_INET
  case MUROARD_NETWORK_TYPE_INET:
    if ( addr == NULL )
     addr = MUROARD_DEFAULT_LISTEN_ADDR_INET;

    if ( (he = gethostbyname(addr)) == NULL ) {
     return -1;
    }

    if ( (fh = socket(AF_INET, SOCK_STREAM, 0)) == MUROARD_HANDLE_INVALID )
     return -1;

    memcpy((struct in_addr *)&in.sin_addr, he->h_addr, sizeof(struct in_addr));

    in.sin_family = AF_INET;
    in.sin_port   = htons(port);

    if ( bind(fh, (const struct sockaddr *)&in, sizeof(in)) == -1 ) {
     muroard_network_close(fh);
     return -1;
    }
   break;
#endif
#ifdef MUROARD_FEATURE_SOCKET_DECNET
  case MUROARD_NETWORK_TYPE_DECNET:
    if ( addr == NULL )
     addr = MUROARD_DEFAULT_LISTEN_ADDR_DECNET;

    // we only support empty node addresses here (bind globaly).
    if ( addr[0] != ':' || addr[1] != ':' )
     return -1;

    addr += 2;

    if ( (fh = socket(AF_DECnet, SOCK_SEQPACKET, DNPROTO_NSP)) == MUROARD_HANDLE_INVALID )
     return -1;

#ifdef DSO_ACCEPTMODE
    acceptmode = ACC_DEFER;
    setsockopt(fh, DNPROTO_NSP, DSO_ACCEPTMODE, &acceptmode, sizeof(acceptmode));
#endif

    muroard_memzero(&dn, sizeof(dn));
    dn.sdn_family    = AF_DECnet;
    dn.sdn_flags     = 0;
    dn.sdn_objnum    = 0;
    objlen = strlen(addr);
#if MUROAR_CODEC_PCM_S_LE == MUROAR_CODEC_PCM_S
    dn.sdn_objnamel  = objlen;
#elif MUROAR_CODEC_PCM_S_BE == MUROAR_CODEC_PCM_S
    if ( sizeof(unsigned short) == 2 ) {
     dn.sdn_objnamel = ((objlen & 0xFF00) >> 8) | ((objlen & 0x00FF) << 8);
    } else {
     return -1;
    }
#else
#error Unsupported Endianess.
#endif
    strcpy((char *)dn.sdn_objname, addr);

    bind(fh, (struct sockaddr *)&dn, sizeof(dn));
   break;
#endif
  default:
    return -1;
   break;
 }

 if ( listen(fh, 8) == -1 ) {
  muroard_network_close(fh);
 }

 muroard_state_member(listen_socket) = fh;
#ifdef MUROARD_FEATURE_SOCKET_LISTEN_SOCKTYPE
 muroard_state_member(listen_type) = type;
#endif

 return 0;
}
#endif

#ifndef __WIN32
int muroard_network_nonblock(muroard_handle_t fh, int reset) {
 int flags;

 if ( (flags = fcntl(fh, F_GETFL, 0)) == -1 )
  return -1;

 flags |= O_NONBLOCK;
 if (reset)
  flags -= O_NONBLOCK;

 return fcntl(fh, F_SETFL, flags);
}
#endif

int muroard_network_check(enum muroard_blocking blocking) {
 struct timeval tv;
 fd_set rfds;
 void * data;
 int ret;
 muroard_handle_t fh;
 int i;
 int max_fh = -1;

 switch (blocking) {
  case MUROARD_BLOCKING_NONE:
    tv.tv_sec  = 0;
    tv.tv_usec = 1;
   break;
  case MUROARD_BLOCKING_LONG:
    tv.tv_sec  = 1024;
    tv.tv_usec = 0;
   break;
 }

 FD_ZERO(&rfds);

#ifdef MUROARD_FEATURE_SOCKET_LISTEN
 if ( muroard_state_member(listen_socket) != MUROARD_HANDLE_INVALID ) {
  FD_SET(muroard_state_member(listen_socket), &rfds);
  max_fh = muroard_state_member(listen_socket);
 }
#endif

 for (i = 0; i < MUROARD_MAX_STREAMS; i++) {
  if ( muroard_stream_get_datadir(i) == STREAM_DATADIR_IN ) {
   if ( (fh = muroard_stream_get_sock(i)) != MUROARD_HANDLE_INVALID ) {
    FD_SET(fh, &rfds);
    if ( fh > max_fh )
     max_fh = fh;
   }
  }
 }

 for (i = 0; i < MUROARD_MAX_CLIENTS; i++) {
  if ( (fh = client_get_sock(i)) != MUROARD_HANDLE_INVALID ) {
   FD_SET(fh, &rfds);
   if ( fh > max_fh )
    max_fh = fh;
  }
 }

 if ( max_fh == -1 ) { // all IOs are gone, terminate the daemon
  muroard_state_member(alive) = 0;
  return 0;
 }

 ret = select(max_fh + 1, &rfds, NULL, NULL, &tv);

 if ( ret == -1 )
  return -1;

 if ( ret == 0 ) {
  for (i = 0; i < MUROARD_MAX_STREAMS; i++) {
   if ( muroard_stream_get_datadir(i) == STREAM_DATADIR_IN ) {
    if ( (data = muroard_stream_get_iobuf(i)) != NULL ) {
     muroard_memzero(data, muroard_state_member(abuffer_size));
    }
   }
  }
  return 0;
 }

#ifdef MUROARD_FEATURE_SOCKET_LISTEN
 if ( muroard_state_member(listen_socket) != MUROARD_HANDLE_INVALID ) {
  if ( FD_ISSET(muroard_state_member(listen_socket), &rfds) ) {
   fh = accept(muroard_state_member(listen_socket), NULL, NULL);
#ifdef MUROARD_FEATURE_SOCKET_DECNET
   if ( muroard_state_member(listen_type) == MUROARD_NETWORK_TYPE_DECNET )
    dnet_accept(fh, 0, NULL, 0);
#endif
   muroard_client_new(fh);
  }
 }
#endif

 for (i = 0; i < MUROARD_MAX_STREAMS; i++) {
  if ( muroard_stream_get_datadir(i) == STREAM_DATADIR_IN ) {
   if ( (fh = muroard_stream_get_sock(i)) != MUROARD_HANDLE_INVALID ) {
    if ( FD_ISSET(fh, &rfds) ) {
     muroard_stream_read(i);
    } else {
     if ( (data = muroard_stream_get_iobuf(i)) != NULL )
      muroard_memzero(data, muroard_state_member(abuffer_size));
    }
   }
  }
 }

 for (i = 0; i < MUROARD_MAX_CLIENTS; i++) {
  if ( (fh = client_get_sock(i)) != MUROARD_HANDLE_INVALID ) {
   if ( FD_ISSET(fh, &rfds) ) {
    muroard_client_handle(i);
   }
  }
 }

 return 0;
}

#ifdef MUROARD_FEATURE_CMD_PASSFH
#define _SCMR_CONTROLLEN (sizeof(struct cmsghdr) + sizeof(int))
muroard_handle_t muroard_network_recvfh(muroard_handle_t fh) {
 struct iovec     iov[1];
 struct msghdr    msg;
 char             cmptr_buf[_SCMR_CONTROLLEN];
 struct cmsghdr * cmptr = (struct cmsghdr *) cmptr_buf;
 char             localmes[1];

 iov[0].iov_base = localmes;
 iov[0].iov_len  = 1;
 msg.msg_iov     = iov;
 msg.msg_iovlen  = 1;
 msg.msg_name    = NULL;
 msg.msg_namelen = 0;

 msg.msg_control    = (caddr_t) cmptr;
 msg.msg_controllen = _SCMR_CONTROLLEN;

 if ( recvmsg(fh, &msg, 0) == -1 ) {
#ifndef __WIN32
  if ( errno == EAGAIN ) {
   sched_yield();
   if ( recvmsg(fh, &msg, 0) == -1 )
    return MUROARD_HANDLE_INVALID;
  } else {
   return MUROARD_HANDLE_INVALID;
  }
#else
  return MUROARD_HANDLE_INVALID;
#endif
 }

 if ( msg.msg_controllen != _SCMR_CONTROLLEN )
  return MUROARD_HANDLE_INVALID;

 return *(int *)CMSG_DATA(cmptr);
}
#endif

//ll
