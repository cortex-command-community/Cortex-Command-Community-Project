#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fmem.h"

struct fmem_impl {
    FILE *file;
    void *buf;
};

union fmem_conv {
    fmem *fm;
    struct fmem_impl *impl;
};

void fmem_init(fmem *file)
{
    union fmem_conv cv = { .fm = file };
    memset(cv.impl, 0, sizeof (*cv.impl));
}

void fmem_term(fmem *file)
{
    union fmem_conv cv = { .fm = file };

    if (cv.impl->buf) {
        free(cv.impl->buf);
    }
}

FILE *fmem_open(fmem *file, const char *mode)
{
    union fmem_conv cv = { .fm = file };
    FILE *f = tmpfile();
    if (f) {
        if (cv.impl->buf) {
            free(cv.impl->buf);
        }
        cv.impl->file = f;
    }
    return f;
}

void fmem_mem(fmem *file, void **mem, size_t *size)
{
    union fmem_conv cv = { .fm = file };
    *mem = NULL;
    *size = 0;

    if (cv.impl->buf) {
        free(cv.impl->buf);
        cv.impl->buf = NULL;
    }

    fseek(cv.impl->file, 0, SEEK_END);
    long bufsize = ftell(cv.impl->file);
    if (bufsize < 0) {
        return;
    }

    void *buf = malloc(bufsize);
    if (!buf) {
        return;
    }

    rewind(cv.impl->file);
    if (fread(buf, 1, bufsize, cv.impl->file) < (size_t)bufsize) {
        free(buf);
        return;
    }
    cv.impl->buf = buf;
    *mem = buf;
    *size = bufsize;
}
