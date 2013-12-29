//muroarstream.c:

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

#include "muroar.h"
#ifndef __WIN32
#include <errno.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#ifndef O_LARGEFILE
#define O_LARGEFILE 0
#endif

#define NAME "muroarstream"

static int str2chans(const char * chans) {
 if ( !strcasecmp(chans, "stereo") )
  return MUROAR_CM_STEREO;
 if ( !strcasecmp(chans, "mono") )
  return MUROAR_CM_MONO;
 if ( !strcasecmp(chans, "default") )
  return MUROAR_AID_CHANNELS;
 return atoi(chans);
}

static int str2codec(const char * codec) {
 if ( !strcasecmp(codec, "default") )
  return -1;
 if ( !strcasecmp(codec, "pcm") || !strcasecmp(codec, "pcm_s") )
  return MUROAR_CODEC_PCM_S;
 if ( !strcasecmp(codec, "pcm_u") )
  return MUROAR_CODEC_PCM_U;
 if ( !strcasecmp(codec, "alaw") )
  return MUROAR_CODEC_ALAW;
 if ( !strcasecmp(codec, "mulaw") || !strcasecmp(codec, "ulaw") )
  return MUROAR_CODEC_MULAW;
 if ( !strcasecmp(codec, "ogg_vorbis") || !strcasecmp(codec, "vorbis") )
  return MUROAR_CODEC_OGG_VORBIS;
 if ( !strcasecmp(codec, "dmx512") )
  return MUROAR_CODEC_DMX512;
 if ( !strcasecmp(codec, "roardmx") )
  return MUROAR_CODEC_ROARDMX;
 if ( !strcasecmp(codec, "midi") )
  return MUROAR_CODEC_MIDI;
 return atoi(codec);
}

void usage (char * progname) {
 printf("Usage: %s [OPTIONS]\n", progname);
 printf("\n");
 printf("Options:\n");
 printf("    --help              - Display this help.\n"
        " -f --file     FILE     - File to read from or write to. (default: stdin/stdout)\n"
        "    --server   SERVER   - Sets the server. (default: autodetect)\n"
        " -R --rate     RATE     - Sets the sample rate (default: 44100Hz)\n"
        " -C --channels CHANS    - Sets the number of channels (default: 2 channels)\n"
        " -B --bits     BITS     - Sets the number of bits per sample (default: 16bit)\n"
        " -E --codec    CODEC    - Set codec to use (default: depend on used subsystem)\n"
        "    --play              - Use Playback mode (send data to the server) (default)\n"
        "    --monitor           - Use monitor mode (get a copy of the played data from the server)\n"
        "    --wave              - Use Waveform subsystem (default)\n"
        "    --midi              - Use MIDI subsystem\n"
        "    --light             - Use Light Control subsystem\n"
       );
}

void run_stream(muroar_t stream, int file, int tx) {
 char buf[1024];
 ssize_t len;

 if ( tx ) {
  if ( file == -1 )
   file = STDIN_FILENO;

  while ((len = read(file, buf, sizeof(buf))) > 0)
   if ( muroar_write(stream, buf, len) != len )
    return;
 } else {
  if ( file == -1 )
   file = STDOUT_FILENO;

  while ((len = muroar_read(stream, buf, sizeof(buf))) > 0)
   if ( write(file, buf, len) != len )
    return;
 }
}

#ifdef __YIFF__
yiff_uint_least16_t yiff_main(yiff_usize_t argc, char * argv[]) {
#else
int main (int argc, char *argv[]) {
#endif
 muroar_t fh;
 int filefh = -1;
 int dir    = MUROAR_PLAY_WAVE;
 int codec  = -1;     /* use default depending in dir */
 int rate   =  MUROAR_AID_RATE;
 int chans  =  MUROAR_AID_CHANNELS;
 int bits   =  MUROAR_AID_BITS;
 int tx     =  1;
#ifdef __YIFF__
 yiff_usize_t i;
#else
 int i;
#endif
 char * k;
 const char * server = NULL;
 const char * file   = NULL;

 for (i = 1; i < argc; i++) {
  k = argv[i];

  if ( !strcmp(k, "--help") ) {
   usage(argv[0]);
   return 0;
  } else if ( !strcmp(k, "--file") || !strcmp(k, "-f") ) {
   file = argv[++i];
  } else if ( !strcmp(k, "--server") ) {
   server = argv[++i];
  } else if ( !strcmp(k, "--rate") || !strcmp(k, "-R") ) {
   rate = atoi(argv[++i]);
  } else if ( !strcmp(k, "--channels") || !strcmp(k, "-C") ) {
   chans = str2chans(argv[++i]);
  } else if ( !strcmp(k, "--bits") || !strcmp(k, "-B") ) {
   bits = atoi(argv[++i]);
  } else if ( !strcmp(k, "--codec") || !strcmp(k, "-E") ) {
   codec = str2codec(argv[++i]);
  } else if ( !strcmp(k, "--play") ) {
   tx = 1;
  } else if ( !strcmp(k, "--monitor") ) {
   tx = 0;
  } else if ( !strcmp(k, "--wave") ) {
   dir = MUROAR_PLAY_WAVE;
  } else if ( !strcmp(k, "--midi") ) {
   dir = MUROAR_PLAY_MIDI;
  } else if ( !strcmp(k, "--light") ) {
   dir = MUROAR_PLAY_LIGHT;
  } else {
   fprintf(stderr, "%s: Unknown argument: %s\n", argv[0], k);
   return 4;
  }
 }

 // Set codec depending on dir in case codec is default (-1):
 if ( codec == -1 ) {
  switch (dir) {
   case MUROAR_PLAY_WAVE:  codec = MUROAR_CODEC_PCM;    break;
   case MUROAR_PLAY_MIDI:  codec = MUROAR_CODEC_MIDI;   break;
   case MUROAR_PLAY_LIGHT: codec = MUROAR_CODEC_DMX512; break;
  }
 }

 // Correct dir in case we do not transmit to MONITOR dirs:
 if ( !tx ) {
  switch (dir) {
   case MUROAR_PLAY_WAVE:  dir = MUROAR_MONITOR_WAVE;  break;
   case MUROAR_PLAY_MIDI:  dir = MUROAR_MONITOR_MIDI;  break;
   case MUROAR_PLAY_LIGHT: dir = MUROAR_MONITOR_LIGHT; break;
  }
 }

 if ( file != NULL ) {
  filefh = open(file, O_LARGEFILE|(tx ? O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC), 0666);
  if ( filefh == -1 ) {
   fprintf(stderr, "Error: Can not open file: %s\n", file);
   return 5;
  }
 }

 // Connect to server:
 if ( (fh = muroar_connect(server, NAME)) == MUROAR_HANDLE_INVALID ) {
#ifndef __WIN32
  fprintf(stderr, "Error: Can not connect to server: %s: %s.\n",
                  server == NULL ? "(default)" : server, strerror(errno));
#else
  fprintf(stderr, "Error: Can not connect to server.\n");
#endif
  return 1;
 }

 if ( (fh = muroar_stream(fh, dir, NULL, codec, rate, chans, bits)) == MUROAR_HANDLE_INVALID ) {
#ifndef __WIN32
  fprintf(stderr, "Error: Can not connect stream to server: %s\n", strerror(errno));
#else
  fprintf(stderr, "Error: Can not connect stream to server.\n");
#endif
  // We do not need to close fh here as it is already closed by muroar_stream().
  return 2;
 }

 run_stream(fh, filefh, tx);

 muroar_close(fh);

 if ( filefh != -1 )
  close(filefh);

 return 0;
}

//ll
