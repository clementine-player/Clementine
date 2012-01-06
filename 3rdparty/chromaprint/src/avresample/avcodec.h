/*
 * copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVCODEC_H
#define AVCODEC_H

/* Just a heavily bastardized version of the original file from
 * ffmpeg, just enough to get resample2.c to compile without
 * modification -- Lennart */

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

#include <sys/types.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

typedef void *AVClass;

#define av_mallocz(l) calloc(1, (l))
#define av_malloc(l) malloc(l)
#define av_realloc(p,l) realloc((p),(l))
#define av_free(p) free(p)

#ifdef _MSC_VER
#define CHROMAPRINT_C_INLINE __inline
#else
#define CHROMAPRINT_C_INLINE inline
#endif

static CHROMAPRINT_C_INLINE void av_freep(void *k) {
    void **p = (void **)k;

    if (p) {
        free(*p);
        *p = NULL;
    }
}

static CHROMAPRINT_C_INLINE int av_clip(int a, int amin, int amax)
{
    if (a < amin)      return amin;
    else if (a > amax) return amax;
    else               return a;
}

#define av_log(a,b,c)

#define FFABS(a) ((a) >= 0 ? (a) : (-(a)))
#define FFSIGN(a) ((a) > 0 ? 1 : -1)

#define FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define FFMIN(a,b) ((a) > (b) ? (b) : (a))

struct AVResampleContext;
struct AVResampleContext *av_resample_init(int out_rate, int in_rate, int filter_length, int log2_phase_count, int linear, double cutoff);
int av_resample(struct AVResampleContext *c, short *dst, short *src, int *consumed, int src_size, int dst_size, int update_ctx);
void av_resample_compensate(struct AVResampleContext *c, int sample_delta, int compensation_distance);
void av_resample_close(struct AVResampleContext *c);
void av_build_filter(int16_t *filter, double factor, int tap_count, int phase_count, int scale, int type);

/* error handling */
#if EDOM > 0
#define AVERROR(e) (-(e))   ///< Returns a negative error code from a POSIX error code, to return from library functions.
#define AVUNERROR(e) (-(e)) ///< Returns a POSIX error code from a library function error return value.
#else
/* Some platforms have E* and errno already negated. */
#define AVERROR(e) (e)
#define AVUNERROR(e) (e)
#endif

/*
 * crude lrintf for non-C99 systems.
 */
#ifndef HAVE_LRINTF
#define lrintf(x) ((long int)floor(x + 0.5))
#endif

#endif /* AVCODEC_H */
