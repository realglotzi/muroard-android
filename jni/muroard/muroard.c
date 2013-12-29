//muroard.c:

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

#include "muroard.h"
#include <android/log.h>

#if defined(MUROARD_FEATURE_DAEMONIZE) && defined(__WIN32)
#include <windows.h>
#endif

struct muroard_state * muroard_g_state;

#ifdef MUROARD_FEATURE_HELP_TEXT
void muroard_usage (void) {
 printf("Usage: muroard [OPTIONS]...\n\n");

 printf("Misc Options:\n\n");
 printf(" -h --help             - Print this help\n"
#ifdef MUROARD_FEATURE_VERSION_TEXT
        "    --version          - Print version string\n"
#endif
#ifdef MUROARD_FEATURE_DAEMONIZE
        "    --daemon           - Bring the server into background after init\n"
#endif
#ifdef MUROARD_FEATURE_STANDBY
        "    --autostandby      - Start in autostandby mode"
#ifdef MUROARD_FEATURE_STANDBY_AUTOMODE
                                  " (default)\n"
#else
                                  "\n"
#endif
        "    --no-autostandby   - Do not start in autostandby mode"
#ifdef MUROARD_FEATURE_STANDBY_AUTOMODE
                                  "\n"
#else
                                  " (default)\n"
#endif

        "    --standby          - Start in standby mode"
#ifdef MUROARD_FEATURE_STANDBY_ACTIVE
                                  " (default)\n"
#else
                                  "\n"
#endif
        "    --no-standby       - Do not start in standby mode"
#ifdef MUROARD_FEATURE_STANDBY_ACTIVE
                                  "\n"
#else
                                  " (default)\n"
#endif
#endif
       );

 printf("\nAudio Options:\n\n");
 printf(
        " -R  --rate   RATE     - Set server rate\n"
        " -C  --chans  CHANNELS - Set server channels\n"
       );

 printf("\nDriver Options:\n\n");
 printf(
        " -O  --odevice DEV     - Set output device\n"
       );

#ifdef MUROARD_FEATURE_SOURCES
 printf("\nSources Options:\n\n");
 printf(
        " -S DEV                - Set source device or filename\n"
        " -sC CHANS             - Set source number of channels\n"
        " -sB BITS              - Set source number of bits\n"
        " -sE CODEC             - Set source codec (no symbolic names)\n"
//        " -sD DIR               - Set source direction: in or out\n"
       );
#endif

 printf("\nNetwork Options:\n\n");
 printf(""
#ifdef MUROARD_FEATURE_SOCKET_LISTEN
        "     --no-listen       - Disable listen socket\n"
#endif
#ifdef MUROARD_FEATURE_CLIENT_FH
        "     --client-fh FH    - Add a client via FH\n"
#endif
#ifdef MUROARD_FEATURE_SOCKET_LISTEN
        "     --bind ADDR       - Set path/hostname of listen socket\n"
        "     --port PORT       - Set port of listen socket\n"
#endif
#ifdef MUROARD_FEATURE_SOCKET_UNIX
        " -u  --unix            - Use UNIX Domain listen socket\n"
#endif
#ifdef MUROARD_FEATURE_SOCKET_INET
        " -t  --tcp             - Use TCP listen socket\n"
#endif
#ifdef MUROARD_FEATURE_SOCKET_DECNET
        " -n  --decnet          - Use DECnet listen socket\n"
#endif
       );
}
#else
#define muroard_usage()
#endif

#ifdef MUROARD_FEATURE_VERSION_TEXT
void muroard_version (void) {
 printf("Version: %s\n", MUROARD_VERSION);
}
#endif

// signal handler:
#ifdef MUROARD_FEATURE_SIGNALS
void muroard_on_sig(int sig) {
 (void)sig; // we do not need this parameter

 muroard_state_member(alive) = 0; // terminate daemon
}
#endif

#ifdef MUROARD_FEATURE_SIGNALS
void muroard_init_signals(void) {
 signal(SIGINT,  muroard_on_sig);
 signal(SIGTERM, muroard_on_sig);
 signal(SIGPIPE, SIG_IGN);
}
#endif

#ifdef MUROARD_FEATURE_DAEMONIZE
void muroard_daemonize(void) {
#ifdef __WIN32
 FreeConsole();
#else
 close(STDIN_FILENO);
 close(STDOUT_FILENO);
 close(STDERR_FILENO);

 if ( fork() )
  exit(0);

 setsid();
#endif
}
#endif

