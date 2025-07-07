# fmem
[![Build Status](https://img.shields.io/endpoint.svg?url=https%3A%2F%2Factions-badge.atrox.dev%2FKreijstal%2Ffmem%2Fbadge%3Fref%3Dmaster&style=plastic)](https://actions-badge.atrox.dev/Kreijstal/fmem/goto?ref=master) ![GitHub Workflow Status (with event)](https://img.shields.io/github/actions/workflow/status/kreijstal/fmem/cmake-multi-platform.yml?style=plastic&labelColor=blue)


Forked from Snaipe, since he hasn't reviewed pull request in the last 6 years.

A cross-platform library for opening memory-backed libc streams.

This library was written for [Criterion][criterion] to implement stringification functions for user-defined types.

## Rationale

C doesn't define any way to open "virtual" streams that write to memory rather than a real file. A lot of libc implementations roll their own nonstandard mechanisms to achieve this, namely `open_memstream`, or `fmemopen`. Other implementations provide more generic functions to call users functions for various operations on the file, like `funopen` or `fopencookie`. Finally, some implementations support none of these nonstandard functions.

fmem tries in sequence the following implementations:

* `open_memstream`.
* `fopencookie`, with growing dynamic buffer.
* `funopen`, with growing dynamic buffer.
* WinAPI temporary memory-backed file.

When no other mean is available, fmem falls back to `tmpfile()`.

[criterion]: https://github.com/Snaipe/Criterion
