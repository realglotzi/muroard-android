//dsp.c:

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

int muroard_dsp_mixer(int16_t * out, int16_t ** in, size_t num) {
 size_t i;
 size_t n;
 register int32_t s;

 switch (num) {
#ifdef MUROARD_FEATURE_OPTIMIZE
  case 0:
    muroard_memzero(out, muroard_state_member(abuffer_size));
   break;
/* This case can not happen as the waveform code will use zero-copy.
  case 1:
    memcpy(out, in[0], muroard_state_member(abuffer_size));
   break;
*/
  case 2:
    for (i  = 0; i < muroard_state_member(abuffer_size)/2; i++) {
     s      = 0;
     s     += in[0][i];
     s     += in[1][i];
     out[i] = s;
    }
   break;
  case 3:
    for (i  = 0; i < muroard_state_member(abuffer_size)/2; i++) {
     s      = 0;
     s     += in[0][i];
     s     += in[1][i];
     s     += in[2][i];
     out[i] = s;
    }
   break;
#endif
  default:
    for (i  = 0; i < muroard_state_member(abuffer_size)/2; i++) {
     s      = 0;
     for (n = 0; n < num; n++)
      s    += in[n][i];
     out[i] = s;
    }
   break;
 }

 return 0;
}

#ifdef MUROARD_FEATURE_UPMIX
void muroard_dsp_upmix(void * data, size_t samples) {
 int16_t * ds = data;
 int16_t * dd = data;
 ssize_t   i, c;
 register int32_t s;

 samples--;

#ifdef MUROARD_FEATURE_OPTIMIZE
 switch (muroard_state_member(sa_channels)) {
  case 1:
    return;
   break;
  case 2:
    for (i = samples*2 /* stereo */; i >= 0; i -= 2) {
     s = ds[i/2];
     dd[i+0] = s;
     dd[i+1] = s;
    }
   break;
  case 3:
    for (i = samples*3 /* 3 channel */; i >= 0; i -= 3) {
     s = ds[i/3];
     dd[i+0] = s;
     dd[i+1] = s;
     dd[i+2] = s;
    }
   break;
  case 4:
    for (i = samples*4 /* quad */; i >= 0; i -= 4) {
     s = ds[i/4];
     dd[i+0] = s;
     dd[i+1] = s;
     dd[i+2] = s;
     dd[i+3] = s;
    }
   break;
  default:
#endif
    for (i = samples; i >= 0; i--) {
     s = ds[i];
     for (c = 0; c < muroard_state_member(sa_channels); c++) {
      dd[i*muroard_state_member(sa_channels)+c] = s;
     }
    }
#ifdef MUROARD_FEATURE_OPTIMIZE
   break;
 }
#endif
}
#endif
#ifdef MUROARD_FEATURE_DOWNMIX
void muroard_dsp_downmix(void * data, size_t samples) {
 int16_t * ds = data;
 int16_t * dd = data;
 ssize_t   c;
 size_t    i;
 register int32_t s;

#ifdef MUROARD_FEATURE_OPTIMIZE
 switch (muroard_state_member(sa_channels)) {
  case 1:
    return;
   break;
  case 2:
    for (i = 0; i < samples*2; i += 2) {
     s  = ds[i + 0];
     s += ds[i + 1];
     dd[i/2] = s / 2;
    }
   break;
  case 3:
    for (i = 0; i < samples*3; i += 3) {
     s  = ds[i + 0];
     s += ds[i + 1];
     s += ds[i + 2];
     dd[i/3] = s / 3;
    }
   break;
  case 4:
    for (i = 0; i < samples*4; i += 4) {
     s  = ds[i + 0];
     s += ds[i + 1];
     s += ds[i + 2];
     s += ds[i + 3];
     dd[i/4] = s / 4;
    }
   break;
  default:
#endif
    for (i = 0; i < samples; i++) {
     s = 0;
     for (c = 0; c < muroard_state_member(sa_channels); c++) {
      s += ds[i*muroard_state_member(sa_channels) + c];
     }
     dd[i] = s / muroard_state_member(sa_channels);
    }
#ifdef MUROARD_FEATURE_OPTIMIZE
 }
#endif
}
#endif
#ifdef MUROARD_FEATURE_UPSCALE
void muroard_dsp_upscale(void * data, size_t samples) {
 char    * ds = data;
 int16_t * dd = data;
 ssize_t    i;

 samples--;

 for (i = samples; i >= 0; i--)
  dd[i] = ds[i] << 8;
}
#endif
#ifdef MUROARD_FEATURE_DOWNSCALE
void muroard_dsp_downscale(void * data, size_t samples) {
 int16_t * ds = data;
 char    * dd = data;
 size_t    i;

 for (i = 0; i < samples; i++)
  dd[i] = ds[i] >> 8;
}
#endif

#ifdef MUROARD_FEATURE_BYTE_SWAP
void muroard_dsp_swap16(int16_t * data, size_t samples) {
 register size_t i;

 for (i = 0; i < samples; i++) {
  data[i] = ((data[i] << 8) & 0xFF00) | ((data[i] >> 8) & 0x00FF);
 }
}
#endif

#ifdef MUROARD_FEATURE_VOLUME_CONTROL
void muroard_dsp_volume(int16_t * data, size_t samples, uint16_t volume) {
 register int_least32_t s;
 register size_t i;

 for (i = 0; i < samples; i++) {
  s  = data[i] * volume;
  s /= 65535;
  data[i] = s;
 }
}
#endif

//ll
