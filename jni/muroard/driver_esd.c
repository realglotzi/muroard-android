//driver_esd.c:

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
#include <esd.h>

#define DRIVER_HAVE_INIT
#define DRIVER_USE_SYSIO

int muroard_driver_init(const char * dev) {
 esd_format_t fmt = ESD_PLAY|ESD_STREAM|ESD_BITS16;

 switch (muroard_state_member(sa_channels)) {
  case 1:  fmt |= ESD_MONO;   break;
  case 2:  fmt |= ESD_STEREO; break;
  default:
    return -1;
   break;
 }

 muroard_state_member(driver_fh) = esd_play_stream_fallback(fmt, muroard_state_member(sa_rate), dev, MUROARD_DEFAULT_DRIVER_CLIENT_NAME);
 if ( muroard_state_member(driver_fh) == -1 )
  return -1;

 return 0;
}

//ll
