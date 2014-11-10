#ifndef CBUFFER_H
#define CBUFFER_H

typedef struct
{
    unsigned long int size;
    unsigned int head, tail;
    void *data;
} cbuf_t;

/**
 * creat new circular buffer.
 * @param order to the power of two equals size*/
cbuf_t *cbuf_new(const unsigned int order);

void cbuf_free(cbuf_t* cb);

/**
 * @return number of bytes offered */
int cbuf_offer(cbuf_t* cb, const unsigned char *data, const int size);

/**
 * @return tell us how much space we have assigned */
int cbuf_usedspace(const cbuf_t* cb);

/**
 * @return tell us how much space we can assign */
int cbuf_unusedspace(const cbuf_t* cb);

/**
 * Look at data.
 * Don't move cursor */
unsigned char *cbuf_peek(const cbuf_t* cb);

/**
 * Get pointer to data to read. Move the cursor on.
 *
 * @return pointer to data, null if we can't poll this much data */
unsigned char *cbuf_poll(cbuf_t *cb, const unsigned int size);

void cbuf_poll_release(cbuf_t* cb, const int size);

int cbuf_size(const cbuf_t* cb);

int cbuf_is_empty(const cbuf_t* cb);

#endif /* CBUFFER_H */
