//driver_wmm.c:

/*
 *      Copyright (C) Philipp 'ph3-der-loewe' Schafft - 2009-2013
 *      Copyright (C) Benjamin Gerard                 - March 2007
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
#include <windows.h>
#include <mmsystem.h>

#define _debug 0

#define dbg _debug &&

#define DRIVER_HAVE_INIT
#define DRIVER_HAVE_FREE
#define DRIVER_HAVE_WRITE

struct driver_wmm {
  UINT  id;             /* device id                       */
  HWAVEOUT hwo;         /* waveout handler                 */
  WAVEOUTCAPS caps;     /* device caps                     */
  WAVEFORMATEX wavefmt; /* sample format                   */

  int opened;           /* device has been opened          */
  int prepared;         /* waveheaders have been prepared  */
  int blocks;           /* number of blocks (wave headers) */
  int splPerBlock;      /* sample per blocks.              */
  int msPerBlock;       /* millisecond per block (approx.) */

  void * bigbuffer;     /* Allocated buffer for waveheaders and sound data */
  struct {
   WAVEHDR wh;          /* waveheader                        */
   char *  data;        /* sample data ptr                   */
   int     idx;         /* index of this header              */
   int     count;       /* current byte count                */
   int     length;      /* size of data                      */
   int     sent;        /* set when header is sent to device */
  } * wh;          /* Pointer to waveheaders in bigbuffer             */
  BYTE * spl;           /* Pointer to sound data in bigbuffer              */

  int sent_blocks;      /* Number of waveheader sent (not ack).        */
  int full_blocks;      /* Number of waveheader full (ready to send).  */
  int widx;             /* Index to the block being currently filled.  */
  int ridx;             /* Index to the block being sent.              */
};



static int _alloc_wave_headers(struct driver_wmm * self) {
  int bytesPerBlock = self->wavefmt.nBlockAlign * self->splPerBlock;
  /*   int bytes = internal->blocks * (sizeof(WAVEHDR) + bytesPerBlock); */
  int bytes = self->blocks * (sizeof(*self->wh) + bytesPerBlock);
  int res;
  MMRESULT mmres;

  self->bigbuffer = malloc(bytes);

  if (self->bigbuffer != NULL) {
    int i;
    BYTE * b;

    muroard_memzero(self->bigbuffer, bytes);

    self->wh = self->bigbuffer;
    self->spl = (LPBYTE) (self->wh+self->blocks);
    for (i=0, b=self->spl; i<self->blocks; ++i, b+=bytesPerBlock) {
      self->wh[i].data = b;
      self->wh[i].wh.lpData = self->wh[i].data;
      self->wh[i].length = bytesPerBlock;
      self->wh[i].wh.dwBufferLength = self->wh[i].length;
      self->wh[i].wh.dwUser = (DWORD_PTR)self;
      mmres = waveOutPrepareHeader(self->hwo,
                                   &self->wh[i].wh,sizeof(WAVEHDR));
      if (MMSYSERR_NOERROR != mmres) {
        break;
      }
    }
    if (i<self->blocks) {
      while (--i >= 0) {
        waveOutUnprepareHeader(self->hwo,
                               &self->wh[i].wh,sizeof(WAVEHDR));
      }
      free(self->bigbuffer);
      self->wh        = 0;
      self->spl       = 0;
      self->bigbuffer = 0;
    } else {
      /* all ok ! */
    }
  }

  res = (self->bigbuffer != NULL);
  return res;
}

static int _get_free_block(struct driver_wmm * self);
static int _wait_wave_headers(struct driver_wmm * self, int wait_all) {
  int res = 1;

  while (self->sent_blocks > 0) {
    int n;
    _get_free_block(self);
    n = self->sent_blocks;
    if (n > 0) {
      unsigned int ms = (self->msPerBlock>>1)+1;
      if (wait_all) ms *= n;
      Sleep(ms);
    }
  }

  res &= !self->sent_blocks;
  return res;
}

