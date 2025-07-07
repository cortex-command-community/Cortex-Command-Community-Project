#define _GNU_SOURCE
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "fmem.h"

union fmem_conv {
    fmem *fm;
    struct fmemi_buf *buf;
};

void fmem_init(fmem *file)
{
    union fmem_conv cv = { .fm = file };
    memset(cv.buf, 0, sizeof (*cv.buf));
}

void fmem_term(fmem *file)
{
    union fmem_conv cv = { .fm = file };
    free(cv.buf->mem);
}

static ssize_t mem_write(void *cookie, const char *buf, size_t size)
{
    struct fmem_stream *stream = cookie;

    struct fmemi_buf from = { (char *) buf, size };
    struct fmemi_buf to;

    if (fmemi_grow(stream, size) < 0) {
        return -1;
    }
    if (fmemi_cursor(&to, stream) < 0) {
        return 0;
    }

    size_t copied = fmemi_copy(&to, &from);
    stream->cursor += copied;
    if (stream->buf->size < stream->cursor)
        stream->buf->size = stream->cursor;
    return copied;
}

static ssize_t mem_read(void *cookie, char *buf, size_t size)
{
    struct fmem_stream *stream = cookie;

    struct fmemi_buf to = { buf, size };
    struct fmemi_buf from;

    if (fmemi_cursor(&from, stream) < 0) {
        return 0;
    }

    size_t copied = fmemi_copy(&to, &from);
    stream->cursor += copied;
    return copied;
}

static int mem_seek(void *cookie, off64_t *off, int whence)
{
    struct fmem_stream *stream = cookie;

    size_t newoff;
    switch (whence) {
        case SEEK_SET: newoff = *off; break;
        case SEEK_CUR: newoff = stream->cursor + *off; break;
        case SEEK_END: newoff = stream->buf->size + *off; break;
        default: errno = EINVAL; return -1;
    }
    if (newoff > stream->buf->size || (off64_t)newoff < 0) {
        return -1;
    }
    *off = newoff;
    stream->cursor = newoff;
    return 0;
}

static int mem_close(void *cookie)
{
    free(cookie);
    return 0;
}

FILE *fmem_open(fmem *file, const char *mode)
{
    static cookie_io_functions_t funcs = {
        .read   = mem_read,
        .write  = mem_write,
        .seek   = mem_seek,
        .close  = mem_close,
    };

    union fmem_conv cv = { .fm = file };

    free(cv.buf->mem);
    cv.buf->mem = malloc(128);
    if (!cv.buf->mem)
        return NULL;

    struct fmem_stream *stream = malloc(sizeof (*stream));
    if (!stream) {
        free(cv.buf->mem);
        cv.buf->mem = NULL;
        return NULL;
    }

    *stream = (struct fmem_stream) {
        .buf = cv.buf,
        .region_size = 128,
    };

    FILE *f = fopencookie(stream, mode, funcs);
    if (!f)
        free(stream);
    return f;
}

void fmem_mem(fmem *file, void **mem, size_t *size)
{
    union fmem_conv cv = { .fm = file };
    *mem = cv.buf->mem;
    *size = cv.buf->size;
}
