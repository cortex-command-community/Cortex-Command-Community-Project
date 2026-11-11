# SIMDe 0.8.2

[SIMD Everywhere](https://github.com/simd-everywhere/simde), MIT licensed. The licence is at the top of the header.

Only `simde/x86/sse2.h` is here, unchanged from the amalgamated release `simde-amalgamated-0.8.2.tar.xz` (sha256 `59068edc3420e75c5ff85ecfd80a77196fb3a151227a666cc20abb313a5360bf`). Amalgamated headers contain everything they include, so it needs nothing else from SIMDe.

It lets code written with SSE2 intrinsics build on every platform: as SSE2 on x86, and translated to NEON on ARM, such as Apple Silicon.
