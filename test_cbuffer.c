/*
 * =====================================================================================
 *
 *       Filename:  test_splaytree.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/26/11 22:19:05
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *        Company:  
 *
 * =====================================================================================
 */


#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CuTest.h"

#include "cbuffer.h"

void TestCbuffer_set_size_with_init(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(16);
    CuAssertTrue(tc, 1UL << 16 == cbuf_get_size(cb));
}

void TestCbuffer_is_empty_after_init(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(16);
    CuAssertTrue(tc, cbuf_is_empty(cb));
}

void TestCbuffer_is_not_empty_after_offer(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(16);
    cbuf_offer(cb, "abcd", 4);
    CuAssertTrue(tc, !cbuf_is_empty(cb));
}

void TestCbuffer_is_empty_after_poll_release(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(16);
    cbuf_offer(cb, "abcd", 4);
    cbuf_poll(cb, 4);
    CuAssertTrue(tc, cbuf_is_empty(cb));
}

void TxestCbuffer_cant_offer_if_full(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(0);

    CuAssertTrue(tc, 0 == cbuf_offer(cb, "1000", 4));
}

void TestCbuffer_offer_and_poll(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(16);

    cbuf_offer(cb, "abcd", 4);
//    CuAssertTrue(tc, 96 == cbuf_get_unused_size(cb));
    cbuf_peek(cb, 4);
    CuAssertTrue(tc, 0 == strncmp("abcd", cbuf_peek(cb, 4), 4));
}

void TestCbuffer_cant_poll_nonexistant(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(16);

    CuAssertTrue(tc, NULL == cbuf_peek(cb, 4));
}

void TestCbuffer_can_poll_twice_without_release(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(16);

    cbuf_offer(cb, "1000", 4);
    cbuf_peek(cb, 4);
    CuAssertTrue(tc, NULL != cbuf_peek(cb, 4));
}

void TestCbuffer_cant_poll_twice_when_released(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(16);

    cbuf_offer(cb, "1000", 4);
    cbuf_peek(cb, 4);
    cbuf_poll(cb, 4);
    CuAssertTrue(tc, NULL == cbuf_peek(cb, 4));
}

void TxestCbuffer_get_unused_when_overlapping(CuTest * tc)
{
    cbuf_t *cb;

    cb = cbuf_new(16);

    cbuf_offer(cb, "123", 3);
    cbuf_poll(cb, 2);
    cbuf_offer(cb, "45", 2);
    CuAssertTrue(tc, 0 == strncmp("1000", cbuf_peek(cb, 4), 4));
}
