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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "util.h"

#define SIZE             (16 * 1024 * 1024)
#define BLOCKSIZE        2048
#define COUNT            64

void bandwidth_bench(void *dstbuf, void *srcbuf, void *tmpbuf, int size,
                     int blocksize, char *indent_prefix)
{
    int i, j;
    double t1, t2, t3, t4, tx;

    aligned_block_copy_backwards_noprefetch(dstbuf, srcbuf, size);
    tx = gettime();
    for (i = 0; i < COUNT; i++)
    {
        aligned_block_copy_backwards_noprefetch(dstbuf, srcbuf, size);
    }
    tx = gettime() - tx;

    aligned_block_copy_noprefetch(dstbuf, srcbuf, size);
    t1 = gettime();
    for (i = 0; i < COUNT; i++)
    {
        aligned_block_copy_noprefetch(dstbuf, srcbuf, size);
    }
    t1 = gettime() - t1;

    aligned_block_copy(dstbuf, srcbuf, size);
    t2 = gettime();
    for (i = 0; i < COUNT; i++)
    {
        aligned_block_copy(dstbuf, srcbuf, size);
    }
    t2 = gettime() - t2;

    t3 = gettime();
    for (i = 0; i < COUNT; i++)
    {
        for (j = 0; j < size; j += blocksize)
        {
            aligned_block_copy_noprefetch(tmpbuf, srcbuf + j, blocksize);
            aligned_block_copy_noprefetch(dstbuf + j, tmpbuf, blocksize);
        }
    }
    t3 = gettime() - t3;

    t4 = gettime();
    for (i = 0; i < COUNT; i++)
    {
        for (j = 0; j < size; j += blocksize)
        {
            aligned_block_copy(tmpbuf, srcbuf + j, blocksize);
            aligned_block_copy(dstbuf + j, tmpbuf, blocksize);
        }
    }
    t4 = gettime() - t4;

    printf("%sdirect copy backwards:          %.3f MB/s\n", indent_prefix,
        (double)size * COUNT / tx / 1000000.);
    printf("%s---\n", indent_prefix);
    printf("%sdirect copy:                    %.3f MB/s\n", indent_prefix,
        (double)size * COUNT / t1 / 1000000.);
    printf("%sdirect copy prefetched:         %.3f MB/s\n", indent_prefix,
        (double)size * COUNT / t2 / 1000000.);
    printf("%scopy via tmp buffer:            %.3f MB/s\n", indent_prefix,
        (double)size * COUNT / t3 / 1000000.);
    printf("%scopy via tmp buffer prefetched: %.3f MB/s\n", indent_prefix,
        (double)size * COUNT / t4 / 1000000.);
    printf("%s---\n", indent_prefix);
    printf("%stmp buffer use slowdown ratio:  %.2fx\n", indent_prefix,
        fmin(t3, t4) / fmin(t1, t2));
}

int main(void)
{
    int dummy;
    char *srcbuf, *dstbuf, *tmpbuf;
    void *poolbuf = alloc_four_nonaliased_buffers(&srcbuf, SIZE, &dstbuf, SIZE,
                                                  &tmpbuf, BLOCKSIZE, NULL, 0);
    printf("\n== Memory bandwidth tests (non-aliased buffers) ===\n\n");
    bandwidth_bench(dstbuf, srcbuf, tmpbuf, SIZE, BLOCKSIZE, "    ");
    free(poolbuf);

    dummy = posix_memalign((void **)&srcbuf, 128, SIZE);
    dummy = posix_memalign((void **)&dstbuf, 128, SIZE);
    dummy = posix_memalign((void **)&tmpbuf, 128, BLOCKSIZE);
    memset(srcbuf, 0xCC, SIZE);
    memset(dstbuf, 0xCC, SIZE);
    memset(tmpbuf, 0xCC, BLOCKSIZE);
    printf("\n== Memory bandwidth tests (normal, potentially aliased buffers) ===\n\n");
    bandwidth_bench(dstbuf, srcbuf, tmpbuf, SIZE, BLOCKSIZE, "    ");
    free(srcbuf);
    free(dstbuf);
    free(tmpbuf);

    return 0;
}
