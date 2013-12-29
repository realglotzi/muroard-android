//state.h:

/*
 *      Copyright (C) Philipp 'ph3-der-loewe' Schafft - 2012-2013
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

#ifndef _MUROARD_STATE_H_
#define _MUROARD_STATE_H_

struct muroard_state {
 int alive;
 int standby;

/*********************************************************/
/* audio config                                          */
/*********************************************************/

 int sa_rate;
 int sa_channels;

/*********************************************************/
/* mixer config                                          */
/*********************************************************/

 size_t abuffer_size;

/*********************************************************/
/* network config                                        */
/*********************************************************/

#ifdef MUROARD_FEATURE_SOCKET_LISTEN
 muroard_handle_t listen_socket;
#endif
#ifdef MUROARD_FEATURE_SOCKET_LISTEN_SOCKTYPE
 int listen_type;
#endif

// ...
 int16_t * waveform_mixercore;
 struct muroard_client client[MUROARD_MAX_CLIENTS];
 struct muroard_stream stream[MUROARD_MAX_STREAMS];

#ifdef MUROARD_FEATURE_OPTIMIZE
 ssize_t waveform_last_scount;
#endif
 size_t waveform_offset;

 enum muroard_blocking driver_old_blocking;
 int driver_fh;
 void * driver_vp;
};

extern struct muroard_state * muroard_g_state;

#define muroard_state_member(m) (muroard_g_state->m)

#endif

//ll
