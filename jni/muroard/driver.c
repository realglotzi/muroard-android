//driver.c:

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

#ifndef MUROARD_FEATURE_DRIVER
#define MUROARD_FEATURE_DRIVER MUROARD_DRIVER_NONE
#endif

#if   MUROARD_FEATURE_DRIVER == MUROARD_DRIVER_NONE
/* use dummy driver from below */
#elif MUROARD_FEATURE_DRIVER == MUROARD_DRIVER_MUROAR
#include "driver_muroar.c"
#elif MUROARD_FEATURE_DRIVER == MUROARD_DRIVER_LIBAO
#include "driver_ao.c"
#elif MUROARD_FEATURE_DRIVER == MUROARD_DRIVER_OSS
#include "driver_oss.c"
#elif MUROARD_FEATURE_DRIVER == MUROARD_DRIVER_WMM
#include "driver_wmm.c"
#elif MUROARD_FEATURE_DRIVER == MUROARD_DRIVER_ESD
#include "driver_esd.c"
#elif MUROARD_FEATURE_DRIVER == MUROARD_DRIVER_SNDIO
#include "driver_sndio.c"
#elif MUROARD_FEATURE_DRIVER == MUROARD_DRIVER_TINYALSA
#include "driver_tinyalsa.c"
#else
#error Invalid driver setting. check MUROARD_FEATURE_DRIVER
#endif

#ifndef DRIVER_HAVE_INIT
int muroard_driver_init(const char * dev) {
 (void)dev;
 return 0;
}
#endif

#ifndef DRIVER_HAVE_FREE
int muroard_driver_free(void) {
#ifdef DRIVER_USE_SYSIO
 if ( muroard_state_member(driver_fh) != -1 )
  return close(muroard_state_member(driver_fh));
#endif
 return 0;
}
#endif

#ifndef DRIVER_HAVE_WRITE
ssize_t muroard_driver_write(int16_t * data, size_t len, enum muroard_blocking blocking) {
#ifdef DRIVER_USE_SYSIO
 ssize_t ret;
 size_t done = 0;

 if ( blocking != muroard_state_member(driver_old_blocking) ) {
  muroard_network_nonblock(muroard_state_member(driver_fh), blocking != MUROARD_BLOCKING_NONE);
  muroard_state_member(driver_old_blocking) = blocking;
 }

 while (len) {
  ret = write(muroard_state_member(driver_fh), data, len);

#ifndef __WIN32
  if ( blocking == MUROARD_BLOCKING_NONE && ret == -1 && errno == EAGAIN )
   ret = 0;
#endif

  if ( ret > 0 ) {
   done += ret;
   len  -= ret;
   data += ret;
  } if ( ret < 0 ) {
   return -1;
  } else { // ret == 0
   return done;
  }
 }
 return done;

#else
 (void)data;
 if ( blocking == MUROARD_BLOCKING_NONE )
  return len;
#ifndef __WIN32
 usleep(muroard_g_abuffer_size*1000000LLU/(long long unsigned)(muroard_g_sa_rate*muroard_g_sa_channels*2));
#else
 Sleep(muroard_g_abuffer_size*1000LLU/(long long unsigned)(muroard_g_sa_rate*muroard_g_sa_channels*2));
#endif
 return len;
#endif
}
#endif

//ll
