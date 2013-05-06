

void *cbuf_new(const unsigned int order);

int cbuf_offer(void * cb, const unsigned char *data, const int size);

int cbuf_get_unused_size(const void * cb);

unsigned char *cbuf_peek(const void * cb);

unsigned char *cbuf_poll(void * cb, const unsigned int size);

void cbuf_poll_release(void * cb, const int size);

int cbuf_get_size(const void * cb);

void cbuf_free(void * cb);

int cbuf_is_empty(const void * cb);
