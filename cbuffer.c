
/**
 * Copyright (c) 2014, Willem-Hendrik Thiart
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <unistd.h>
#include "cbuffer.h"

#define fail() assert(0)

/** OSX needs some help here */
#ifndef MAP_ANONYMOUS
#  define MAP_ANONYMOUS MAP_ANON
#endif

static void __create_buffer_mirror(cbuf_t* cb)
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
    cb->data = mmap(NULL, cb->size << 1, PROT_NONE, MAP_ANONYMOUS | MAP_PRIVATE,
                    -1, 0);
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
    me->head = me->tail = 0;
    __create_buffer_mirror(me);
    return me;
}

void cbuf_free(cbuf_t *me)
{
    munmap(me->data, me->size << 1);
    free(me);
}

int cbuf_is_empty(const cbuf_t *me)
{
    return me->head == me->tail;
}

int cbuf_offer(cbuf_t *me, const unsigned char *data, const int size)
{
    /* prevent buffer from getting completely full or over commited */
    if (cbuf_unusedspace(me) <= size)
        return 0;

    int written = cbuf_unusedspace(me);
    written = size < written ? size : written;
    memcpy(me->data + me->tail, data, written);
    me->tail += written;
    if (me->size < me->tail)
        me->tail %= me->size;
    return written;
}

unsigned char *cbuf_peek(const cbuf_t *me)
{
    if (cbuf_is_empty(me))
        return NULL;

    return me->data + me->head;
}

unsigned char *cbuf_poll(cbuf_t *me, const unsigned int size)
{
    if (cbuf_is_empty(me))
        return NULL;

    void *end = me->data + me->head;
    me->head += size;
    return end;
}

int cbuf_size(const cbuf_t *me)
{
    return me->size;
}

int cbuf_usedspace(const cbuf_t *me)
{
    if (me->head <= me->tail)
        return me->tail - me->head;
    else
        return me->size - (me->head - me->tail);
}

int cbuf_unusedspace(const cbuf_t *me)
{
    return me->size - cbuf_usedspace(me);
}
