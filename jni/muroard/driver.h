//driver.h:

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

#ifndef _MUROARD_DRIVER_H_
#define _MUROARD_DRIVER_H_

#define MUROARD_DRIVER_NONE        0
#define MUROARD_DRIVER_MUROAR      1
#define MUROARD_DRIVER_ESD         2
#define MUROARD_DRIVER_LIBAO       3
#define MUROARD_DRIVER_OSS         4
#define MUROARD_DRIVER_WMM         5
#define MUROARD_DRIVER_SNDIO       6
#define MUROARD_DRIVER_TINYALSA    7

int muroard_driver_init(const char * dev);
int muroard_driver_free(void);
ssize_t muroard_driver_write(int16_t * data, size_t len, enum muroard_blocking blocking);

#endif

//ll