int muroard_init(void) {
 if ( muroard_memmgr_init() == -1 ) {
  return -1;
 }

#ifdef MUROARD_MEMMGR_LOCK_ALL
#ifdef MUROARD_MEMMGR_LOCK_ALL_FAIL_ON_ERROR
 if ( muroard_memmgr_lock_all() == -1 ) {
  return -1;
 }
#else
 muroard_memmgr_lock_all();
#endif
#endif

 if ( muroard_network_init() == -1 ) {
  return -1;
 }

 if ( muroard_waveform_init() == -1 ) {
  return -1;
 }

 if ( muroard_client_init() == -1 ) {
  return -1;
 }

 if ( muroard_stream_init() == -1 ) {
  return -1;
 }

 return 0;
}

int muroard_mainloop_iteration(enum muroard_blocking network_blocking, enum muroard_blocking audio_blocking) {
 int ret;

 if ( muroard_network_check(network_blocking) == -1 ) {
  muroard_state_member(alive) = 0;
  return -1;
 }

 if ( (ret = muroard_waveform_update(audio_blocking)) == -1 ) {
  muroard_state_member(alive) = 0;
  return -1;
 }

 return ret;
}

void muroard_mainloop(void) {
#ifdef MUROARD_FEATURE_STANDBY
 int ret = 0;
#endif

 while (muroard_state_member(alive)) {
#ifdef MUROARD_FEATURE_STANDBY
  ret = muroard_mainloop_iteration(ret == 0 ? MUROARD_BLOCKING_NONE : MUROARD_BLOCKING_LONG, MUROARD_BLOCKING_LONG);
  if ( ret == -1 )
   break;
#else
  if ( muroard_mainloop_iteration(MUROARD_BLOCKING_NONE, MUROARD_BLOCKING_LONG) == -1 )
   break;
#endif
 }
}

void muroard_shutdown(void) {
 muroard_network_prefree(); // this will free the listen socket early

 muroard_driver_free();
 muroard_stream_free();
 muroard_client_free();
 muroard_waveform_free();
 muroard_network_free();
}

