/*
 
Copyright (c) 2011, Willem-Hendrik Thiart
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * The names of its contributors may not be used to endorse or promote
      products derived from this software without specific prior written
      permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL WILLEM-HENDRIK THIART BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include "cbuffer.h"

#define fail() assert(0)


int cbuf_get_unused_size(const cbuf_t * cb)
{
    if (cb->end < cb->start)
    {
        return cb->start - cb->end;
    }
    else
    {
        return cb->size - (cb->start - cb->end);
    }
}

/**
 * creat new circular buffer.
 * @param order to the power of two equals size*/
cbuf_t *cbuf_new(const unsigned int order)
{
    cbuf_t *cb;

    cb = malloc(sizeof(cbuf_t));
    cb->size = 1UL << order;
//    cb->data = malloc(cb->size);

    char path[] = "/dev/shm/ring-cb-XXXXXX";

    int fd, status;

    void *address;

    fd = mkstemp(path);
    if (fd < 0)
        fail();

    status = unlink(path);
    if (status)
        fail();

    cb->size = 1UL << order;
    cb->start = 0;
    cb->end = 0;

    status = ftruncate(fd, cb->size);
    if (status)
        fail();

    cb->data = mmap(NULL, cb->size << 1, PROT_NONE,
                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (cb->data == MAP_FAILED)
        fail();

    address = mmap(cb->data, cb->size, PROT_READ | PROT_WRITE,
                   MAP_FIXED | MAP_SHARED, fd, 0);

    if (address != cb->data)
        fail();

    address = mmap(cb->data + cb->size, cb->size, PROT_READ | PROT_WRITE,
                   MAP_FIXED | MAP_SHARED, fd, 0);

    if (address != cb->data + cb->size)
        fail();

    status = close(fd);
    if (status)
        fail();

    return cb;
}

void cbuf_free(cbuf_t * cb)
{
    munmap(cb->data, cb->size << 1);
    free(cb);
}

int cbuf_is_empty(const cbuf_t * cb)
{
    return cb->start == cb->end;
}

/**
 * @return number of bytes offered
 * */
int cbuf_offer(cbuf_t * cb, const unsigned char *data, const int size)
{
    int written;

    written = cbuf_get_unused_size(cb);
    written = size < written ? size : written;
    memcpy(cb->data + cb->end, data, written);
    cb->end += written;
    cb->end %= cb->size;
    return written;
}

/**
 * Look at data.
 * Don't move cursor
 */
unsigned char *cbuf_peek(const cbuf_t * cb, const int size)
{
    void *end;

    if (cbuf_is_empty(cb))
        return NULL;

    return cb->data + cb->start;
}

/** 
 * Get pointer to data to read.
 * Move the cursor on
 *
 * @return pointer to data, null if we can't poll this much data
 */
unsigned char *cbuf_poll(cbuf_t * cb, const int size)
{
    void *end;

//    printf("%lx %d\n", cb->data, cb->start);
//    printf("%lx %d %lx\n", cb->data, cb->start, cb->data + cb->start);
    if (cbuf_is_empty(cb))
        return NULL;

    end = cb->data + cb->start;
    cb->start += size;
    return end;
}

int cbuf_get_size(const cbuf_t * cb)
{
    return cb->size;
}
