//memmgr.h:

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

#ifndef _MUROARD_MEMMGR_H_
#define _MUROARD_MEMMGR_H_

#ifdef MUROARD_MEMMGR_NOUSE_MALLOC
int    muroard_memmgr_init(void);
void * muroard_malloc(size_t size);
void   muroard_free(void * ptr);
#else
#define muroard_memmgr_init()       (0)
#define muroard_malloc(size)        malloc((size))
#define muroard_free(ptr)           free((ptr))
#endif

#ifndef muroard_memzero
#define muroard_memzero(ptr,size)  memset((ptr), 0, (size))
#endif

#ifdef MUROARD_MEMMGR_LOCK_ALL
int    muroard_memmgr_lock_all(void);
#endif

#endif

//ll
