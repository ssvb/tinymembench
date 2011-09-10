/*
 * Copyright © 2011 Siarhei Siamashka <siarhei.siamashka@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "asm-opt.h"

#if defined(__linux__) || defined(ANDROID) || defined(__ANDROID__)

#define SOMEWHAT_SANE_PROC_CPUINFO_SIZE_LIMIT (1024 * 1024)

#if defined(__i386__) || defined(__amd64__)
#define FEATURES_ID "flags"
#elif defined(__arm__)
#define FEATURES_ID "Features"
#elif defined(__mips__)
#define FEATURES_ID "cpu model"
#else
#define FEATURES_ID "?"
#endif

static int check_feature (char *buffer, const char *feature)
{
  char *p;
  if (*feature == 0)
    return 0;
  if (strncmp(buffer, FEATURES_ID, strlen(FEATURES_ID)) != 0)
    return 0;
  buffer += strlen(FEATURES_ID);
  while (isspace(*buffer))
    buffer++;

  /* Check if 'feature' is present in the buffer as a separate word */
  while ((p = strstr(buffer, feature))) {
    if (p > buffer && !isspace(*(p - 1))) {
      buffer++;
      continue;
    }
    p += strlen(feature);
    if (*p != 0 && !isspace(*p)) {
      buffer++;
      continue;
    }
    return 1;
  }
  return 0;
}

static int parse_proc_cpuinfo(int bufsize, const char *feature)
{
  char *buffer = (char *)malloc(bufsize);
  FILE *fd;
  int feature_support = 0;

  if (!buffer)
    return 0;

  fd = fopen("/proc/cpuinfo", "r");
  if (fd) {
    while (fgets(buffer, bufsize, fd)) {
      if (!strchr(buffer, '\n') && !feof(fd)) {
        /* "impossible" happened - insufficient size of the buffer! */
        fclose(fd);
        free(buffer);
        return -1;
      }
      if (check_feature(buffer, feature))
        feature_support = 1;
    }
    fclose(fd);
  }
  free(buffer);
  return feature_support;
}

#define SOMEWHAT_SANE_PROC_CPUINFO_SIZE_LIMIT (1024 * 1024)

int check_cpu_feature(const char *feature)
{
  int bufsize = 1024;
  int result;
  while ((result = parse_proc_cpuinfo(bufsize, feature)) == -1)
  {
    bufsize *= 2;
    if (bufsize > SOMEWHAT_SANE_PROC_CPUINFO_SIZE_LIMIT)
      return 0;
  }
  return result;
}

#else

int check_cpu_feature(const char *feature)
{
    return 0;
}

#endif

static bench_info empty[] = { { NULL, 0, NULL } };

#if defined(__i386__) || defined(__amd64__)

#include "x86-sse2.h"

static bench_info x86_sse2[] =
{
    { "SSE2 copy", 0, aligned_block_copy_sse2 },
    { "SSE2 nontemporal copy", 0, aligned_block_copy_nt_sse2 },
    { "SSE2 copy prefetched (32 bytes step)", 0, aligned_block_copy_pf32_sse2 },
    { "SSE2 copy prefetched (64 bytes step)", 0, aligned_block_copy_pf64_sse2 },
    { "SSE2 nontemporal copy prefetched (32 bytes step)", 0, aligned_block_copy_nt_pf32_sse2 },
    { "SSE2 nontemporal copy prefetched (64 bytes step)", 0, aligned_block_copy_nt_pf64_sse2 },
    { "SSE2 copy via tmp buffer", 1, aligned_block_copy_sse2 },
    { "SSE2 copy via tmp buffer prefetched (32 bytes step)", 1, aligned_block_copy_pf32_sse2 },
    { "SSE2 copy via tmp buffer prefetched (64 bytes step)", 1, aligned_block_copy_pf64_sse2 },
    { "SSE2 fill", 0, aligned_block_fill_sse2 },
    { "SSE2 nontemporal fill", 0, aligned_block_fill_nt_sse2 },
    { NULL, 0, NULL }
};

bench_info *get_asm_benchmarks(void)
{
    if (check_cpu_feature("sse2"))
        return x86_sse2;
    else
        return empty;
}

#elif defined(__arm__)

#include "arm-neon.h"

static bench_info arm_neon[] =
{
    { "NEON copy", 0, aligned_block_copy_neon },
    { "NEON copy prefetched (32 bytes step)", 0, aligned_block_copy_pf32_neon },
    { "NEON copy prefetched (64 bytes step)", 0, aligned_block_copy_pf64_neon },
    { "NEON copy backwards", 0, aligned_block_copy_backwards_neon },
    { "NEON copy backwards prefetched (32 bytes step)", 0, aligned_block_copy_backwards_pf32_neon },
    { "NEON copy backwards prefetched (64 bytes step)", 0, aligned_block_copy_backwards_pf64_neon },
    { "NEON copy via tmp buffer", 1, aligned_block_copy_neon },
    { "NEON copy via tmp buffer prefetched (32 bytes step)", 1, aligned_block_copy_pf32_neon },
    { "NEON copy via tmp buffer prefetched (64 bytes step)", 1, aligned_block_copy_pf64_neon },
    { "NEON fill", 0, aligned_block_fill_neon },
    { NULL, 0, NULL }
};

bench_info *get_asm_benchmarks(void)
{
    if (check_cpu_feature("neon"))
        return arm_neon;
    else
        return empty;
}

#else

bench_info *get_asm_benchmarks(void)
{
    return empty;
}

#endif