static int _get_free_block(struct driver_wmm * self) {
  const int idx = self->widx;
  int ridx = self->ridx;

  while (self->wh[ridx].sent && !!(self->wh[ridx].wh.dwFlags & WHDR_DONE)) {
    /* block successfully sent to hardware, release it */
    /*debug("_ao_get_free_block: release block %d\n",ridx);*/
    self->wh[ridx].sent = 0;
    self->wh[ridx].wh.dwFlags &= ~WHDR_DONE;

    --self->full_blocks;
    if (self->full_blocks<0) {
      self->full_blocks = 0;
    }

    --self->sent_blocks;
    if (self->sent_blocks<0) {
      self->sent_blocks = 0;
    }
    if (++ridx >= self->blocks) ridx = 0;
  }
  self->ridx = ridx;

  return self->wh[idx].sent
    ? -1
    : idx;
}

static int _free_wave_headers(struct driver_wmm * self) {
  MMRESULT mmres;
  int res = 1;

  if (self->wh) {
    int i;

    /* Reset so we dont need to wait ... Just a satefy net
     * since _ao_wait_wave_headers() has been called once before.
     */
    mmres = waveOutReset(self->hwo);
    /* Wait again to be sure reseted waveheaders has been released. */
    _wait_wave_headers(self, 0);

    for (i=self->blocks; --i>=0; ) {
      mmres = waveOutUnprepareHeader(self->hwo,
                                     &self->wh[i].wh,sizeof(WAVEHDR));

      res &= mmres == MMSYSERR_NOERROR;
    }
    self->wh  = 0;
    self->spl = 0;
  }

  return res;
}

/* Send a block to audio hardware */
static int _send_block(struct driver_wmm * self, const int idx) {
  MMRESULT mmres;

  dbg printf("_send_block(*) = ?\n");

  /* Satanity checks */
  if (self->wh[idx].sent) {
    return 0;
  }
  if (!!(self->wh[idx].wh.dwFlags & WHDR_DONE)) {
    return 0;
  }

  /* count <= 0, just pretend it's been sent */
  if (self->wh[idx].count <= 0) {
    self->wh[idx].sent = 2; /* set with 2 so we can track these special cases */
    self->wh[idx].wh.dwFlags |= WHDR_DONE;
    ++self->sent_blocks;
    return 1;
  }

  self->wh[idx].wh.dwBufferLength = self->wh[idx].count;
  self->wh[idx].count = 0;
  mmres = waveOutWrite(self->hwo,
                       &self->wh[idx].wh, sizeof(WAVEHDR));
  self->wh[idx].sent = (mmres == MMSYSERR_NOERROR);
  /*&& !(internal->wh[idx].wh.dwFlags & WHDR_DONE);*/
  self->sent_blocks += self->wh[idx].sent;
  return mmres == MMSYSERR_NOERROR;
}


