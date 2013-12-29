//deps.h:

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

#ifndef _MUROARD_DEPS_H_
#define _MUROARD_DEPS_H_

#if defined(MUROARD_FEATURE_SOCKET_DECNET) && !defined(HAVE_LIB_DNET_DAEMON)
#undef MUROARD_FEATURE_SOCKET_DECNET
#endif

#if defined(MUROARD_FEATURE_CODECFILTER_OGG_VORBIS) && !defined(HAVE_LIB_VORBISFILE)
#undef MUROARD_FEATURE_CODECFILTER_OGG_VORBIS
#endif

/*** define things we disabple below if not supported ***/
#ifndef TARGET_MICROCONTROLLER
#define MUROARD_HAVE_FORK
#define MUROARD_HAVE_FCHMOD
#define MUROARD_HAVE_UMASK
#endif

/*** calc TARGET_MICROCONTROLLER stuff first ***/
#ifdef TARGET_MICROCONTROLLER
#ifdef MUROARD_FEATURE_HELP_TEXT
#undef MUROARD_FEATURE_HELP_TEXT
#endif
#ifdef MUROARD_FEATURE_VERSION_TEXT
#undef MUROARD_FEATURE_VERSION_TEXT
#endif
#ifdef MUROARD_FEATURE_SOCKET_UNIX
#undef MUROARD_FEATURE_SOCKET_UNIX
#endif
#ifdef MUROARD_FEATURE_SIGNALS
#undef MUROARD_FEATURE_SIGNALS
#endif
#endif

/*** calc general things ***/
#ifndef MUROARD_FEATURE_ARGV
// disable help:
#ifdef MUROARD_FEATURE_HELP_TEXT
#undef MUROARD_FEATURE_HELP_TEXT
#endif
#ifdef MUROARD_FEATURE_VERSION_TEXT
#undef MUROARD_FEATURE_VERSION_TEXT
#endif
#endif


/*** calc network things ***/
#ifndef MUROARD_FEATURE_SOCKET_LISTEN
// disable all kinds of listen sockets:

// IP sockets:
#ifdef MUROARD_FEATURE_SOCKET_INET
#undef MUROARD_FEATURE_SOCKET_INET
#endif

// UNIX sockets:
#ifdef MUROARD_FEATURE_SOCKET_UNIX
#undef MUROARD_FEATURE_SOCKET_UNIX
#endif

// DECnet sockets:
#ifdef MUROARD_FEATURE_SOCKET_DECNET
#undef MUROARD_FEATURE_SOCKET_DECNET
#endif
#endif

// disable things Win32 does not support:
#if defined(__WIN32)

// No UNIX sockets support on win32
#ifdef MUROARD_FEATURE_SOCKET_UNIX
#undef MUROARD_FEATURE_SOCKET_UNIX
#endif

// No Signals on win32
#ifdef MUROARD_FEATURE_SIGNALS
#undef MUROARD_FEATURE_SIGNALS
#endif

// No fork() on win32:
#ifdef MUROARD_HAVE_FORK
#undef MUROARD_HAVE_FORK
#endif

// No fchmod() on win32:
#ifdef MUROARD_HAVE_FCHMOD
#undef MUROARD_HAVE_FCHMOD
#endif

// No umask() on win32:
#ifdef MUROARD_HAVE_UMASK
#undef MUROARD_HAVE_UMASK
#endif
#endif


#ifndef MUROARD_FEATURE_SOCKET_UNIX
#ifdef MUROARD_FEATURE_CMD_PASSFH
#undef MUROARD_FEATURE_CMD_PASSFH
#endif
#endif

/*** calc support of functions ***/
#if !defined(MUROARD_HAVE_FORK) && !defined(__WIN32)
#ifdef MUROARD_FEATURE_DAEMONIZE
#undef MUROARD_FEATURE_DAEMONIZE
#endif
#endif

#ifndef MUROARD_HAVE_FCHMOD
#ifdef MUROARD_DEFAULT_LISTEN_CHMOD
#undef MUROARD_DEFAULT_LISTEN_CHMOD
#endif
#endif

/*** calc list of needed dsp functions ***/
#ifdef MUROARD_FEATURE_UPMIX_INPUT
#define MUROARD_FEATURE_UPMIX
#endif
#ifdef MUROARD_FEATURE_UPSCALE_INPUT
#define MUROARD_FEATURE_UPSCALE
#endif
#ifdef MUROARD_FEATURE_DOWNMIX_OUTPUT
#define MUROARD_FEATURE_DOWNMIX
#endif
#ifdef MUROARD_FEATURE_DOWNSCALE_OUTPUT
#define MUROARD_FEATURE_DOWNSCALE
#endif

/*** calc minimum requirements, throw error if we hit a error ***/
#if !defined(MUROARD_FEATURE_ARGV) && !defined(MUROARD_FEATURE_SOCKET_LISTEN)
#if !(defined(MUROARD_FEATURE_SOCKET_INET) || defined(MUROARD_FEATURE_SOCKET_UNIX))
#error You did not configure any way to comunicate with the clients
#endif
#endif

#if defined(MUROARD_MEMMGR_NOUSE_MALLOC) && !defined(MUROARD_FIXED_ABUFFER_SIZE)
#error You must define MUROARD_FIXED_ABUFFER_SIZE in feature.h if going to use MUROARD_MEMMGR_NOUSE_MALLOC
#endif

/*** calc defaults based on what is left ***/
#ifdef MUROARD_FEATURE_SOCKET_LISTEN
#ifndef MUROARD_DEFAULT_LISTEN_TYPE
#ifdef MUROARD_FEATURE_SOCKET_UNIX
#define MUROARD_DEFAULT_LISTEN_TYPE        MUROARD_NETWORK_TYPE_UNIX
#elif defined(MUROARD_FEATURE_SOCKET_INET)
#define MUROARD_DEFAULT_LISTEN_TYPE        MUROARD_NETWORK_TYPE_INET
#elif defined(MUROARD_FEATURE_SOCKET_DECNET)
#define MUROARD_DEFAULT_LISTEN_TYPE        MUROARD_NETWORK_TYPE_DECNET
#else
#undef MUROARD_FEATURE_SOCKET_LISTEN
#endif
#endif
#endif

#ifndef MUROARD_FEATURE_DRIVER
#ifdef __WIN32
#define MUROARD_FEATURE_DRIVER             MUROARD_DRIVER_WMM
#else
#define MUROARD_FEATURE_DRIVER             MUROARD_DRIVER_OSS
#endif
#endif

#if defined(MUROARD_FEATURE_SOCKET_DECNET) && !defined(MUROARD_FEATURE_SOCKET_LISTEN_SOCKTYPE)
#define MUROARD_FEATURE_SOCKET_LISTEN_SOCKTYPE
#endif

#if defined(MUROARD_FEATURE_REREADWRITE) && defined(__WIN32)
#undef MUROARD_FEATURE_REREADWRITE
#endif

#endif

//ll
