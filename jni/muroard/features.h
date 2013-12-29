//features.h:

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
 * This header contains the user configuration for µRoarD.
 */

#ifndef _MUROARD_FEATURES_H_
#define _MUROARD_FEATURES_H_

/*********************************************************/
/* below you find general/misc features                  */
/*********************************************************/

// Support help text? (--help/-h):
//#define MUROARD_FEATURE_HELP_TEXT

// Support version text? (--version):
//#define MUROARD_FEATURE_VERSION_TEXT

// Support progress arguments (argv[]):
#define MUROARD_FEATURE_ARGV

// Add unneeded code that optimzire CPU load?:
// This may make the binary a bit bigger but faster
#define MUROARD_FEATURE_OPTIMIZE

// Include signal handling?:
#define MUROARD_FEATURE_SIGNALS

// Support to daemonize?:
//#define MUROARD_FEATURE_DAEMONIZE

// Support standby mode?:
//#define MUROARD_FEATURE_STANDBY

// Activate auto standby by default?:
//#define MUROARD_FEATURE_STANDBY_AUTOMODE

// Start in standby mode by default?:
//#define MUROARD_FEATURE_STANDBY_ACTIVE

/*********************************************************/
/* below you find the default audio settings             */
/*********************************************************/

// Sample rate for the server:
// The server will reject all streams with a diffrent rate.
#define MUROARD_DEFAULT_RATE               44100

// nummber of channels:
#define MUROARD_DEFAULT_CHANNELS           2

// size of the internal cycle:
#define MUROARD_DEFAULT_FRAMES_PER_CYCLE   (muroard_state_member(sa_rate)/25) /* 40ms */

// Fixed audio buffer size (in Bytes):
// This is used to use a fixed audio buffer size.
// It is only useful in case when there is no malloc() for your system or
// you want to enforce muroard is locked in memory.
#define MUROARD_FIXED_ABUFFER_SIZE        1024

/*********************************************************/
/* below you find the settings for DSP features          */
/*********************************************************/

// Enable support for upmix signals from mono?:
// This may be buggy.
#define MUROARD_FEATURE_UPMIX_INPUT

// Enable support for downmix signals to mono?:
// This may be buggy.
#define MUROARD_FEATURE_DOWNMIX_OUTPUT

// Enable support fo upscale signals from 8 to 16 bit?:
#define MUROARD_FEATURE_UPSCALE_INPUT

// Enable support fo downscale signals from 16 to 8 bit?:
#define MUROARD_FEATURE_DOWNSCALE_OUTPUT

// Enable support to convert between diffrent byte orders?:
#define MUROARD_FEATURE_BYTE_SWAP

// Enable software volume control?:
#define MUROARD_FEATURE_VOLUME_CONTROL

/*********************************************************/
/* below you find the codecfilter settings               */
/*********************************************************/

// Should we support codec filters at all?:
#define MUROARD_FEATURE_CODECFILTER

// Should we support the Sun's AU container?:
#define MUROARD_FEATURE_CODECFILTER_AU

// Should we support M$'s RIFF/WAVE container?:
#define MUROARD_FEATURE_CODECFILTER_RIFF_WAVE

// Should we support Xiph's Ogg Vorbis?:
//#define MUROARD_FEATURE_CODECFILTER_OGG_VORBIS

/*********************************************************/
/* below you find the stream settings                    */
/*********************************************************/

// Should we support MONITOR streams?
#define MUROARD_FEATURE_MONITOR

// Should we support stream position handling?:
#define MUROARD_FEATURE_POSITION

// Should we support re-reading and re-writing?:
// This does not work on win32 as we need nonblocking sockets
// for this to work correctly.
#define MUROARD_FEATURE_REREADWRITE

/*********************************************************/
/* below you find client and stream options              */
/*********************************************************/

// Maximum number of clients on this server:
#define MUROARD_MAX_CLIENTS                8

// Maximum number of streams on this server:
#define MUROARD_MAX_STREAMS                8

// Support internal client (used for background streams):
#define MUROARD_FEATURE_INTERNAL_CLIENT

