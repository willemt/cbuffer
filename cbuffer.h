typedef struct
{
    int size;
    int start, end;
    void *data;
} cbuf_t;


cbuf_t *cbuf_new(const unsigned int order);

int cbuf_offer(cbuf_t * cb, const unsigned char *data, const int size);

int cbuf_get_unused_size(const cbuf_t * cb);

unsigned char *cbuf_peek(const cbuf_t * cb);

unsigned char *cbuf_poll(cbuf_t * cb, const int size);

void cbuf_poll_release(cbuf_t * cb, const int size);

int cbuf_get_size(const cbuf_t * cb);

void cbuf_free(cbuf_t * cb);
