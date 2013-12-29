//waveform.c:

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

#ifdef MUROARD_FEATURE_STANDBY
#define STANDBY_ACTIVE     0x01
#define STANDBY_AUTOACTIVE 0x02
#define STANDBY_AUTO       0x20
#endif

int muroard_waveform_init(void) {
#ifdef MUROARD_FEATURE_STANDBY
 muroard_state_member(standby) = 0
#ifdef MUROARD_FEATURE_STANDBY_AUTOMODE
                           |STANDBY_AUTO
#endif
#ifdef MUROARD_FEATURE_STANDBY_ACTIVE
                           |STANDBY_ACTIVE
#endif
                           ;
#endif

#ifdef MUROARD_FIXED_ABUFFER_SIZE
 muroard_state_member(abuffer_size) = MUROARD_FIXED_ABUFFER_SIZE;
#else
 muroard_state_member(abuffer_size) = muroard_state_member(sa_channels) * 2 * MUROARD_DEFAULT_FRAMES_PER_CYCLE;
#endif

 if ( (muroard_state_member(waveform_mixercore) = muroard_malloc(muroard_state_member(abuffer_size))) == NULL )
  return -1;

 return 0;
}

int muroard_waveform_free(void) {
 if ( muroard_state_member(waveform_mixercore) != NULL )
  muroard_free(muroard_state_member(waveform_mixercore));

 return 0;
}

int muroard_waveform_update(enum muroard_blocking blocking) {
 int16_t * out = NULL;
 int16_t * streams[MUROARD_MAX_STREAMS];
 size_t scount = 0;
 int i;
#ifdef MUROARD_FEATURE_VOLUME_CONTROL
 uint16_t volume;
#endif
 ssize_t ret;
 size_t todo = muroard_state_member(abuffer_size) - muroard_state_member(waveform_offset);

 if ( !muroard_state_member(waveform_offset) ) {
  for (i = 0; i < MUROARD_MAX_STREAMS; i++) {
   if ( muroard_stream_get_datadir(i) == STREAM_DATADIR_IN ) {
    streams[scount] = muroard_stream_get_iobuf(i);
    if ( streams[scount] != NULL ) {
#ifdef MUROARD_FEATURE_VOLUME_CONTROL
#ifdef MUROARD_FEATURE_STANDBY
     if ( (muroard_state_member(standby) & STANDBY_ACTIVE) || (volume = muroard_stream_get_volume(i)) == 65535 ) {
#else
     if ( (volume = muroard_stream_get_volume(i)) == 65535 ) {
#endif
      // full volume
      scount++;
     } else {
      if ( volume ) {
       // non-muted
       muroard_dsp_volume(streams[scount], todo/2, muroard_stream_get_volume(i));
       scount++;
      }
     }
#else
     scount++;
#endif
    }
   }
  }

#ifdef MUROARD_FEATURE_STANDBY
  if ( !(muroard_state_member(standby) & STANDBY_ACTIVE) ) {
#endif
#ifdef MUROARD_FEATURE_OPTIMIZE
   if ( muroard_state_member(waveform_last_scount) == 0 && scount == 0 ) {
    out = muroard_state_member(waveform_mixercore);
   } else {
#endif
    if ( scount == 1 ) {
     out = streams[0];
    } else {
     if ( muroard_dsp_mixer(muroard_state_member(waveform_mixercore), streams, scount) == -1 )
      return -1;
     out = muroard_state_member(waveform_mixercore);
    }
#ifdef MUROARD_FEATURE_OPTIMIZE
   }

   muroard_state_member(waveform_last_scount) = scount;
#endif
#ifdef MUROARD_FEATURE_STANDBY
  }
#endif
 }

#ifdef MUROARD_FEATURE_STANDBY
 if ( scount == 0 && (muroard_state_member(standby) & STANDBY_AUTO) ) {
  //printf("-> STANDBY_AUTOACTIVE\n");
  muroard_state_member(standby) |= STANDBY_AUTOACTIVE;
 } else if ( scount != 0 && (muroard_state_member(standby) & STANDBY_AUTOACTIVE) ) {
  //printf("<- STANDBY_AUTOACTIVE\n");
  muroard_state_member(standby) -= STANDBY_AUTOACTIVE;
 }

 ret = todo;
 if ( (muroard_state_member(standby) & (STANDBY_ACTIVE|STANDBY_AUTOACTIVE)) == 0 ) {
  if ( (ret = muroard_driver_write(out+muroard_state_member(waveform_offset), todo, blocking)) == -1 )
   return -1;
 } else if ( scount != 0 && blocking != MUROARD_BLOCKING_NONE ) {
#ifndef __WIN32
  usleep(todo*1000000LLU/(long long unsigned)(muroard_state_member(sa_rate)*muroard_state_member(sa_channels)*2));
#else
  Sleep(todo*1000LLU/(long long unsigned)(muroard_state_member(sa_rate)*muroard_state_member(sa_channels)*2));
#endif
 }
#else
 if ( (ret = muroard_driver_write(out+muroard_state_member(waveform_offset), todo, blocking)) == -1 )
  return -1;
#endif

 if ( ret == -1 )
  return -1;

 if ( ret < (ssize_t)todo ) {
  muroard_state_member(waveform_offset) += (ssize_t)ret;
 } else {
  muroard_state_member(waveform_offset) = 0;
 }

#ifdef MUROARD_FEATURE_MONITOR
 if ( !muroard_state_member(waveform_offset) ) {
  for (i = 0; i < MUROARD_MAX_STREAMS; i++) {
   if ( muroard_stream_get_datadir(i) == STREAM_DATADIR_OUT ) {
    muroard_stream_write(i, out);
   }
  }
 }
#endif

#ifdef MUROARD_FEATURE_STANDBY
 return (muroard_state_member(standby) & (STANDBY_ACTIVE|STANDBY_AUTOACTIVE)) == 0 ? 0 : 1;
#else
 return 0;
#endif
}

#ifdef MUROARD_FEATURE_STANDBY
int muroard_waveform_standby(int manual_mode, int auto_mode) {
// printf("muroard_waveform_standby(manual_mode=%i, auto_mode=%i) = ?\n", manual_mode, auto_mode);

 if ( manual_mode != -1 ) {
  muroard_state_member(standby) |= STANDBY_ACTIVE;
  if ( manual_mode == 0 )
   muroard_state_member(standby) -= STANDBY_ACTIVE;
 }

 if ( auto_mode != -1 ) {
  muroard_state_member(standby) |= STANDBY_AUTO;
  if ( auto_mode == 0 ) {
   muroard_state_member(standby) |= STANDBY_AUTOACTIVE;
   muroard_state_member(standby) -= STANDBY_AUTO|STANDBY_AUTOACTIVE;
  }
 }

 return (muroard_state_member(standby) & (STANDBY_ACTIVE|STANDBY_AUTOACTIVE)) == 0 ? 0 : 1;
}
#endif

//ll
