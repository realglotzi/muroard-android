//cf.h:

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

#ifndef _MUROARD_CF_H_
#define _MUROARD_CF_H_

#ifdef MUROARD_FEATURE_CODECFILTER

#ifdef MUROARD_FEATURE_CODECFILTER_AU
ssize_t muroard_cf_au_read(int id, struct muroard_stream * stream, void * buf, size_t len);
#endif

#ifdef MUROARD_FEATURE_CODECFILTER_RIFF_WAVE
ssize_t muroard_cf_riff_wave_read(int id, struct muroard_stream * stream, void * buf, size_t len);
#endif

#ifdef MUROARD_FEATURE_CODECFILTER_OGG_VORBIS
ssize_t muroard_cf_ogg_vorbis_read (int id, struct muroard_stream * stream, void * buf, size_t len);
int     muroard_cf_ogg_vorbis_close(int id, struct muroard_stream * stream);
#endif

#endif

#endif

//ll
