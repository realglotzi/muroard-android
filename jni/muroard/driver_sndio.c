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
#include <sndio.h>

#define DRIVER_HAVE_INIT
#define DRIVER_HAVE_FREE
#define DRIVER_HAVE_WRITE

/* TODO: FIXME: this driver needs error handlung! */

int muroard_driver_init(const char * dev) {
 struct sio_par par;

 sio_initpar(&par);

 par.bits  = 16;
 par.rate  = muroard_state_member(sa_rate);
 par.pchan = muroard_state_member(sa_channels);
 par.le    = SIO_LE_NATIVE;
 par.sig   = 1;

 if ( (muroard_state_member(driver_vp) = sio_open(dev, SIO_PLAY, 0)) == NULL )
  return -1;

 if ( sio_setpar(muroard_state_member(driver_vp), &par) == 0 ) {
  sio_close(muroard_state_member(driver_vp));
  return -1;
 }

 if ( sio_start(muroard_state_member(driver_vp)) == 0 ) {
  sio_close(muroard_state_member(driver_vp));
  return -1;
 }

 return 0;
}

int muroard_driver_free(void) {
 if ( muroard_state_member(driver_vp) != NULL ) {
  sio_stop(muroard_state_member(driver_vp));
  sio_close(muroard_state_member(driver_vp));
 }

 return 0;
}

ssize_t muroard_driver_write(int16_t * data, size_t len, enum muroard_blocking blocking) {
 if ( blocking == MUROARD_BLOCKING_NONE )
  return -1;

 sio_write(muroard_state_member(driver_vp), data, len);

 return 0;
}

//ll
