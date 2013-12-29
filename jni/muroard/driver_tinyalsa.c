//driver_tinyalsa.c:

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
#include <tinyalsa/asoundlib.h>

#define DRIVER_HAVE_INIT
#define DRIVER_HAVE_FREE
#define DRIVER_HAVE_WRITE

// this driver seems to only work on little endian systems...

int muroard_driver_init(const char * dev) {
 struct pcm_config config;
 unsigned int period_size = muroard_state_member(abuffer_size);
 unsigned int period_count = 4;
 unsigned int card = 0;
 unsigned int device = 0;

 config.channels = muroard_state_member(sa_channels);
 config.rate = muroard_state_member(sa_rate);
 config.format = PCM_FORMAT_S16_LE;
 config.start_threshold = 0;
 config.stop_threshold = 0;
 config.silence_threshold = 0;
 config.period_size = period_size;
 config.period_count = period_count;


 muroard_state_member(driver_vp) = pcm_open(card, device, PCM_OUT, &config);
 if (!muroard_state_member(driver_vp) || !pcm_is_ready(muroard_state_member(driver_vp)))
  return -1;

 return 0;
}

int muroard_driver_free(void) {
 if ( muroard_state_member(driver_vp) != NULL ) {
  pcm_close(muroard_state_member(driver_vp));
 }

 return 0;
}

ssize_t muroard_driver_write(int16_t * data, size_t len, enum muroard_blocking blocking) {
 if ( blocking == MUROARD_BLOCKING_NONE )
  return -1;

 pcm_write(muroard_state_member(driver_vp), data, len);

 return 0;
}

//ll
