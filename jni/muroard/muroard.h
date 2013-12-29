//muroard.h:

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

#ifndef _MUROARD_H_
#define _MUROARD_H_

#include "features.h" /* feature config */
#include "deps.h"     /* calc deps of config options */

// MUROARD_VERSION can not be used by externel applications as it depends
// on parameters passed by the Makefile.
#define MUROARD_VERSION "muroard/" PACKAGE_VERSION " <" DEVICE_VENDOR_STRING ">" DISTRIBUTION_VERSION_STRING

#define muroard_handle_t int
#define MUROARD_HANDLE_INVALID -1

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Socket includes */
#ifdef __WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#if defined(PF_UNIX)
#include <sys/un.h>
#endif
#include <netdb.h>
#ifdef MUROARD_FEATURE_SOCKET_DECNET
#include <netdnet/dn.h>
#include <netdnet/dnetdb.h>
#endif
#endif

#if !defined(TARGET_MICROCONTROLLER)
#include <fcntl.h>
#endif

#ifndef __WIN32
#include <errno.h>
#endif

#ifdef MUROARD_FEATURE_SIGNALS
#include <signal.h>
#endif

#ifdef MUROARD_HAVE_FCHMOD
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef __OpenBSD__
#include <sys/uio.h>
#endif

#if defined(MUROARD_FEATURE_CODECFILTER) && defined(MUROARD_FEATURE_CODECFILTER_OGG_VORBIS)
#define OV_EXCLUDE_STATIC_CALLBACKS
#include <vorbis/vorbisfile.h>
#endif

struct muroard_state;

#include <muroar.h>   /* µRoar header file */

#include "para.h"     /* server parameter and config */

#include "memmgr.h"   /* macros for memory functions */

#include "callbacks.h"

enum muroard_blocking {
 MUROARD_BLOCKING_NONE =  0,
 MUROARD_BLOCKING_LONG = -1,
};

#include "network.h"
#include "waveform.h"
#include "proto.h"
#include "driver.h"
#include "dsp.h"
#include "clients.h"
#include "streams.h"
#ifdef MUROARD_FEATURE_CODECFILTER
#include "cf.h"
#endif
#include "state.h"

void muroard_usage (void);
void muroard_version (void);

void muroard_init_signals(void);
void muroard_daemonize(void);
int  muroard_init(void);
int muroard_mainloop_iteration(enum muroard_blocking network_blocking, enum muroard_blocking audio_blocking);
void muroard_mainloop(void);
void muroard_shutdown(void);
int muroard_main_initonly (int argc, char * argv[]);
int muroard_main (int argc, char * argv[]);

#endif

//ll
