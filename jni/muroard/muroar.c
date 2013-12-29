//muroar.c:

/*
 *      Copyright (C) Philipp 'ph3-der-loewe' Schafft - 2009-2013
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


/* ckport options:
 * ckport: ignore-symbol: strcpy of target security   -- length is check before the function is called.
 * ckport: ignore-symbol: atoi   of target security   -- returned by functions ensuring tailing \0.
 * ckport: ignore-symbol: strdup of target security   -- Caller is enforced to pass valid buffer.
 */

#include "muroar.h"
#include "private.h"

#include <string.h>
#ifndef NO_STDINT_H
#include <stdint.h>
#endif
#include <stdlib.h>
#include <stdio.h> /* for snprintf() */

#define MUROAR_IOBUF         128

#define _MU_STR              "\265"
#define _DEFAULT_CLIENT_NAME _MU_STR "Roar Client";

/* Format of RoarAudio Message format 0:
 *
 * | 0      | 1      | 2      | 3      | Byte
 * +--------+--------+--------+--------+------
 * |Version |Command | Stream ID       | 0-3
 * +--------+--------+--------+--------+------
 * | Stream Position                   | 4-7
 * +--------+--------+--------+--------+------
 * | Data Length     | Data ....       | 7-11
 * +--------+--------+--------+--------+------
 *
 * All data is in network byte order.
 * Version         : Version number of message, always 0.
 * Command         : Command, one of MUROAR_CMD_*.
 * Stream ID       : ID of Stream we use this command on,
 *                   or -1 (0xFFFF) on non-stream commands.
 * Stream Position : Position of Stream in units depending on stream type.
                     For waveform streams samples (not frames) played.
 * Data Length     : Length of data in the data part of this message
                     in bytes.
 */

// Need to init the network layer on win32 because of it's
// broken design.
#ifdef __WIN32
static void muroar_init_win32 (void) {
 static int inited = 0;
 WSADATA wsadata;

 if ( !inited ) {
  WSAStartup(MAKEWORD(1,1) , &wsadata);
  inited++;
 }
}
#endif