// Support sources and additional sinks?:
#define MUROARD_FEATURE_SOURCES

/*********************************************************/
/* below you find driver options                         */
/*********************************************************/

// Which driver should we build?
// Keep undefined to use internal defaults.
//#define MUROARD_FEATURE_DRIVER             MUROARD_DRIVER_OSS

// Default device to be used when user does not give one or
// when compiled without MUROARD_FEATURE_ARGV.
// Use NULL for driver's internal default.
#define MUROARD_DEFAULT_DRIVER_DEV         NULL

// Client name to be used by the driver.
// This is only used by drivers for sound interface
// which have some kind of client concept.
// This string is commonly used to identify the client
// to the user.
#define MUROARD_DEFAULT_DRIVER_CLIENT_NAME "muRoarD"

// Default device for OSS:
// Comment this out for no default device.
#define MUROARD_DEFAULT_DRIVER_OSS_DEV     "/dev/audio"

// Support setting of OSS fragment size?
// This is recommneted as it will lead in extra-high latency
// on some systems if not enabled.
#define MUROARD_FEATURE_DRIVER_OSS_SETFRAGMENT

// The number of fragments (blocks) used in the ring buffer of
// the WMM driver.
#define MUROARD_FEATURE_DRIVER_WMM_FRAGMENTS 8

/*********************************************************/
/* below you find protocol options                       */
/*********************************************************/

// This is the maximum size of the data part of a
// RoarAudio protocol message:
#define MUROARD_MAX_MSGSIZE                256


/*********************************************************/
/* below you find command options                        */
/*********************************************************/

// Handle some commands we do not support as noops?:
// Some of the commands can safely be handel as noop.
// This may help with random clients, must be considered
// a workarounds.
#define MUROARD_FEATURE_NOOP_SIUCMDS

// Support WHOAMI command?:
// This command is not used by much clients but not much
// code is needed to implemeent this.
// keep enabled if you do not require to safe every byte
// you can.
#define MUROARD_FEATURE_CMD_WHOAMI

// Support EXIT command?:
// This enables the EXIT command. If you want to support
// the termination option of this command, too, you need to
// enable MUROARD_FEATURE_TERMINATE in addition to this option.
#define MUROARD_FEATURE_CMD_EXIT

// Support terminate feature?:
// This will clients enable to set the server into
// termination state. In termination state it will
// terminate as soon as the last client disconnects.
// while in termination state the server will not accept
// new connections and closes the listening socket so a
// new instance of the server can re-open it.
#define MUROARD_FEATURE_TERMINATE

// Support ATTACH command?:
// This will enable simple attaches.
// it will not work with background streams but will
// enable the use-execed workaround of libroar.
#define MUROARD_FEATURE_CMD_ATTACH

// Support GET_STANDBY command?:
// As this server does not support standby will
// allways return 'server active' to client.
#define MUROARD_FEATURE_CMD_GET_STANDBY

// Support SERVER_OINFO?:
// This command is used by the client to get infos on
// native format of server. This may be very helpfull
// as we do not support much formats.
#define MUROARD_FEATURE_CMD_SERVER_OINFO

// Support PSSFH command?:
// This enables the client for passive streams and multi-stream
// architectures.
#define MUROARD_FEATURE_CMD_PASSFH

// Support GET_VOL command?:
// this is just a dummy at the moment, returning a volume of
// 0dB (100%).
#define MUROARD_FEATURE_CMD_GET_VOL

// Support to list clients?:
// this will implement the LIST_CLIENTS command.
// you will most probably also want to enable
// the GET_CLIENT command.
#define MUROARD_FEATURE_CMD_LIST_CLIENTS

// Support to list streams?:
// this will implement the LIST_STREAMS command.
// you will most probably also want to enable
// the GET_STREAMS command.
#define MUROARD_FEATURE_CMD_LIST_STREAMS

// Support to get infos on a client?:
// this implements the GET_CLIENT command.
#define MUROARD_FEATURE_CMD_GET_CLIENT

// Support sending a dummy name for clients?:
#define MUROARD_FEATURE_GET_CLIENT_DUMMYNAME

