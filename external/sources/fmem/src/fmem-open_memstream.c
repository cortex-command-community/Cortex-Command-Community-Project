#define _GNU_SOURCE
#include <stdio.h>
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

FILE *fmem_open(fmem *file, const char *mode)
{
    (void) mode;

    union fmem_conv cv = { .fm = file };
    free(cv.buf->mem);
    return open_memstream(&cv.buf->mem, &cv.buf->size);
}

void fmem_mem(fmem *file, void **mem, size_t *size)
{
    union fmem_conv cv = { .fm = file };
    *mem = cv.buf->mem;
    *size = cv.buf->size;
}

