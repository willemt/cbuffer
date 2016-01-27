#ifndef CBUFFER_H
#define CBUFFER_H

typedef struct
{
    unsigned long int size;
    unsigned int head, tail;
    void *data;
} cbuf_t;

/** Create new circular buffer.
 * @param order Size of the circular buffer when taking this as a power of 2.
 * @return pointer to new circular buffer */
cbuf_t *cbuf_new(const unsigned int order);

/** Free memory used by circular buffer
 * @param cb The circular buffer. */
void cbuf_free(cbuf_t* cb);

/** Write data to the tail of the circular buffer.
 * Increases the position of the tail.
 * This copies data to the circular buffer using memcpy.
 * After calling you should free the data if you don't need it anymore.
 *
 * @param cb The circular buffer.
 * @param data Buffer to be written to the circular buffer.
 * @param size Size of the buffer to be added to the circular buffer in bytes.
 * @return number of bytes offered */
int cbuf_offer(cbuf_t* cb, const unsigned char *data, const int size);

/** Look at data at the circular buffer's head.
 * Use cbuf_usedspace to determine how much data in bytes can be read.
 * @param cb The circular buffer.
 * @return pointer to the head of the circular buffer */
unsigned char *cbuf_peek(const cbuf_t* cb);

/** Release data at the head from the circular buffer.
 * Increase the position of the head.
 *
 * WARNING: this is a dangerous call if:
 *  1. You are using the returned data pointer.
 *  2. Another thread has offerred data to the circular buffer.
 *
 * If you want to access the data from the returned  pointer you are better off
 * using cbuf_peek.
 *
 * @param cb The circular buffer.
 * @param size Number of bytes to release
 * @return pointer to data; NULL if we can't poll this much data */
unsigned char *cbuf_poll(cbuf_t *cb, const unsigned int size);

/** Size in bytes of the circular buffer.
 * Is equal to 2 ^ order.
 *
 * @param cb The circular buffer.
 * @return size of the circular buffer in bytes */
int cbuf_size(const cbuf_t* cb);

/** Tell how much data has been written in bytes to the circular buffer.
 * @param cb The circular buffer.
 * @return number of bytes of how data has been written */
int cbuf_usedspace(const cbuf_t* cb);

/** Tell how much data we can write in bytes to the circular buffer.
 * @param cb The circular buffer.
 * @return number of bytes of how much data can be written */
int cbuf_unusedspace(const cbuf_t* cb);

/** Tell if the circular buffer is empty.
 * @param cb The circular buffer.
 * @return 1 if empty; otherwise 0 */
int cbuf_is_empty(const cbuf_t* cb);

#endif /* CBUFFER_H */
