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
    double t1, t2, t3, t4, t5, t6, t7, tx;

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

    t5 = gettime();
    for (i = 0; i < COUNT; i++)
    {
        aligned_block_fill(dstbuf, srcbuf, size);
    }
    t5 = gettime() - t5;

    t6 = gettime();
    for (i = 0; i < COUNT; i++)
    {
        memcpy(dstbuf, srcbuf, size);
    }
    t6 = gettime() - t6;

    t7 = gettime();
    for (i = 0; i < COUNT; i++)
    {
        memset(dstbuf, ((char *)srcbuf)[0], size);
    }
    t7 = gettime() - t7;

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
    printf("%swrite (fill):                   %.3f MB/s\n", indent_prefix,
        (double)size * COUNT / t5 / 1000000.);
    printf("%s---\n", indent_prefix);
    printf("%sstdandard memcpy:               %.3f MB/s\n", indent_prefix,
        (double)size * COUNT / t6 / 1000000.);
    printf("%sstdandard memset:               %.3f MB/s\n", indent_prefix,
        (double)size * COUNT / t7 / 1000000.);
}

static void __attribute__((noinline)) random_test(char *zerobuffer,
                                                  int count, int nbits)
{
    uint32_t seed = 0;
    uintptr_t addrmask = (1 << nbits) - 1;
    uint32_t v;
    static volatile uint32_t dummy;

    #define RANDOM_MEM_ACCESS()                 \
        seed = seed * 1103515245 + 12345;       \
        v = (seed >> 16) & 0xFF;                \
        seed = seed * 1103515245 + 12345;       \
        v |= ((seed >> 16) & 0xFF) << 8;        \
        seed = seed * 1103515245 + 12345;       \
        v |= ((seed >> 16) & 0x7FFF) << 16;     \
        seed |= zerobuffer[v & addrmask];

    while (count > 16) {
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        RANDOM_MEM_ACCESS();
        count -= 16;
    }
    while (count-- > 0) {
        RANDOM_MEM_ACCESS();
    }
    dummy = seed;
}

void latency_bench(int size, int count)
{
    double t, t_before, t_after, t_noaccess;
    int nbits;
    char *buffer;
    if (posix_memalign((void **)&buffer, 4096, size) != 0)
        return;
    memset(buffer, 0, size);

    t_before = gettime();
    random_test(buffer, count, 1);
    t_after = gettime();
    t_noaccess = t_after - t_before;

    printf("block size : random access time\n");
    for (nbits = 1; (1 << nbits) <= size; nbits++)
    {
        t_before = gettime();
        random_test(buffer, count, nbits);
        t_after = gettime();
        t = t_after - t_before - t_noaccess;
        if (t < 0) t = 0;
        printf("%10d : %-3.1f ns\n", (1 << nbits), t * 1000000000. / count);
    }
    free(buffer);
}

int main(void)
{
    int dummy;
    char *srcbuf, *dstbuf, *tmpbuf;
    void *poolbuf = alloc_four_nonaliased_buffers(&srcbuf, SIZE, &dstbuf, SIZE,
                                                  &tmpbuf, BLOCKSIZE, NULL, 0);
    printf("\n");
    printf("===================================================\n");
    printf("== Memory bandwidth tests (non-aliased buffers) ===\n");
    printf("===================================================\n\n");
    bandwidth_bench(dstbuf, srcbuf, tmpbuf, SIZE, BLOCKSIZE, "    ");
    free(poolbuf);

    dummy = posix_memalign((void **)&srcbuf, 128, SIZE);
    dummy = posix_memalign((void **)&dstbuf, 128, SIZE);
    dummy = posix_memalign((void **)&tmpbuf, 128, BLOCKSIZE);
    memset(srcbuf, 0xCC, SIZE);
    memset(dstbuf, 0xCC, SIZE);
    memset(tmpbuf, 0xCC, BLOCKSIZE);
    printf("\n");
    printf("===================================================================\n");
    printf("== Memory bandwidth tests (normal, potentially aliased buffers) ===\n");
    printf("== WARNING: the results below are likely bogus, especially if   ===\n");
    printf("==          obtained on a freshly rebooted system               ===\n");
    printf("===================================================================\n\n");
    bandwidth_bench(dstbuf, srcbuf, tmpbuf, SIZE, BLOCKSIZE, "    ");
    free(srcbuf);
    free(dstbuf);
    free(tmpbuf);

    printf("\n");
    printf("==========================\n");
    printf("== Memory latency test ===\n");
    printf("==========================\n\n");

    latency_bench(SIZE * 4, 10000000);

    return 0;
}
