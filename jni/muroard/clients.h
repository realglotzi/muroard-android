//clients.h:

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

#ifndef _MUROARD_CLIENTS_H_
#define _MUROARD_CLIENTS_H_

#define CLIENT_STATE_UNUSED        0
#define CLIENT_STATE_NEW           1
#define CLIENT_STATE_OLD           2
#define CLIENT_STATE_EXECED        3
#define CLIENT_STATE_CLOSING       4
#ifdef MUROARD_FEATURE_INTERNAL_CLIENT
#define CLIENT_STATE_INTERNAL      5
#endif


struct muroard_client {
 int state;
 int stream;
 muroard_handle_t sock;
};

int muroard_client_init(void);
int muroard_client_free(void);
int muroard_client_new(muroard_handle_t sock);
int muroard_client_delete(int id);
int muroard_client_handle(int id);
muroard_handle_t muroard_client_exec(int id);

int muroard_client_handle_new_stream(int id, struct muroard_message * mes);

#ifdef MUROARD_FEATURE_CMD_ATTACH
int muroard_client_handle_attach(int id, struct muroard_message * mes);
#endif
#ifdef MUROARD_FEATURE_CMD_SERVER_OINFO
int muroard_client_handle_server_oinfo(int id, struct muroard_message * mes);
#endif
#ifdef MUROARD_FEATURE_CMD_PASSFH
int muroard_client_handle_passfh(int id, struct muroard_message * mes);
#endif
#ifdef MUROARD_FEATURE_CMD_LIST_CLIENTS
int muroard_client_handle_list_clients(int id, struct muroard_message * mes);
#endif
#ifdef MUROARD_FEATURE_CMD_LIST_STREAMS
int muroard_client_handle_list_streams(int id, struct muroard_message * mes);
#endif
#ifdef MUROARD_FEATURE_CMD_GET_CLIENT
int muroard_client_handle_get_client(int id, struct muroard_message * mes);
#endif
#ifdef MUROARD_FEATURE_CMD_GET_STREAM
int muroard_client_handle_get_stream(int id, struct muroard_message * mes);
#endif
#ifdef MUROARD_FEATURE_CMD_CAPS
int muroard_client_handle_caps(int id, struct muroard_message * mes);
#endif
#ifdef MUROARD_FEATURE_CMD_SERVER_INFO
int muroard_client_handle_server_info(int id, struct muroard_message * mes);
#endif
#ifdef MUROARD_FEATURE_CMD_GETTIMEOFDAY
int muroard_client_handle_gettimeofday(int id, struct muroard_message * mes);
#endif

#define client_get_sock(id)      (muroard_state_member(client)[(id)].state == CLIENT_STATE_UNUSED ? -1 : muroard_state_member(client)[(id)].sock)
#define client_get_stream(id)    (muroard_state_member(client)[(id)].state == CLIENT_STATE_UNUSED ? -1 : muroard_state_member(client)[(id)].stream)
#define client_set_stream(id, s) (muroard_state_member(client)[(id)].state == CLIENT_STATE_UNUSED || muroard_state_member(client)[(id)].stream != -1 ? \
                                  -1 : (muroard_state_member(client)[(id)].stream = (s)) && 0)

#endif

//ll
