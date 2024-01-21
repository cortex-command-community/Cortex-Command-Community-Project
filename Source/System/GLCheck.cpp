#include "GLCheck.h"
#include "glad/gl.h"

#include <cstdio>
#include <cstdlib>

void CheckOpenGLError(const char* stmt, const char* fname, int line) {
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
		abort();
	}
}
