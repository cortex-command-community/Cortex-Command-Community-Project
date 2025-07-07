#include <criterion/criterion.h>
#include <stdio.h>

#include "fmem.h"

static fmem fm;

void setup(void)    { fmem_init(&fm); }
void teardown(void) { fmem_term(&fm); }

TestSuite(fmem, .init = setup, .fini = teardown);

#define assert_written(Fmem, What) do {                     \
        const char *__str = (What);                         \
        void *__base;                                       \
        size_t __size;                                      \
        fmem_mem(&(Fmem), &__base, &__size);                \
        cr_assert_eq(__size, strlen(__str),                 \
                "size is %llu, not %llu",                   \
                (unsigned long long) __size,                \
                (unsigned long long) strlen(__str));        \
        cr_assert(!memcmp((char *)__base, __str, __size));  \
    } while (0)

Test(fmem, open)
{
    FILE *f = fmem_open(&fm, "w+");
    cr_assert(f);
    fclose(f);
}

Test(fmem, mem)
{
    const char *str = "Hello world\n";

    FILE *f = fmem_open(&fm, "w+");
    fprintf(f, "%s", str);
    fflush(f);

    assert_written(fm, str);
    fclose(f);
}

Test(fmem, append)
{
    FILE *f = fmem_open(&fm, "w+");

    fprintf(f, "abcd");
    fflush(f);

    assert_written(fm, "abcd");

    fprintf(f, "efgh");
    fflush(f);

    assert_written(fm, "abcdefgh");
    fclose(f);
}

Test(fmem, reopen)
{
    FILE *f = fmem_open(&fm, "w+");
    fprintf(f, "abcd");
    fflush(f);

    assert_written(fm, "abcd");
    fclose(f);

    f = fmem_open(&fm, "w+");
    fprintf(f, "efgh");
    fflush(f);

    assert_written(fm, "efgh");
    fclose(f);
}

Test(fmem, cursor)
{
    FILE *f = fmem_open(&fm, "w+");
    fprintf(f, "abcd");
    fseek(f, 2, SEEK_SET);
    fprintf(f, "efgh");
    fflush(f);

    assert_written(fm, "abefgh");

    fclose(f);
}

Test(fmem, large)
{
    char buf[4096];
    memset(buf, 0xcc, sizeof (buf));

    FILE *f = fmem_open(&fm, "w+");

    for (size_t i = 0; i < 1024; ++i) {
        size_t written = fwrite(buf, 1, sizeof (buf), f);
        if (written < sizeof (buf)) {
            cr_assert_fail("could only write %llu/%llu bytes",
                    (unsigned long long) written,
                    (unsigned long long) sizeof (buf));
        }
    }

    fflush(f);

    void *base;
    size_t size;
    fmem_mem(&fm, &base, &size);

    cr_assert_eq(size, 1024 * sizeof (buf),
            "size is %llu, not %llu",
            (unsigned long long) size,
            (unsigned long long) 1024 * sizeof (buf));

    fclose(f);
}
