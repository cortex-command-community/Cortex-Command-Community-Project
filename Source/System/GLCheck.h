#pragma once

/// Debug function to print GL errors to the console from:
/// https : // stackoverflow.com/questions/11256470/define-a-macro-to-facilitate-opengl-command-debugging
void CheckOpenGLError(const char* stmt, const char* fname, int line);

/// Debug macro to be used for all GL calls.
#ifdef DEBUG
#define GL_CHECK(stmt) \
	do { \
		stmt; \
		CheckOpenGLError(#stmt, __FILE__, __LINE__); \
	} while (0)
#else
#define GL_CHECK(stmt) stmt
#endif
