//driver_oss.c:

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

#if defined(__OpenBSD__) || defined(__NetBSD__)
#include <soundcard.h>
#else
#include <sys/soundcard.h>
#endif

#include <sys/ioctl.h> /* for ioctl() */

#include <fcntl.h> /* for open() */

#define DRIVER_HAVE_INIT
#define DRIVER_USE_SYSIO

#if defined(MUROARD_FEATURE_DRIVER_OSS_SETFRAGMENT) && defined(SNDCTL_DSP_SETFRAGMENT)
#define driver_set_fragments(x) driver_try_buf_setups((x))
static void driver_try_buf_setups(int fh) {
 unsigned int blocksizes[] = {11, 12, 13};
 unsigned int blocks[]     = {4, 5, 6, 3, 7, 2, 8};
 unsigned int bs, b;
 int tmp;

 for (bs = 0; bs < sizeof(blocksizes)/sizeof(int); bs++) {
  for (b = 0; b  < sizeof(blocks)    /sizeof(int); b++ ) {
   tmp = blocksizes[bs] | (blocks[b] << 16);
   if ( ioctl(fh, SNDCTL_DSP_SETFRAGMENT, &tmp) == 0 )
    return;
  }
 }
}
#else
#define driver_set_fragments(x) ((void)(x))
#endif

#define _err() close(muroard_state_member(driver_fh)); return -1

int muroard_driver_init(const char * dev) {
 int ctmp, tmp;

#ifdef MUROARD_DEFAULT_DRIVER_OSS_DEV
 if ( dev == NULL )
  dev = MUROARD_DEFAULT_DRIVER_OSS_DEV;
#endif

 muroard_state_member(driver_fh) = open(dev, O_WRONLY, 0644);

 if ( muroard_state_member(driver_fh) == -1 )
  return -1;

// channels:
#ifdef SNDCTL_DSP_CHANNELS
 tmp = muroard_state_member(sa_channels);

 if ( ioctl(muroard_state_member(driver_fh), SNDCTL_DSP_CHANNELS, &tmp) == -1 ) {
  _err();
 }

 if ( tmp != muroard_state_member(sa_channels) ) {
  _err();
 }
#else
 switch (muroard_state_member(sa_channels)) {
  case  1: tmp = 0; break;
  case  2: tmp = 1; break;
  default: _err();
 }

 if ( ioctl(muroard_state_member(driver_fh), SNDCTL_DSP_STEREO, &tmp) == -1 ) {
  _err();
 }
#endif

 ctmp = tmp = AFMT_S16_NE;

#ifdef SNDCTL_DSP_SETFMT
 if ( ioctl(muroard_state_member(driver_fh), SNDCTL_DSP_SETFMT, &tmp) == -1 ) {
#else
 if ( ioctl(muroard_state_member(driver_fh), SNDCTL_DSP_SAMPLESIZE, &tmp) == -1 ) {
#endif
  _err();
 }

 if ( tmp != ctmp ) {
  _err();
 }

 tmp = muroard_state_member(sa_rate);

 if ( ioctl(muroard_state_member(driver_fh), SNDCTL_DSP_SPEED, &tmp) == -1 ) {
  _err();
 }

 if ( tmp != muroard_state_member(sa_rate) ) {
  _err();
 }

 driver_set_fragments(muroard_state_member(driver_fh));

 return 0;
}

//ll
