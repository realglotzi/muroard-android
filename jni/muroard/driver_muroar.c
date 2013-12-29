//driver_muroar.c:

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

#define DRIVER_HAVE_INIT
#define DRIVER_USE_SYSIO

int muroard_driver_init(const char * dev) {

 if ( (muroard_state_member(driver_fh) = muroar_connect(dev, MUROARD_DEFAULT_DRIVER_CLIENT_NAME)) == -1 )
  return -1;

 if ( (muroard_state_member(driver_fh) = muroar_stream(muroard_state_member(driver_fh),
                                                       MUROAR_PLAY_WAVE,
                                                       NULL,
                                                       MUROAR_CODEC_PCM,
                                                       muroard_state_member(sa_rate),
                                                       muroard_state_member(sa_channels),
                                                       16)) == -1 )
  return -1;
 
 return 0;
}

//ll
