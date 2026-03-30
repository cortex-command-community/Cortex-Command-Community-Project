#!/usr/bin/env python3
"""
Development server for the Cortex Command WASM build.

Serves files from the build directory with the required COOP/COEP headers
that SharedArrayBuffer (and thus pthreads) require in browsers.

Usage:
    python3 emscripten/serve.py [build_dir] [port]
    python3 emscripten/serve.py build-web 8080
"""

import sys
import os
from http.server import HTTPServer, SimpleHTTPRequestHandler


class COOPCOEPHandler(SimpleHTTPRequestHandler):
    """Adds Cross-Origin-Opener-Policy and Cross-Origin-Embedder-Policy headers."""

    def end_headers(self):
        # Required for SharedArrayBuffer / Atomics (pthreads, WASM threads)
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        # Allow wasm MIME type
        self.send_header("Cache-Control", "no-store, no-cache, must-revalidate, max-age=0")
        super().end_headers()

    def guess_type(self, path):
        mime = super().guess_type(path)
        if isinstance(mime, tuple):
            mime = mime[0]
        if str(path).endswith(".wasm"):
            return "application/wasm"
        return mime

    def log_message(self, fmt, *args):
        # Suppress asset request noise; only log errors
        if args and str(args[1]) not in ("200", "304"):
            super().log_message(fmt, *args)


def main():
    build_dir = sys.argv[1] if len(sys.argv) > 1 else "build-web"
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 8080

    os.chdir(build_dir)
    server = HTTPServer(("", port), COOPCOEPHandler)
    print(f"Serving {os.path.abspath('.')} at http://localhost:{port}")
    print("Required headers: COOP=same-origin, COEP=require-corp")
    print("Press Ctrl+C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")


if __name__ == "__main__":
    main()
