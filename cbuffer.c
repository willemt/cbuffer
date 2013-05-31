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

#include "stdio.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#if WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#endif
#include <unistd.h>
#include "cbuffer.h"

#define fail() assert(0)

typedef struct
{
    unsigned long int size;
    int start, end;
    void *data;
#if WIN32
    HANDLE hMapFile;
#endif
} cbuf_t;

int cbuf_get_unused_size(const void * cb)
{
    const cbuf_t *me = cb;

    if (me->end < me->start)
    {
        return me->start - me->end;
    }
    else
    {
        return me->size - (me->start - me->end);
    }
}

#if WIN32

static void __init_cbuf_win32(cbuf_t* cb)
{
    UINT_PTR addr;
    LPVOID address, address2;
    unsigned long int size;

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    //printf("%d\n", (int)si.dwAllocationGranularity);

    size = si.dwAllocationGranularity;

//    printf("%ld\n", size * 2);

    /* create a mapping backed by a pagefile */
    cb->hMapFile = CreateFileMapping (    
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        size * 2,
        "Mapping");
    assert(cb->hMapFile != NULL);

    /* find a free bufferSize*2 address space */
    cb->data = address = MapViewOfFile (    
        cb->hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,                   
        0,                   
        size * 2);
    assert(address != NULL);
    UnmapViewOfFile(address);

    /* found it. hopefully it'll remain free while we map to it */
    addr = ((UINT_PTR)address);
    address = MapViewOfFileEx (
        cb->hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,                   
        0,                   
        cb->size, 
        (LPVOID)addr );
    assert(address != NULL);

//    ErrorExit(TEXT("GetProcessId"));

    addr = ((UINT_PTR)address) + size;        
    address2 = MapViewOfFileEx (
        cb->hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,                   
        0,                   
        size,
        (LPVOID)addr);  

    assert(address2 != NULL);
}
#else

static void __init_cbuf_mmap(cbuf_t* cb)
{
    char path[] = "/dev/shm/ring-cb-XXXXXX";
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
}


#endif

/**
 * creat new circular buffer.
 * @param order to the power of two equals size*/
void *cbuf_new(const unsigned int order)
{
    cbuf_t *me;

    me = malloc(sizeof(cbuf_t));
    me->size = 1UL << order;
    me->start = 0;
    me->end = 0;
//    me->data = malloc(me->size);
//    me->data = malloc(me->size);

#if WIN32
    __init_cbuf_win32(me);
#else
    __init_cbuf_mmap(me);
#endif

    return me;
}

void cbuf_free(void * cb)
{
    cbuf_t *me = cb;

#if WIN32
    UnmapViewOfFile(me->data);
    UnmapViewOfFile(me->data + me->size * 2);
    CloseHandle(me->hMapFile);
#else
    munmap(me->data, me->size << 1);
#endif
    free(me);
}

int cbuf_is_empty(const void * cb)
{
    const cbuf_t *me = cb;
    return me->start == me->end;
}

/**
 * @return number of bytes offered
 * */
int cbuf_offer(void * cb, const unsigned char *data, const int size)
{
    cbuf_t *me = cb;
    int written;

    written = cbuf_get_unused_size(cb);
    written = size < written ? size : written;
    memcpy(me->data + me->end, data, written);
    me->end += written;
    me->end %= me->size;
    return written;
}

/**
 * Look at data.
 * Don't move cursor
 */
unsigned char *cbuf_peek(const void * cb)//, const int size)
{
    const cbuf_t *me = cb;

    if (cbuf_is_empty(cb))
        return NULL;

    return me->data + me->start;
}

/** 
 * Get pointer to data to read. Move the cursor on.
 *
 * @return pointer to data, null if we can't poll this much data
 */
unsigned char *cbuf_poll(void * cb, const unsigned int size)
{
    cbuf_t *me = cb;
    void *end;

//    printf("%lx %d\n", me->data, me->start);
//    printf("%lx %d %lx\n", me->data, me->start, me->data + me->start);
    if (cbuf_is_empty(cb))
        return NULL;

    end = me->data + me->start;
    me->start += size;
    return end;
}

int cbuf_get_size(const void * cb)
{
    const cbuf_t *me = cb;
    return me->size;
}

/**
 * @return tell us how much space we have assigned */
int cbuf_get_spaceused(const void* cb)
{
    const cbuf_t *me = cb;
    return me->end - me->start;
}
