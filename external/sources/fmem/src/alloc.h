#ifndef ALLOC_H_
#define ALLOC_H_

#include <stddef.h>

struct fmemi_buf {
    char *mem;
    size_t size;
};

struct fmem_stream {
    struct fmemi_buf *buf;
    size_t cursor;
    size_t region_size;
};

int fmemi_grow(struct fmem_stream *stream, size_t required);
int fmemi_cursor(struct fmemi_buf *buf, struct fmem_stream *from);
size_t fmemi_copy(struct fmemi_buf *to, struct fmemi_buf *from);

#endif /* !ALLOC_H_ */
