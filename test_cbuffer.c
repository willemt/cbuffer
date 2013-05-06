#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"

#include "cbuffer.h"

void TestCbuffer_set_size_with_init(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);
    CuAssertTrue(tc, 1UL << 16 == cbuf_get_size(cb));
}

void TestCbuffer_is_empty_after_init(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);
    CuAssertTrue(tc, cbuf_is_empty(cb));
}

void TestCbuffer_is_not_empty_after_offer(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);
    cbuf_offer(cb, (unsigned char*)"abcd", 4);
    CuAssertTrue(tc, !cbuf_is_empty(cb));
}

void TestCbuffer_is_empty_after_poll_release(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);
    cbuf_offer(cb, (unsigned char*)"abcd", 4);
    cbuf_poll(cb, 4);
    CuAssertTrue(tc, cbuf_is_empty(cb));
}

void TxestCbuffer_cant_offer_if_full(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(0);

    CuAssertTrue(tc, 0 == cbuf_offer(cb, (unsigned char*)"1000", 4));
}

void TestCbuffer_offer_and_poll(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);

    cbuf_offer(cb, (unsigned char*)"abcd", 4);
//    CuAssertTrue(tc, 96 == cbuf_get_unused_size(cb));
    CuAssertTrue(tc, 0 == strncmp("abcd", (char*)cbuf_poll(cb, 4), 4));
}

void TestCbuffer_cant_poll_nonexistant(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);

    CuAssertTrue(tc, NULL == cbuf_poll(cb, 4));
}

#if 0
void TxestCbuffer_can_poll_twice_without_release(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);

    cbuf_offer(cb, (unsigned char*)"1000", 4);
    cbuf_poll(cb, 4);
    CuAssertTrue(tc, NULL != cbuf_poll(cb, 4));
}
#endif

void TestCbuffer_cant_poll_twice_when_released(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);

    cbuf_offer(cb, (unsigned char*)"1000", 4);
    cbuf_poll(cb, 4);
    cbuf_poll(cb, 4);
    CuAssertTrue(tc, NULL == cbuf_poll(cb, 4));
}

#if 0
void TxestCbuffer_get_unused_when_overlapping(CuTest * tc)
{
    void *cb;

    cb = cbuf_new(16);

    cbuf_offer(cb, (unsigned char*)"123", 3);
    cbuf_poll(cb, 2);
    cbuf_offer(cb, (unsigned char*)"45", 2);
    CuAssertTrue(tc, 0 == strncmp("1000", (char*)cbuf_poll(cb, 4), 4));
}
#endif
