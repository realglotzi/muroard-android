//driver_ao.c:

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
#include <ao/ao.h>

#define DRIVER_HAVE_INIT
#define DRIVER_HAVE_FREE
#define DRIVER_HAVE_WRITE

int muroard_driver_init(const char * dev) {
 ao_sample_format  format;
 int               driver;
#ifdef HAVE_LIBAO_MATRIX
 char * map = NULL;
#endif

 ao_initialize();

 if ( dev == NULL ) {
  driver = ao_default_driver_id();
 } else {
  driver = ao_driver_id(dev);
 }

 if (driver < 0)
  return -1;

 muroard_memzero(&format, sizeof(format));

 format.bits        = 16;
 format.channels    = muroard_state_member(sa_channels);
 format.rate        = muroard_state_member(sa_rate);

 format.byte_format = AO_FMT_NATIVE;

#ifdef HAVE_LIBAO_MATRIX
 switch (muroard_state_member(sa_channels)) {
  case  1: map = "M";               break;
  case  2: map = "L,R";             break;
  case  3: map = "L,R,C";           break;
  case  4: map = "L,R,BL,BR";       break;
  case  5: map = "L,R,C,BL,BR";     break;
  case  6: map = "L,R,C,LFE,BL,BR"; break;
 }

 if ( map != NULL ) {
  format.matrix = map;
 }
#endif

 muroard_state_member(driver_vp) = ao_open_live(driver, &format, NULL /* no options */);

 if ( muroard_state_member(driver_vp) == NULL )
  return -1;

 return 0;
}

int muroard_driver_free(void) {
 if ( muroard_state_member(driver_vp) != NULL )
  ao_close(muroard_state_member(driver_vp));

 ao_shutdown();
 return 0;
}

ssize_t muroard_driver_write(int16_t * data, size_t len, enum muroard_blocking blocking) {
 if ( blocking == MUROARD_BLOCKING_NONE )
  return -1;

 if ( ao_play(muroard_state_member(driver_vp), (char*)data, len) == 0 )
  return -1;
 return 0;
}

//ll
