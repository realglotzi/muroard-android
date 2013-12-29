//memmgr.c:

/*
 *      Copyright (C) Philipp 'ph3-der-loewe' Schafft - 2011-2013
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

#ifdef MUROARD_MEMMGR_LOCK_ALL
#ifndef __WIN32
#include <sys/mman.h>
#endif
#endif

#ifdef MUROARD_MEMMGR_NOUSE_MALLOC
#define POOLSIZE (1+MUROARD_MAX_STREAMS)
static char _memory_pool[POOLSIZE][MUROARD_FIXED_ABUFFER_SIZE];
static char _memory_pool_map[POOLSIZE];

int    muroard_memmgr_init(void) {
 muroard_memzero(_memory_pool, sizeof(_memory_pool));
 muroard_memzero(_memory_pool_map, sizeof(_memory_pool_map));

 return 0;
}

void * muroard_malloc(size_t size) {
 int i;

 if ( size != MUROARD_FIXED_ABUFFER_SIZE )
  return NULL;

 for (i = 0; i < POOLSIZE; i++) {
  if ( _memory_pool_map[i] == 0 ) {
   _memory_pool_map[i] = 1;
   return _memory_pool[i];
  }
 }

 return NULL;
}

void   muroard_free(void * ptr) {
 int i;

 for (i = 0; i < POOLSIZE; i++) {
  if ( _memory_pool[i] == ptr ) {
   _memory_pool_map[i] = 0;
   return;
  }
 }
}

#endif

#ifdef MUROARD_MEMMGR_LOCK_ALL
int    muroard_memmgr_lock_all(void) {
#ifdef __WIN32
#warning MUROARD_MEMMGR_LOCK_ALL but not supported on win32.
 return -1;
#else
 if ( mlockall(MUROARD_MEMMGR_LOCK_ALL_FLAGS) == -1 )
  return -1;
#endif

 return 0;
}
#endif

//ll