int muroard_main_initonly (int argc, char * argv[]) {

 __android_log_print(ANDROID_LOG_DEBUG, "muroard_main_initonly", "start");
#ifdef MUROARD_FEATURE_SOCKET_LISTEN
 int    n_type = MUROARD_DEFAULT_LISTEN_TYPE;
 char * n_addr = NULL; // use defualts from network module
 int    n_port = MUROARD_DEFAULT_LISTEN_PORT;
 int    n_no_listen = 0;
#endif
 int i;
#ifdef MUROARD_FEATURE_ARGV
 char * k;
 char * o_dev = MUROARD_DEFAULT_DRIVER_DEV;
#ifdef MUROARD_FEATURE_DAEMONIZE
 int daemon = 0;
#endif
#endif
#ifdef MUROARD_FEATURE_CLIENT_FH
 muroard_handle_t c_sock[MUROARD_MAX_CLIENTS];
 int              c_sock_count = 0; // this is int so we can use 'i' in the for(){}.

#endif
#ifdef MUROARD_FEATURE_SOURCES
 const char * s_filename = NULL;
 int          s_dir = STREAM_DATADIR_IN;
 struct muroard_audio_info s_info = {.channels = muroard_state_member(sa_channels),
                                     .bits = 16,
                                     .codec = MUROAR_CODEC_PCM_S};
#endif

 muroard_para_init(muroard_g_state);

#ifdef MUROARD_FEATURE_ARGV
 for (i = 1; i < argc; i++) {
  k = argv[i];

#ifdef MUROARD_FEATURE_HELP_TEXT
  if ( !strcmp(k, "--help") || !strcmp(k, "-h") ) {
   muroard_usage();
   return 0;
#else
  if ( 0 ) {
   // noop.
#endif
#ifdef MUROARD_FEATURE_VERSION_TEXT
  } else if ( !strcmp(k, "--version") ) {
   muroard_version();
   return 0;
#endif
#ifdef MUROARD_FEATURE_DAEMONIZE
  } else if ( !strcmp(k, "--daemon") ) {
   daemon = 1;
#endif
#ifdef MUROARD_FEATURE_STANDBY
  } else if ( !strcmp(k, "--autostandby") ) {
   muroard_waveform_standby(-1, 1);
  } else if ( !strcmp(k, "--no-autostandby") ) {
   muroard_waveform_standby(-1, 0);
  } else if ( !strcmp(k, "--standby") ) {
   muroard_waveform_standby(1, -1);
  } else if ( !strcmp(k, "--no-standby") ) {
   muroard_waveform_standby(0, -1);
#endif
  } else if ( !strcmp(k, "--rate") || !strcmp(k, "-R") ) {
   muroard_state_member(sa_rate) = atoi(argv[++i]);
  } else if ( !strcmp(k, "--channels") || !strcmp(k, "-C") ) {
   muroard_state_member(sa_channels) = atoi(argv[++i]);
  } else if ( !strcmp(k, "--odevice") || !strcmp(k, "-O") ) {
   o_dev  = argv[++i];
#ifdef MUROARD_FEATURE_SOURCES
  } else if ( !strcmp(k, "-S") ) {
   s_filename = argv[++i];
  } else if ( !strcmp(k, "-sC") ) {
   s_info.channels = atoi(argv[++i]);
  } else if ( !strcmp(k, "-sB") ) {
   s_info.bits = atoi(argv[++i]);
  } else if ( !strcmp(k, "-sE") ) {
   s_info.codec = atoi(argv[++i]);
  } else if ( !strcmp(k, "-sD") ) {
   i++;
   if ( !strcmp(argv[i], "in") ) {
    s_dir = STREAM_DATADIR_IN;
   } else if ( !strcmp(argv[i], "out") ) {
    s_dir = STREAM_DATADIR_OUT;
   } else {
    fprintf(stderr, "Error: Unknown stream direction: %s.\n", argv[i]);
    return 1;
   }
/* TODO: this is currently not supported as the init for the streams will overwrite it.
  } else if ( !strcmp(k, "-sN") ) {
   if ( s_filename != NULL ) {
    if ( muroard_stream_new_source(s_filename, s_dir, &s_info) == -1 ) {
     fprintf(stderr, "Error: Can not add source/sink: %s.\n", s_filename);
    }
   }
   s_filename = NULL;
   s_dir = STREAM_DATADIR_IN;
   s_info.channels = muroard_g_sa_channels;
   s_info.bits = 16;
   s_info.codec = MUROAR_CODEC_PCM_S;
*/
#endif
#ifdef MUROARD_FEATURE_SOCKET_LISTEN
  } else if ( !strcmp(k, "--no-listen") ) {
   n_no_listen = 1;
#endif
#ifdef MUROARD_FEATURE_CLIENT_FH
  } else if ( !strcmp(k, "--client-fh") ) {
   c_sock[c_sock_count++] = atoi(argv[++i]);
#endif
#ifdef MUROARD_FEATURE_SOCKET_LISTEN
  } else if ( !strcmp(k, "--bind") ) {
   n_addr = argv[++i];
  } else if ( !strcmp(k, "--port") ) {
   n_port = atoi(argv[++i]);
#ifdef MUROARD_FEATURE_SOCKET_UNIX
  } else if ( !strcmp(k, "--unix") || !strcmp(k, "-u") ) {
    n_type = MUROARD_NETWORK_TYPE_UNIX;
#endif
#ifdef MUROARD_FEATURE_SOCKET_INET
  } else if ( !strcmp(k, "--tcp") || !strcmp(k, "-t") ) {
    n_type = MUROARD_NETWORK_TYPE_INET;
#endif
#ifdef MUROARD_FEATURE_SOCKET_DECNET
  } else if ( !strcmp(k, "--decnet") || !strcmp(k, "-n") ) {
    n_type = MUROARD_NETWORK_TYPE_DECNET;
#endif
#endif
  } else {
   muroard_usage();
   return 1;
  }
 }
#endif

  __android_log_print(ANDROID_LOG_DEBUG, "muroard_main_initonly", " muroard_init()");
 if ( muroard_init() == -1 )
  return 1;

#ifdef MUROARD_FEATURE_ARGV
 if ( muroard_driver_init(o_dev) == -1 ) {
#else
 if ( muroard_driver_init(MUROARD_DEFAULT_DRIVER_DEV) == -1 ) {
#endif
  return 3;
 }

#ifdef MUROARD_FEATURE_SOURCES
 if ( s_filename != NULL ) {
  if ( muroard_stream_new_source(s_filename, s_dir, &s_info) == -1 ) {
   fprintf(stderr, "Error: Can not add source/sink: %s.\n", s_filename);
  }
 }
#endif

#ifdef MUROARD_FEATURE_CLIENT_FH
 for (i = 0; i < c_sock_count; i++) {
  if ( muroard_client_new(c_sock[i]) == -1 ) {
   return 1;
  }
 }
#endif

#ifdef MUROARD_FEATURE_SOCKET_LISTEN
 if ( !n_no_listen ) {
  if ( muroard_network_listen(n_type, n_addr, n_port) == -1 ) {
   return 2;
  }
 }
#endif

 // everything is up, get it running!:
 muroard_state_member(alive) = 1;

#ifdef MUROARD_FEATURE_SIGNALS
 muroard_init_signals();
#endif

#ifdef MUROARD_FEATURE_DAEMONIZE
 if ( daemon ) {
  muroard_daemonize();
 }
#endif

 return 0;
}

int muroard_main (int argc, char * argv[]) {
 int ret;

 if ( (ret = muroard_main_initonly(argc, argv)) != 0 )
  return ret;

 muroard_mainloop();

 muroard_shutdown();

 return 0;
}

//ll
