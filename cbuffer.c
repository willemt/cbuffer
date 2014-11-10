/*

   Copyright(c) 2011, Willem - Hendrik Thiart
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met :
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and / or other materials provided with the distribution.
 * The names of its contributors may not be used to endorse or promote
   products derived from this software without specific prior written
   permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED.IN NO EVENT SHALL WILLEM - HENDRIK THIART BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include "stdio.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include "cbuffer.h"

#define fail() assert(0)

int cbuf_unusedspace(const cbuf_t *me)
{
    if (me->end < me->start)
        return me->start - me->end;
    else
        return me->size - (me->start - me->end);
}

static void __init_cbuf_mmap(cbuf_t* cb)
{
    char path[] = "/tmp/cb-XXXXXX";
    int fd, status;
    void *address;

    fd = mkstemp(path);
    if (fd < 0)
        fail();

    status = unlink(path);
    if (status)
        fail();

    status = ftruncate(fd, cb->size);
    if (status)
        fail();

    /* create the array of data */
    cb->data = mmap(NULL, cb->size << 1, PROT_NONE, MAP_ANON | MAP_PRIVATE, -1,
                    0);
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
}

cbuf_t *cbuf_new(const unsigned int order)
{
    cbuf_t *me = malloc(sizeof(cbuf_t));
    me->size = 1UL << order;
    me->start = 0;
    me->end = 0;
    __init_cbuf_mmap(me);
    return me;
}

void cbuf_free(cbuf_t *me)
{
    munmap(me->data, me->size << 1);
    free(me);
}

int cbuf_is_empty(const cbuf_t *me)
{
    return me->start == me->end;
}

int cbuf_offer(cbuf_t *me, const unsigned char *data, const int size)
{
    int written = cbuf_unusedspace(me);
    written = size < written ? size : written;
    memcpy(me->data + me->end, data, written);
    me->end += written;
    me->end %= me->size;
    return written;
}

unsigned char *cbuf_peek(const cbuf_t *me)
{
    if (cbuf_is_empty(me))
        return NULL;

    return me->data + me->start;
}

unsigned char *cbuf_poll(cbuf_t *me, const unsigned int size)
{
    if (cbuf_is_empty(me))
        return NULL;

    void *end = me->data + me->start;
    me->start += size;
    return end;
}

int cbuf_size(const cbuf_t *me)
{
    return me->size;
}

int cbuf_usedspace(const cbuf_t *me)
{
    return me->end - me->start;
}
