//proto.h:

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

#ifndef _MUROARD_PROTO_H_
#define _MUROARD_PROTO_H_

struct muroard_message {
 int cmd;
 uint16_t stream;
 uint32_t pos;
 size_t datalen;
 char data[MUROARD_MAX_MSGSIZE];
};

int muroard_proto_recv(muroard_handle_t sock, struct muroard_message * mes);
int muroard_proto_send(muroard_handle_t sock, struct muroard_message * mes);


#endif

//ll