// Open Socket to server.
static int muroar_open_socket_bsd (const char * server) {
 struct hostent     * he;
 struct sockaddr_in   in;
#ifdef HAVE_AF_UNIX
 struct sockaddr_un   un;
#endif
 int fh = -1;
 char * buf = NULL;
 char * object;
#ifdef HAVE_LIB_DNET
 char * node;
 static char localnode[16] = {0};
 struct dn_naddr      *binaddr;
 struct nodeent       *dp;
#endif

#ifdef __WIN32
 muroar_init_win32();
#endif

 if ( !strcmp(server, MUROAR_ABSTRACT) || (strstr(server, "/") != NULL && strstr(server, "::") == NULL) ) {
// Handle AF_UNIX Sockets,
// do not build on broken systems like win32 not
// supporting the AF_UNIX sockets.
#ifdef HAVE_AF_UNIX
  if ( (fh = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 )
   return -1;

  un.sun_family = AF_UNIX;

  if ( !strcmp(server, MUROAR_ABSTRACT) ) {
   memset(un.sun_path, 0, sizeof(un.sun_path));
   strncpy(un.sun_path + 1, "RoarAudio/UNIX/Abstract/1", sizeof(un.sun_path) - 2);
  } else {
   strncpy(un.sun_path, server, sizeof(un.sun_path) - 1);
  }

  if ( connect(fh, (struct sockaddr *)&un, sizeof(struct sockaddr_un)) == -1 ) {
   __CLOSE(fh);
   return -1;
  }

  return fh;
#else
  return -1;
#endif
 } else if ( strstr(server, "::") != NULL ) {
#ifdef HAVE_LIB_DNET
  // alloc a temp buffer so we can change the string at will:
  buf = strdup(server);

  // cut node::object into buf and object
  object  = strstr(buf, "::");
  *object = 0;
   object += 2;

  // use default if we have a zero-size node name:
  if ( *buf == 0 ) {
   if ( !localnode[0] ) {
    if ( (binaddr=getnodeadd()) == NULL) {
     free(buf);
     return -1;
    }

    if ( (dp = getnodebyaddr((char*)binaddr->a_addr, binaddr->a_len, AF_DECnet)) == NULL ) {
     free(buf);
     return -1;
    }

    strncpy(localnode, dp->n_name, sizeof(localnode)-1);
    localnode[sizeof(localnode)-1] = 0;
   }

   node = localnode;
  } else {
   node = buf;
  }

  // use default if we have a zero size object name:
  if ( *object == 0 ) {
   object = MUROAR_OBJECT;
  }

  fh = dnet_conn(node, object, SOCK_STREAM, NULL, 0, NULL, 0);

  // free buffer when we are done.
  free(buf);

  return fh;
#else
  return -1;
#endif
 }

 if ( strstr(server, ":") != NULL ) {
  buf = strdup(server);
  server = buf;
  object = strstr(buf, ":");
  *object = 0;
  object++;
  if ( !*object ) /* finnaly check if this is just a tailing :, in that case we assume the default port */
   object = NULL;
 } else {
  object = NULL;
 }

 if ( (he = gethostbyname(server)) == NULL ) {
  if ( buf != NULL )
   free(buf);
  return -1;
 }

 memcpy((struct in_addr *)&(in.sin_addr), he->h_addr, sizeof(struct in_addr));

 in.sin_family = AF_INET;
 if ( object != NULL ) {
  in.sin_port   = htons(atoi(object));
 } else {
  in.sin_port   = htons(MUROAR_PORT);
 }

 if ( buf != NULL )
  free(buf);

 if ( (fh = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
  return -1;

 if ( connect(fh, (const struct sockaddr *)&in, sizeof(in)) == -1 ) {
  __CLOSE(fh);
  return -1;
 }

 return fh;
}

static muroar_t muroar_open_socket (const char * server) {
#if MUROAR_HDLTYPE == MUROAR_HDLTYPE_YIFF_VIO
 yiff_identifier_t fh = -1;
#else
 int fh = muroar_open_socket_bsd(server);
#endif
 muroar_t ret;

 if ( fh == -1 )
  return MUROAR_HANDLE_INVALID;

#if MUROAR_HDLTYPE == MUROAR_HDLTYPE_POSIX_SYSIO || MUROAR_HDLTYPE == MUROAR_HDLTYPE_WIN32_SOCKET
 ret = fh;
#elif MUROAR_HDLTYPE == MUROAR_HDLTYPE_POSIX_STDIO
 ret = fdopen(fh, "a+");
#else
#error Unsupported value for MUROAR_HDLTYPE
#endif

 if ( ret == MUROAR_HANDLE_INVALID ) {
  __CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 return ret;
}

// Open Control connection
muroar_t muroar_connect(const char * server, const char * name) {
 char   useraddr[80] = MUROAR_INVALID;
 char * addr[] = {useraddr, MUROAR_GSOCK, MUROAR_HOST, "::" MUROAR_OBJECT, MUROAR_ABSTRACT, NULL};
 const char * home;
 unsigned char buf[MUROAR_IOBUF];
 uint16_t tmpu16;
 uint16_t pid;
 muroar_t fh = MUROAR_HANDLE_INVALID;
 int i;
#if !defined(__WIN32)
 ssize_t len;
#endif

 // Prepare server address:
 if ( server != NULL && *server == 0 )
  server = NULL;

 if ( server == NULL )
  server = getenv("ROAR_SERVER");

#if !defined(__WIN32)
 if ( server == NULL ) {
  if ( (len = readlink("/etc/roarserver", useraddr, sizeof(useraddr))) != -1 ) {
   useraddr[len < (ssize_t)sizeof(useraddr) ? (size_t)len : sizeof(useraddr)-1] = 0;
   server = useraddr;
  }
 }
#endif

 if ( server != NULL ) {
  // "+default" is an alias to NULL.
  if ( !strcmp(server, MUROAR_DEFAULT) ) {
   server = NULL;

  // "+invalid" always return an invalid handle.
  } else if ( !strcmp(server, MUROAR_INVALID) ) {
#ifdef ECANCELED
   _SET_ERRNO(ECANCELED);
#else
   _SET_ERRNO(EINVAL);
#endif
   return MUROAR_HANDLE_INVALID;
  }
 }

 // Connect to server:
 if ( server != NULL ) {
  if ( (fh = muroar_open_socket(server)) == MUROAR_HANDLE_INVALID )
   return MUROAR_HANDLE_INVALID;
 } else {
  // build string for ~/.roar
  home = getenv("HOME");
  if ( home != NULL && home[0] == '/' && strlen(home) < (sizeof(useraddr) - 7) ) {
   snprintf(useraddr, sizeof(useraddr), "%s/.roar", home);
   useraddr[sizeof(useraddr)-1] = 0;
  }

  // go thru list of possible defaults:
  for (i = 0; fh == MUROAR_HANDLE_INVALID && addr[i] != NULL; i++) {
   if ( !strcmp(addr[i], MUROAR_INVALID) )
    continue;

   fh = muroar_open_socket(addr[i]);
  }

  if ( fh == MUROAR_HANDLE_INVALID ) {
   _SET_ERRNO(ENOENT);
   return MUROAR_HANDLE_INVALID;
  }
 }

 // Prepare client name:
 if ( name == NULL || *name == 0 )
  name = _DEFAULT_CLIENT_NAME;

 // Send IDENTIFY command to server:
 memset(buf, 0, sizeof(buf));
 buf[1] = MUROAR_CMD_IDENTIFY;

 // Calculate the length for the data part of the package.
 // Its 5 bytes plus the length of the name string.
 tmpu16 = strlen(name) + 5;

 // check if we have space for 5 bytes + length of name + tailing \0
 // in the buffer.
 if ( tmpu16 >= MUROAR_IOBUF ) {
  _CLOSE(fh);
  _SET_ERRNO(EINVAL);
  return MUROAR_HANDLE_INVALID;
 }

 buf[8] = (tmpu16 & 0xFF00) >> 8;
 buf[9] = (tmpu16 & 0x00FF);

 if ( muroar_write(fh, buf, 10) != 10 ) {
  _CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 buf[0] = 1;
 pid = getpid();
 buf[1] = (pid & 0xFF000000UL) >> 24;
 buf[2] = (pid & 0x00FF0000UL) >> 16;
 buf[3] = (pid & 0x0000FF00UL) >>  8;
 buf[4] = (pid & 0x000000FFUL) >>  0;

 // sizes are already checked.
 strcpy((char*)&(buf[5]), name);

 if ( muroar_write(fh, buf, tmpu16) != tmpu16 ) {
  _CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 if ( muroar_read(fh, buf, 10) != 10 ) {
  _CLOSE(fh);
  return MUROAR_HANDLE_INVALID;
 }

 if ( buf[1] != MUROAR_CMD_OK ) {
  _CLOSE(fh);
  _SET_ERRNO(EACCES);
  return MUROAR_HANDLE_INVALID;
 }

 // Send AUTH command to server:
 // We send zero-byte AUTH command
 // (type=NONE).
 memset(buf, 0, 10);
 buf[1] = MUROAR_CMD_AUTH;

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
  _SET_ERRNO(EACCES);
  return MUROAR_HANDLE_INVALID;
 }

 // We now have a working control connection, return it.
 return fh;
}

//ll
