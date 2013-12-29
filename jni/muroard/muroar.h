//muroar.h:

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

#ifndef _MUROAR_H_
#define _MUROAR_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __YIFF__
// types:
#include <unistd.h>

// BYTE_ORDER and friends:
#ifdef HAVE_HEADER_ENDIAN_H
#include <endian.h>
#endif
#endif

// types of IO:
#define MUROAR_HDLTYPE_POSIX_SYSIO  1
#define MUROAR_HDLTYPE_POSIX_STDIO  2
#define MUROAR_HDLTYPE_WIN32_SOCKET 3
#define MUROAR_HDLTYPE_YIFF_SYSIO   4
#define MUROAR_HDLTYPE_YIFF_VIO     5

#ifdef __WIN32
#define MUROAR_HDLTYPE MUROAR_HDLTYPE_WIN32_SOCKET
#elif defined(__YIFF__)
#define MUROAR_HDLTYPE MUROAR_HDLTYPE_YIFF_VIO
#else
#define MUROAR_HDLTYPE MUROAR_HDLTYPE_POSIX_SYSIO
#endif

#if MUROAR_HDLTYPE == MUROAR_HDLTYPE_POSIX_SYSIO || MUROAR_HDLTYPE == MUROAR_HDLTYPE_WIN32_SOCKET
typedef int muroar_t;
#define MUROAR_HANDLE_INVALID -1
#elif MUROAR_HDLTYPE == MUROAR_HDLTYPE_POSIX_STDIO
#include <stdio.h>
typedef FILE* muroar_t;
#define MUROAR_HANDLE_INVALID NULL
#elif MUROAR_HDLTYPE == MUROAR_HDLTYPE_YIFF_VIO
/* yiff stuff */
#include <YIFF.h>
#include <YIFFc/error.h>
#include <YIFFc/memory.h> /* for stuff like NULL */
#include <YIFFc/io.h>
typedef yiffc_vio_t * muroar_t;
#define MUROAR_HANDLE_INVALID NULL
#error Unsupported value for MUROAR_HDLTYPE (WIP)
#else
#error Unsupported value for MUROAR_HDLTYPE
#endif

// Some defaults:
#define MUROAR_PORT              16002
#define MUROAR_HOST              "localhost"
#define MUROAR_GSOCK             "/tmp/roar"
#define MUROAR_OBJECT            "roar"
#define MUROAR_DEFAULT           "+default"
#define MUROAR_INVALID           "+invalid"
#define MUROAR_ABSTRACT          "+abstract"

// AID = Audio Info Defaults
#define MUROAR_AID_RATE          44100
#define MUROAR_AID_BITS             16
#define MUROAR_AID_CHANNELS          MUROAR_CM_STEREO

#define MUROAR_CM_MONO               1
#define MUROAR_CM_STEREO             2

#define MUROAR_CMD_NOOP              0
#define MUROAR_CMD_IDENTIFY          1
#define MUROAR_CMD_AUTH              2
#define MUROAR_CMD_NEW_STREAM        3
#define MUROAR_CMD_SET_META          4
#define MUROAR_CMD_EXEC_STREAM       5
#define MUROAR_CMD_QUIT              6
#define MUROAR_CMD_GET_STANDBY       7
#define MUROAR_CMD_SET_STANDBY       8
#define MUROAR_CMD_SERVER_INFO       9
#define MUROAR_CMD_SERVER_STATS     10
#define MUROAR_CMD_SERVER_OINFO     11
#define MUROAR_CMD_ADD_DATA         12
#define MUROAR_CMD_EXIT             13
#define MUROAR_CMD_LIST_STREAMS     14
#define MUROAR_CMD_LIST_CLIENTS     15
#define MUROAR_CMD_GET_CLIENT       16
#define MUROAR_CMD_GET_STREAM       17
#define MUROAR_CMD_KICK             18
#define MUROAR_CMD_SET_VOL          19
#define MUROAR_CMD_GET_VOL          20
#define MUROAR_CMD_CON_STREAM       21
#define MUROAR_CMD_GET_META         22
#define MUROAR_CMD_LIST_META        23
#define MUROAR_CMD_BEEP             24
#define MUROAR_CMD_AUTHCTL          25
#define MUROAR_CMD_ACLCTL           26
#define MUROAR_CMD_GET_STREAM_PARA  27
#define MUROAR_CMD_SET_STREAM_PARA  28
#define MUROAR_CMD_ATTACH           29
#define MUROAR_CMD_PASSFH           30
#define MUROAR_CMD_GETTIMEOFDAY     31
#define MUROAR_CMD_WHOAMI           32

