//network.h:

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

#ifndef _MUROARD_NETWORK_H_
#define _MUROARD_NETWORK_H_

#define MUROARD_NETWORK_TYPE_UNIX      1
#define MUROARD_NETWORK_TYPE_INET      2
#define MUROARD_NETWORK_TYPE_DECNET    3

int muroard_network_init(void);
int muroard_network_free(void);
int muroard_network_prefree(void);

int muroard_network_listen(int type, const char * addr, int port);

int muroard_network_check(enum muroard_blocking blocking);

#ifdef __WIN32
#define muroard_network_read(fh,buf,len)   recv((fh), (char*)(buf), (len), 0)
#define muroard_network_write(fh,buf,len)  send((fh), (const char*)(buf), (len), 0)
#define muroard_network_close(fh)          closesocket((fh))
#define muroard_network_nonblock(fh,reset) (0)
#else
#define muroard_network_read(fh,buf,len)   read((fh), (buf), (len))
#define muroard_network_write(fh,buf,len)  write((fh), (buf), (len))
#define muroard_network_close(fh)          close((fh))
int muroard_network_nonblock(muroard_handle_t fh, int reset);
#endif

#ifdef MUROARD_FEATURE_CMD_PASSFH
muroard_handle_t muroard_network_recvfh(muroard_handle_t fh);
#endif

#endif

//ll