int muroard_driver_init(const char * dev) {
 struct driver_wmm * self = muroard_malloc(sizeof(struct driver_wmm));
 MMRESULT mmres;

 muroard_state_member(driver_vp) = self;

 dbg printf("muroard_driver_init(dev='%s') = ?\n", dev);
 dbg printf("muroard_driver_init(dev='%s'): rate=%i, channels=%i, bits=16\n",
                 dev, muroard_state_member(sa_rate), muroard_state_member(sa_channels));

 muroard_memzero(self, sizeof(*self));

 // WMM Setup:
 self->wavefmt.wFormatTag      = WAVE_FORMAT_PCM;
 self->wavefmt.nChannels       = muroard_state_member(sa_channels);
 self->wavefmt.wBitsPerSample  = 16;
 self->wavefmt.nSamplesPerSec  = muroard_state_member(sa_rate);
 self->wavefmt.nBlockAlign     = (self->wavefmt.wBitsPerSample>>3)*self->wavefmt.nChannels;
 self->wavefmt.nAvgBytesPerSec = self->wavefmt.nSamplesPerSec*self->wavefmt.nBlockAlign;
 self->wavefmt.cbSize          = 0;

 /* $$$ later this should be optionnal parms */
  self->id          = WAVE_MAPPER;
  self->blocks      = MUROARD_FEATURE_DRIVER_WMM_FRAGMENTS;
  //self->splPerBlock = 512;
#ifdef MUROARD_FIXED_ABUFFER_SIZE
  self->splPerBlock = MUROARD_FIXED_ABUFFER_SIZE / self->wavefmt.nBlockAlign;
#else
  self->splPerBlock = MUROARD_DEFAULT_FRAMES_PER_CYCLE;
#endif
  self->msPerBlock  =
    (self->splPerBlock * 1000 + muroard_state_member(sa_rate) - 1) / muroard_state_member(sa_rate);

  mmres =
    waveOutOpen(&(self->hwo),
                self->id,
                &(self->wavefmt),
                (DWORD_PTR)0/* waveOutProc */,
                (DWORD_PTR)self,
                CALLBACK_NULL/* |WAVE_FORMAT_DIRECT */|WAVE_ALLOWSYNC);
 if ( mmres != MMSYSERR_NOERROR ) {
  muroard_driver_free();
  dbg printf("muroard_driver_init(dev='%s') = -1 //mmres=%i, WAVERR_BADFORMAT=%i\n", dev, (int)mmres, WAVERR_BADFORMAT);
  return -1;
 }

 // FIXME: do error checking
 waveOutGetID(self->hwo, &(self->id));

 _alloc_wave_headers(self);

 self->opened = 1;

 dbg printf("muroard_driver_init(dev='%s') = 0\n", dev);
 return 0;
}

int muroard_driver_free(void) {
 struct driver_wmm * self = muroard_state_member(driver_vp);

 dbg printf("muroard_driver_free(void) = ?\n");

 if ( self->opened ) {
  _wait_wave_headers(self, 1);
  _free_wave_headers(self);
  waveOutClose(self->hwo);
 }

 muroard_free(self);
 dbg printf("muroard_driver_free(void) = 0\n");
 return 0;
}

ssize_t muroard_driver_write(int16_t * data, size_t len, enum muroard_blocking blocking) {
 struct driver_wmm * self = muroard_state_member(driver_vp);
 ssize_t ret_ok = len;
 int ret = 1;

 if ( blocking == MUROARD_BLOCKING_NONE )
  return -1;

 dbg printf("muroard_driver_write(data=%p) = ?\n", data);

 if ( ! self->opened )
  return -1;

 dbg printf("muroard_driver_write(data=%p) = ?\n", data);

  dbg printf("-----------------\n");

  while(ret && len > 0) {
    int n;
    const int idx = _get_free_block(self);

    dbg printf("muroard_driver_write(data=%p): XXX\n", data);

    if (idx == -1) {
      dbg printf("sleep of %ims\n", self->msPerBlock);
      Sleep(self->msPerBlock);
      continue;
    } else {
      dbg printf("continue...\n");
    }

    /* Get free bytes in the block */
    n = self->wh[idx].wh.dwBufferLength
      - self->wh[idx].count;

    /*     debug("free in block %d : %d/%d\n", */
    /*    idx,n,internal->wh[idx].dwBufferLength); */

    /* Get amount to copy */
    if (n > (int)len) {
      n = len;
    }
    /*     debug("copy = %d\n",n); */



    /* Do copy */
    CopyMemory((char*)self->wh[idx].wh.lpData
               + self->wh[idx].count,
               data, n);

    /* Updates pointers and counters */
    data += n;
    len -= n;
    /*     debug("rem = %d\n",num_bytes); */
    self->wh[idx].count += n;

    /* Is this block full ? */
    if (self->wh[idx].count
        == self->wh[idx].wh.dwBufferLength) {
      ++self->full_blocks;
      /*       debug("blocks %d full, total:%d\n",internal->widx,internal->full_blocks); */
      if (++self->widx == self->blocks) {
        self->widx = 0;
      }
      ret = _send_block(self, idx);
    }
  }

 dbg printf("muroard_driver_write(data=%p) = %i\n", data, (int)(ret > -1 ? ret_ok : -1));

  return ret > -1 ? ret_ok : -1;
}

//ll