#define MUROAR_CMD_DEVCTL           33 /* send control messages to devices */
#define MUROAR_CMD_CAPS             34 /* send caps */
#define MUROAR_CMD_WAIT             35 /* wait for an event */
#define MUROAR_CMD_NOTIFY           36 /* asyncronus notify of events */
#define MUROAR_CMD_SEEK             37 /* seek in stream */
#define MUROAR_CMD_CLIENTCTL        38 /* like stream ctl, just for client connections */
#define MUROAR_CMD_LOOKUP           39 /* lookup??? */
#define MUROAR_CMD_CONCTL           40 /* change parameter for current connection */

// special CMDs:
#define MUROAR_CMD_OK              254 /* return value OK    */
#define MUROAR_CMD_ERROR           255 /* return value ERROR */

// consts for CAPS command:

#define MUROAR_CF_REQUEST       0x0001
#define MUROAR_CT_CAPS               0
#define MUROAR_CT_STANDARDS          1

// stream directions:
#define MUROAR_PLAY_WAVE             1
#define MUROAR_PLAY_MIDI            12
#define MUROAR_PLAY_LIGHT           14
#define MUROAR_RECORD_WAVE           2
#define MUROAR_MONITOR_WAVE          3
#define MUROAR_MONITOR_MIDI         13
#define MUROAR_MONITOR_LIGHT        15
#define MUROAR_FILTER_WAVE           4


// Codecs:
#define MUROAR_CODEC_PCM_S_LE     0x01
#define MUROAR_CODEC_PCM_S_BE     0x02
#define MUROAR_CODEC_PCM_S_PDP    0x03
#define MUROAR_CODEC_PCM_U_LE     0x05
#define MUROAR_CODEC_PCM_U_BE     0x06
#define MUROAR_CODEC_PCM_U_PDP    0x07
#define MUROAR_CODEC_OGG_VORBIS   0x10
#define MUROAR_CODEC_FLAC         0x11
#define MUROAR_CODEC_OGG_SPEEX    0x12
#define MUROAR_CODEC_OGG_FLAC     0x14
#define MUROAR_CODEC_OGG_CELT     0x16
#define MUROAR_CODEC_ROAR_CELT    0x1a
#define MUROAR_CODEC_ROAR_SPEEX   0x1b
#define MUROAR_CODEC_RIFF_WAVE    0x20
#define MUROAR_CODEC_RIFX         0x22
#define MUROAR_CODEC_AU           0x24
#define MUROAR_CODEC_AIFF         0x28
#define MUROAR_CODEC_ALAW         0x30
#define MUROAR_CODEC_MULAW        0x34
#define MUROAR_CODEC_GSM          0x38
#define MUROAR_CODEC_GSM49        0x39
#define MUROAR_CODEC_MIDI         0x60
#define MUROAR_CODEC_DMX512       0x70
#define MUROAR_CODEC_ROARDMX      0x71

#ifdef __WIN32
#define MUROAR_CODEC_PCM_S      MUROAR_CODEC_PCM_S_LE
#define MUROAR_CODEC_PCM_U      MUROAR_CODEC_PCM_U_LE
#else

#if BYTE_ORDER == BIG_ENDIAN
#define MUROAR_CODEC_PCM_S      MUROAR_CODEC_PCM_S_BE
#define MUROAR_CODEC_PCM_U      MUROAR_CODEC_PCM_U_BE
#elif BYTE_ORDER == LITTLE_ENDIAN
#define MUROAR_CODEC_PCM_S      MUROAR_CODEC_PCM_S_LE
#define MUROAR_CODEC_PCM_U      MUROAR_CODEC_PCM_U_LE
#else
// most likely a PDP
#define MUROAR_CODEC_PCM_S      MUROAR_CODEC_PCM_S_PDP
#define MUROAR_CODEC_PCM_U      MUROAR_CODEC_PCM_U_PDP
#endif

#endif

#define MUROAR_CODEC_PCM        MUROAR_CODEC_PCM_S

// muroar.c:
muroar_t muroar_connect(const char * server, const char * name);

// muroar_close.c:
int      muroar_close  (muroar_t fh);

// muroar_quit.c:
int      muroar_quit   (muroar_t fh);

// muroar_beep.c:
int      muroar_beep   (muroar_t fh);

// muroar_stream.c:
muroar_t muroar_stream (muroar_t fh, int dir, int * stream, int codec, int rate, int channels, int bits);

// muroario.c:
ssize_t muroar_write   (muroar_t fh, const void * buf, size_t len);
ssize_t muroar_read    (muroar_t fh,       void * buf, size_t len);

// muroar_noop.c:
int muroar_noop        (muroar_t fh);

// muroar_setvolume.c:
// We use (long unsigned int) here because we need a portable 16 bit type without <stdint.h>.
// (long unsigned int) should be big enough on all kind of archs.
int muroar_setvolume   (muroar_t fh, int stream, long unsigned int left, long unsigned int right);

#ifdef __cplusplus
}
#endif

#endif

//ll
