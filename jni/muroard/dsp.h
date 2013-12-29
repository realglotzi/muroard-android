//dsp.h:

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

#ifndef _MUROARD_DSP_H_
#define _MUROARD_DSP_H_

// macros to test if we need to swap:
#ifdef MUROARD_FEATURE_BYTE_SWAP
#if MUROAR_CODEC_PCM_S == MUROAR_CODEC_PCM_S_BE
#define muroard_dsp_need_swap_s(c) ((c) == MUROAR_CODEC_PCM_S_BE ? 0 : 1)
#define muroard_dsp_need_swap_u(c) ((c) == MUROAR_CODEC_PCM_U_BE ? 0 : 1)
#else
#define muroard_dsp_need_swap_s(c) ((c) == MUROAR_CODEC_PCM_S_BE ? 1 : 0)
#define muroard_dsp_need_swap_u(c) ((c) == MUROAR_CODEC_PCM_U_BE ? 1 : 0)
#endif
#endif

int muroard_dsp_mixer(int16_t * out, int16_t ** in, size_t num);

#ifdef MUROARD_FEATURE_UPMIX
void muroard_dsp_upmix(void * data, size_t samples);
#endif
#ifdef MUROARD_FEATURE_DOWNMIX
void muroard_dsp_downmix(void * data, size_t samples);
#endif
#ifdef MUROARD_FEATURE_UPSCALE
void muroard_dsp_upscale(void * data, size_t samples);
#endif
#ifdef MUROARD_FEATURE_DOWNSCALE
void muroard_dsp_downscale(void * data, size_t samples);
#endif

#ifdef MUROARD_FEATURE_BYTE_SWAP
void muroard_dsp_swap16(int16_t * data, size_t samples);
#endif

#ifdef MUROARD_FEATURE_VOLUME_CONTROL
void muroard_dsp_volume(int16_t * data, size_t samples, uint16_t volume);
#endif

#endif

//ll
