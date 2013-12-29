//para.c:

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

/*
 * This C code file contains static initialisation for all global options.
 * NOTE: it must be ensured this is allways sync with para.h!
 */

#include "muroard.h"

void muroard_para_init(struct muroard_state * state) {
 memset(state, 0, sizeof(*state));

/*********************************************************/
/* audio config                                          */
/*********************************************************/
 state->sa_rate         = MUROARD_DEFAULT_RATE;
 state->sa_channels     = MUROARD_DEFAULT_CHANNELS;

/*********************************************************/
/* mixer config                                          */
/*********************************************************/
 state->abuffer_size = 0; /* we will calc this at runtime */

/*********************************************************/
/* network config                                        */
/*********************************************************/
#ifdef MUROARD_FEATURE_SOCKET_LISTEN
 state->listen_socket        = MUROARD_HANDLE_INVALID; /* listen socket to use */
#endif
#ifdef MUROARD_FEATURE_SOCKET_LISTEN_SOCKTYPE
 state->listen_type          = -1;
#endif

#ifdef MUROARD_FEATURE_OPTIMIZE
 state->waveform_last_scount = -1;
#endif

 state->driver_old_blocking  = MUROARD_BLOCKING_LONG;
 state->driver_fh            = -1;
}

//ll
