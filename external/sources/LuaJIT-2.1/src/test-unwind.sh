#!/bin/bash

TEST_CC="$1"

cat << EOF > tmpunwind.c
extern void b(void);

int a(void) {
    b();
    return 0;
}
EOF

$TEST_CC -c -x c tmpunwind.c -o tmpunwind.o && {
  grep -qa -e eh_frame -e __unwind_info tmpunwind.o ||
  grep -qU -e eh_frame -e __unwind_info tmpunwind.o
} && echo E

rm -f tmpunwind.c tmpunwind.o