// Support to get infos on a stream?:
// this implements the GET_STREAM command.
#define MUROARD_FEATURE_CMD_GET_STREAM

// Support KICK command?:
// This enables clients to ckick streams and clients.
#define MUROARD_FEATURE_CMD_KICK

// Support CAPS command?:
// This enables the CAPS/Standards command, which is used by clients
// to find out what features are supported by the server.
#define MUROARD_FEATURE_CMD_CAPS

// Support for SERVER_INFO command?:
// This enables support for SERVER_INFO/IT_SERVER command.
// This command can be used by clients to get the version string
// of this server. Other data is currently not supported.
#define MUROARD_FEATURE_CMD_SERVER_INFO

// Support GETTIMEOFDAY command?:
// This enables support for GETTIMEOFDAY command.
// this command is used to get server's system time.
#define MUROARD_FEATURE_CMD_GETTIMEOFDAY

/*********************************************************/
/* below you find network options                        */
/*********************************************************/

// support for client socket passing:
#define MUROARD_FEATURE_CLIENT_FH

// support to listen on sockets:
#define MUROARD_FEATURE_SOCKET_LISTEN

// support for IPv4 sockets:
#define MUROARD_FEATURE_SOCKET_INET

// support for UNIX sockets:
//#define MUROARD_FEATURE_SOCKET_UNIX

// support for DECnet sockets:
//#define MUROARD_FEATURE_SOCKET_DECNET

// default type for listen sockets:
// ensure that this type is enabled above!
// keep undefined to use internal default.
//#define MUROARD_DEFAULT_LISTEN_TYPE        MUROARD_NETWORK_TYPE_INET

// default address of UNIX socket to listen on:
//#define MUROARD_DEFAULT_LISTEN_ADDR_UNIX   "/tmp/muroard"

// default address of IPv4 host to listen on:
#define MUROARD_DEFAULT_LISTEN_ADDR_INET   "0.0.0.0"

// default address of DECnet host to listen on:
//#define MUROARD_DEFAULT_LISTEN_ADDR_DECNET "::roar"

// default port (only IPv4) to listen on:
// port 16002 is RoarAudio default
#define MUROARD_DEFAULT_LISTEN_PORT        16002

// File Mode to set on opened UNIX Sockets:
// undef to not use this.
#define MUROARD_DEFAULT_LISTEN_CHMOD       0770


/*********************************************************/
/* below you find memory mangement options               */
/*********************************************************/

// Use internal malloc rutines:
// It is only useful in case when there is no malloc() for your system or
// you want to enforce muroard is locked in memory.
//#define MUROARD_MEMMGR_NOUSE_MALLOC

// Try to lock all memory?:
// This is useful to ensure muRoarD running smoothly in an environment low on memory.
// It is strongly recommended to use MUROARD_MEMMGR_NOUSE_MALLOC with this option.
//#define MUROARD_MEMMGR_LOCK_ALL

// Should we fail if memory locking can not be done?
// This is only useful in very special environments.
//#define MUROARD_MEMMGR_LOCK_ALL_FAIL_ON_ERROR

// Flags to pass to mlockall()?:
// Most systems provide MCL_CURRENT and MCL_FUTURE.
#define MUROARD_MEMMGR_LOCK_ALL_FLAGS MCL_CURRENT

/*********************************************************/
/* below you find features which are not yet implemented */
/*********************************************************/

/*
 * #define MUROARD_FEATURE_UNSIGNED_8BIT
 * #define MUROARD_FEATURE_UNSIGNED_16BIT
 * #define MUROARD_FEATURE_UNSIGNED
 * #define MUROARD_FEATURE_UPMIX
 * #define MUROARD_FEATURE_DOWNMIX
 * #define MUROARD_FEATURE_CMD_BEEP
 * #define MUROARD_FEATURE_CMD_CON_STREAM // #39
 * #define MUROARD_FEATURE_META_DATA      // #42
 * #define MUROARD_FEATURE_STREAM_FLAGS   // #43
*/

#endif

//ll
