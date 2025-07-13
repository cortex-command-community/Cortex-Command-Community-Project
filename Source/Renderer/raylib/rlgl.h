/**********************************************************************************************
 *
 *   rlgl v5.0 - A multi-OpenGL abstraction layer with an immediate-mode style API
 *
 *   DESCRIPTION:
 *       An abstraction layer for multiple OpenGL versions (1.1, 2.1, 3.3 Core, 4.3 Core, ES 2.0)
 *       that provides a pseudo-OpenGL 1.1 immediate-mode style API (rlVertex, rlTranslate, rlRotate...)
 *
 *   ADDITIONAL NOTES:
 *       When choosing an OpenGL backend different than OpenGL 1.1, some internal buffer are
 *       initialized on rlglInit() to accumulate vertex data
 *
 *       When an internal state change is required all the stored vertex data is renderer in batch,
 *       additionally, rlDrawRenderBatchActive() could be called to force flushing of the batch
 *
 *       Some resources are also loaded for convenience, here the complete list:
 *          - Default batch (RLGL.defaultBatch): RenderBatch system to accumulate vertex data
 *          - Default texture (RLGL.defaultTextureId): 1x1 white pixel R8G8B8A8
 *          - Default shader (RLGL.State.defaultShaderId, RLGL.State.defaultShaderLocs)
 *
 *       Internal buffer (and resources) must be manually unloaded calling rlglClose()
 *
 *   CONFIGURATION:
 *       #define GRAPHICS_API_OPENGL_11
 *       #define GRAPHICS_API_OPENGL_21
 *       #define GRAPHICS_API_OPENGL_33
 *       #define GRAPHICS_API_OPENGL_43
 *       #define GRAPHICS_API_OPENGL_ES2
 *       #define GRAPHICS_API_OPENGL_ES3
 *           Use selected OpenGL graphics backend, should be supported by platform
 *           Those preprocessor defines are only used on rlgl module, if OpenGL version is
 *           required by any other module, use rlGetVersion() to check it
 *
 *       #define RLGL_IMPLEMENTATION
 *           Generates the implementation of the library into the included file
 *           If not defined, the library is in header only mode and can be included in other headers
 *           or source files without problems. But only ONE file should hold the implementation
 *
 *       #define RLGL_RENDER_TEXTURES_HINT
 *           Enable framebuffer objects (fbo) support (enabled by default)
 *           Some GPUs could not support them despite the OpenGL version
 *
 *       #define RLGL_SHOW_GL_DETAILS_INFO
 *           Show OpenGL extensions and capabilities detailed logs on init
 *
 *       #define RLGL_ENABLE_OPENGL_DEBUG_CONTEXT
 *           Enable debug context (only available on OpenGL 4.3)
 *
 *       rlgl capabilities could be customized just defining some internal
 *       values before library inclusion (default values listed):
 *
 *       #define RL_DEFAULT_BATCH_BUFFER_ELEMENTS   8192    // Default internal render batch elements limits
 *       #define RL_DEFAULT_BATCH_BUFFERS              1    // Default number of batch buffers (multi-buffering)
 *       #define RL_DEFAULT_BATCH_DRAWCALLS          256    // Default number of batch draw calls (by state changes: mode, texture)
 *       #define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS    4    // Maximum number of textures units that can be activated on batch drawing (SetShaderValueTexture())
 *
 *       #define RL_MAX_MATRIX_STACK_SIZE             32    // Maximum size of internal RLMatrix stack
 *       #define RL_MAX_SHADER_LOCATIONS              32    // Maximum number of shader locations supported
 *       #define RL_CULL_DISTANCE_NEAR              0.01    // Default projection matrix near cull distance
 *       #define RL_CULL_DISTANCE_FAR             1000.0    // Default projection matrix far cull distance
 *
 *       When loading a shader, the following vertex attributes and uniform
 *       location names are tried to be set automatically:
 *
 *       #define RL_DEFAULT_SHADER_ATTRIB_NAME_POSITION     "vertexPosition"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
 *       #define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD     "vertexTexCoord"    // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD
 *       #define RL_DEFAULT_SHADER_ATTRIB_NAME_NORMAL       "vertexNormal"      // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL
 *       #define RL_DEFAULT_SHADER_ATTRIB_NAME_COLOR        "vertexColor"       // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR
 *       #define RL_DEFAULT_SHADER_ATTRIB_NAME_TANGENT      "vertexTangent"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT
 *       #define RL_DEFAULT_SHADER_ATTRIB_NAME_TEXCOORD2    "vertexTexCoord2"   // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2
 *       #define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEIDS      "vertexBoneIds"     // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS
 *       #define RL_DEFAULT_SHADER_ATTRIB_NAME_BONEWEIGHTS  "vertexBoneWeights" // Bound by default to shader location: RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS
 *       #define RL_DEFAULT_SHADER_UNIFORM_NAME_MVP         "mvp"               // model-view-projection matrix
 *       #define RL_DEFAULT_SHADER_UNIFORM_NAME_VIEW        "matView"           // view matrix
 *       #define RL_DEFAULT_SHADER_UNIFORM_NAME_PROJECTION  "matProjection"     // projection matrix
 *       #define RL_DEFAULT_SHADER_UNIFORM_NAME_MODEL       "matModel"          // model matrix
 *       #define RL_DEFAULT_SHADER_UNIFORM_NAME_NORMAL      "matNormal"         // normal matrix (transpose(inverse(matModelView)))
 *       #define RL_DEFAULT_SHADER_UNIFORM_NAME_COLOR       "colDiffuse"        // color diffuse (base tint color, multiplied by texture color)
 *       #define RL_DEFAULT_SHADER_UNIFORM_NAME_BONE_MATRICES  "boneMatrices"   // bone matrices
 *       #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE0  "texture0"          // texture0 (texture slot active 0)
 *       #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE1  "texture1"          // texture1 (texture slot active 1)
 *       #define RL_DEFAULT_SHADER_SAMPLER2D_NAME_TEXTURE2  "texture2"          // texture2 (texture slot active 2)
 *
 *   DEPENDENCIES:
 *      - OpenGL libraries (depending on platform and OpenGL version selected)
 *      - GLAD OpenGL extensions loading library (only for OpenGL 3.3 Core, 4.3 Core)
 *
 *
 *   LICENSE: zlib/libpng
 *
 *   Copyright (c) 2014-2024 Ramon Santamaria (@raysan5)
 *
 *   This software is provided "as-is", without any express or implied warranty. In no event
 *   will the authors be held liable for any damages arising from the use of this software.
 *
 *   Permission is granted to anyone to use this software for any purpose, including commercial
 *   applications, and to alter it and redistribute it freely, subject to the following restrictions:
 *
 *     1. The origin of this software must not be misrepresented; you must not claim that you
 *     wrote the original software. If you use this software in a product, an acknowledgment
 *     in the product documentation would be appreciated but is not required.
 *
 *     2. Altered source versions must be plainly marked as such, and must not be misrepresented
 *     as being the original software.
 *
 *     3. This notice may not be removed or altered from any source distribution.
 *
 **********************************************************************************************/

#ifndef RLGL_H
#define RLGL_H

#define RLGL_VERSION "5.0"

// Function specifiers in case library is build/used as a shared library
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
// NOTE: visibility(default) attribute makes symbols "visible" when compiled with -fvisibility=hidden
#if defined(_WIN32) && defined(BUILD_LIBTYPE_SHARED)
#define RLAPI __declspec(dllexport) // We are building the library as a Win32 shared library (.dll)
#elif defined(BUILD_LIBTYPE_SHARED)
#define RLAPI __attribute__((visibility("default"))) // We are building the library as a Unix shared library (.so/.dylib)
#elif defined(_WIN32) && defined(USE_LIBTYPE_SHARED)
#define RLAPI __declspec(dllimport) // We are using the library as a Win32 shared library (.dll)
#endif

// Function specifiers definition
#ifndef RLAPI
#define RLAPI // Functions defined as 'extern' by default (implicit specifiers)
#endif
#include <stdbool.h>
// Support TRACELOG macros
#ifndef TRACELOG
#define TRACELOG(level, ...) (void)0
#define TRACELOGD(...) (void)0
#endif

// Allow custom memory allocators
#ifndef RL_MALLOC
#define RL_MALLOC(sz) malloc(sz)
#endif
#ifndef RL_CALLOC
#define RL_CALLOC(n, sz) calloc(n, sz)
#endif
#ifndef RL_REALLOC
#define RL_REALLOC(n, sz) realloc(n, sz)
#endif
#ifndef RL_FREE
#define RL_FREE(p) free(p)
#endif

#define GRAPHICS_API_OPENGL_33

// Security check in case no GRAPHICS_API_OPENGL_* defined
#if !defined(GRAPHICS_API_OPENGL_11) && \
    !defined(GRAPHICS_API_OPENGL_21) && \
    !defined(GRAPHICS_API_OPENGL_33) && \
    !defined(GRAPHICS_API_OPENGL_43) && \
    !defined(GRAPHICS_API_OPENGL_ES2) && \
    !defined(GRAPHICS_API_OPENGL_ES3)
#define GRAPHICS_API_OPENGL_33
#endif

// Security check in case multiple GRAPHICS_API_OPENGL_* defined
#if defined(GRAPHICS_API_OPENGL_11)
#if defined(GRAPHICS_API_OPENGL_21)
#undef GRAPHICS_API_OPENGL_21
#endif
#if defined(GRAPHICS_API_OPENGL_33)
#undef GRAPHICS_API_OPENGL_33
#endif
#if defined(GRAPHICS_API_OPENGL_43)
#undef GRAPHICS_API_OPENGL_43
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
#undef GRAPHICS_API_OPENGL_ES2
#endif
#endif

// OpenGL 2.1 uses most of OpenGL 3.3 Core functionality
// WARNING: Specific parts are checked with #if defines
#if defined(GRAPHICS_API_OPENGL_21)
#define GRAPHICS_API_OPENGL_33
#endif

// OpenGL 4.3 uses OpenGL 3.3 Core functionality
#if defined(GRAPHICS_API_OPENGL_43)
#define GRAPHICS_API_OPENGL_33
#endif

// OpenGL ES 3.0 uses OpenGL ES 2.0 functionality (and more)
#if defined(GRAPHICS_API_OPENGL_ES3)
#define GRAPHICS_API_OPENGL_ES2
#endif

// Support framebuffer objects by default
// NOTE: Some driver implementation do not support it, despite they should
#define RLGL_RENDER_TEXTURES_HINT

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------

// Default internal render batch elements limits
#ifndef RL_DEFAULT_BATCH_BUFFER_ELEMENTS
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
// This is the maximum amount of elements (quads) per batch
// NOTE: Be careful with text, every letter maps to a quad
#define RL_DEFAULT_BATCH_BUFFER_ELEMENTS 8192
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
// We reduce memory sizes for embedded systems (RPI and HTML5)
// NOTE: On HTML5 (emscripten) this is allocated on heap,
// by default it's only 16MB!...just take care...
#define RL_DEFAULT_BATCH_BUFFER_ELEMENTS 2048
#endif
#endif
#ifndef RL_DEFAULT_BATCH_BUFFERS
#define RL_DEFAULT_BATCH_BUFFERS 1 // Default number of batch buffers (multi-buffering)
#endif
#ifndef RL_DEFAULT_BATCH_DRAWCALLS
#define RL_DEFAULT_BATCH_DRAWCALLS 256 // Default number of batch draw calls (by state changes: mode, texture)
#endif
#ifndef RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS
#define RL_DEFAULT_BATCH_MAX_TEXTURE_UNITS 16 // Maximum number of textures units that can be activated on batch drawing (SetShaderValueTexture())
#endif

// Internal RLMatrix stack
#ifndef RL_MAX_MATRIX_STACK_SIZE
#define RL_MAX_MATRIX_STACK_SIZE 32 // Maximum size of RLMatrix stack
#endif

// Shader limits
#ifndef RL_MAX_SHADER_LOCATIONS
#define RL_MAX_SHADER_LOCATIONS 32 // Maximum number of shader locations supported
#endif

// Projection matrix culling
#ifndef RL_CULL_DISTANCE_NEAR
#define RL_CULL_DISTANCE_NEAR 0.01 // Default near cull distance
#endif
#ifndef RL_CULL_DISTANCE_FAR
#define RL_CULL_DISTANCE_FAR 1000.0 // Default far cull distance
#endif

// Texture parameters (equivalent to OpenGL defines)
#define RL_TEXTURE_WRAP_S 0x2802 // GL_TEXTURE_WRAP_S
#define RL_TEXTURE_WRAP_T 0x2803 // GL_TEXTURE_WRAP_T
#define RL_TEXTURE_MAG_FILTER 0x2800 // GL_TEXTURE_MAG_FILTER
#define RL_TEXTURE_MIN_FILTER 0x2801 // GL_TEXTURE_MIN_FILTER

#define RL_TEXTURE_FILTER_NEAREST 0x2600 // GL_NEAREST
#define RL_TEXTURE_FILTER_LINEAR 0x2601 // GL_LINEAR
#define RL_TEXTURE_FILTER_MIP_NEAREST 0x2700 // GL_NEAREST_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_NEAREST_MIP_LINEAR 0x2702 // GL_NEAREST_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_LINEAR_MIP_NEAREST 0x2701 // GL_LINEAR_MIPMAP_NEAREST
#define RL_TEXTURE_FILTER_MIP_LINEAR 0x2703 // GL_LINEAR_MIPMAP_LINEAR
#define RL_TEXTURE_FILTER_ANISOTROPIC 0x3000 // Anisotropic filter (custom identifier)
#define RL_TEXTURE_MIPMAP_BIAS_RATIO 0x4000 // Texture mipmap bias, percentage ratio (custom identifier)

#define RL_TEXTURE_WRAP_REPEAT 0x2901 // GL_REPEAT
#define RL_TEXTURE_WRAP_CLAMP 0x812F // GL_CLAMP_TO_EDGE
#define RL_TEXTURE_WRAP_MIRROR_REPEAT 0x8370 // GL_MIRRORED_REPEAT
#define RL_TEXTURE_WRAP_MIRROR_CLAMP 0x8742 // GL_MIRROR_CLAMP_EXT

// RLMatrix modes (equivalent to OpenGL)
#define RL_MODELVIEW 0x1700 // GL_MODELVIEW
#define RL_PROJECTION 0x1701 // GL_PROJECTION
#define RL_TEXTURE 0x1702 // GL_TEXTURE

// Primitive assembly draw modes
#define RL_LINES 0x0001 // GL_LINES
#define RL_TRIANGLES 0x0004 // GL_TRIANGLES
#define RL_QUADS 0x0007 // GL_QUADS

// GL equivalent data types
#define RL_UNSIGNED_BYTE 0x1401 // GL_UNSIGNED_BYTE
#define RL_FLOAT 0x1406 // GL_FLOAT

// GL buffer usage hint
#define RL_STREAM_DRAW 0x88E0 // GL_STREAM_DRAW
#define RL_STREAM_READ 0x88E1 // GL_STREAM_READ
#define RL_STREAM_COPY 0x88E2 // GL_STREAM_COPY
#define RL_STATIC_DRAW 0x88E4 // GL_STATIC_DRAW
#define RL_STATIC_READ 0x88E5 // GL_STATIC_READ
#define RL_STATIC_COPY 0x88E6 // GL_STATIC_COPY
#define RL_DYNAMIC_DRAW 0x88E8 // GL_DYNAMIC_DRAW
#define RL_DYNAMIC_READ 0x88E9 // GL_DYNAMIC_READ
#define RL_DYNAMIC_COPY 0x88EA // GL_DYNAMIC_COPY

// GL Shader type
#define RL_FRAGMENT_SHADER 0x8B30 // GL_FRAGMENT_SHADER
#define RL_VERTEX_SHADER 0x8B31 // GL_VERTEX_SHADER
#define RL_COMPUTE_SHADER 0x91B9 // GL_COMPUTE_SHADER

// GL blending factors
#define RL_ZERO 0 // GL_ZERO
#define RL_ONE 1 // GL_ONE
#define RL_SRC_COLOR 0x0300 // GL_SRC_COLOR
#define RL_ONE_MINUS_SRC_COLOR 0x0301 // GL_ONE_MINUS_SRC_COLOR
#define RL_SRC_ALPHA 0x0302 // GL_SRC_ALPHA
#define RL_ONE_MINUS_SRC_ALPHA 0x0303 // GL_ONE_MINUS_SRC_ALPHA
#define RL_DST_ALPHA 0x0304 // GL_DST_ALPHA
#define RL_ONE_MINUS_DST_ALPHA 0x0305 // GL_ONE_MINUS_DST_ALPHA
#define RL_DST_COLOR 0x0306 // GL_DST_COLOR
#define RL_ONE_MINUS_DST_COLOR 0x0307 // GL_ONE_MINUS_DST_COLOR
#define RL_SRC_ALPHA_SATURATE 0x0308 // GL_SRC_ALPHA_SATURATE
#define RL_CONSTANT_COLOR 0x8001 // GL_CONSTANT_COLOR
#define RL_ONE_MINUS_CONSTANT_COLOR 0x8002 // GL_ONE_MINUS_CONSTANT_COLOR
#define RL_CONSTANT_ALPHA 0x8003 // GL_CONSTANT_ALPHA
#define RL_ONE_MINUS_CONSTANT_ALPHA 0x8004 // GL_ONE_MINUS_CONSTANT_ALPHA

// GL blending functions/equations
#define RL_FUNC_ADD 0x8006 // GL_FUNC_ADD
#define RL_MIN 0x8007 // GL_MIN
#define RL_MAX 0x8008 // GL_MAX
#define RL_FUNC_SUBTRACT 0x800A // GL_FUNC_SUBTRACT
#define RL_FUNC_REVERSE_SUBTRACT 0x800B // GL_FUNC_REVERSE_SUBTRACT
#define RL_BLEND_EQUATION 0x8009 // GL_BLEND_EQUATION
#define RL_BLEND_EQUATION_RGB 0x8009 // GL_BLEND_EQUATION_RGB   // (Same as BLEND_EQUATION)
#define RL_BLEND_EQUATION_ALPHA 0x883D // GL_BLEND_EQUATION_ALPHA
#define RL_BLEND_DST_RGB 0x80C8 // GL_BLEND_DST_RGB
#define RL_BLEND_SRC_RGB 0x80C9 // GL_BLEND_SRC_RGB
#define RL_BLEND_DST_ALPHA 0x80CA // GL_BLEND_DST_ALPHA
#define RL_BLEND_SRC_ALPHA 0x80CB // GL_BLEND_SRC_ALPHA
#define RL_BLEND_COLOR 0x8005 // GL_BLEND_COLOR

#define RL_READ_FRAMEBUFFER 0x8CA8 // GL_READ_FRAMEBUFFER
#define RL_DRAW_FRAMEBUFFER 0x8CA9 // GL_DRAW_FRAMEBUFFER

// Default shader vertex attribute locations
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_POSITION 0
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD 1
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_NORMAL 2
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_COLOR 3
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TANGENT 4
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_TEXCOORD2 5
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_INDICES 6
#endif
#ifdef RL_SUPPORT_MESH_GPU_SKINNING
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEIDS 7
#endif
#ifndef RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS
#define RL_DEFAULT_SHADER_ATTRIB_LOCATION_BONEWEIGHTS 8
#endif
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
#if (defined(__STDC__) && __STDC_VERSION__ >= 199901L) || (defined(_MSC_VER) && _MSC_VER >= 1800)
#include <stdbool.h>
#elif !defined(__cplusplus) && !defined(bool) && !defined(RL_BOOL_TYPE)
// Boolean type
typedef enum bool {
	false = 0,
	true = !false
} bool;
#endif

#if !defined(RL_MATRIX_TYPE)
// Matrix, 4x4 components, column major, OpenGL style, right handed
typedef struct RLMatrix {
	float m0, m4, m8, m12; // RLMatrix first row (4 components)
	float m1, m5, m9, m13; // RLMatrix second row (4 components)
	float m2, m6, m10, m14; // RLMatrix third row (4 components)
	float m3, m7, m11, m15; // RLMatrix fourth row (4 components)
} RLMatrix;
#define RL_MATRIX_TYPE
#endif

// OpenGL version
typedef enum {
	RL_OPENGL_11 = 1, // OpenGL 1.1
	RL_OPENGL_21, // OpenGL 2.1 (GLSL 120)
	RL_OPENGL_33, // OpenGL 3.3 (GLSL 330)
	RL_OPENGL_43, // OpenGL 4.3 (using GLSL 330)
	RL_OPENGL_ES_20, // OpenGL ES 2.0 (GLSL 100)
	RL_OPENGL_ES_30 // OpenGL ES 3.0 (GLSL 300 es)
} rlGlVersion;

// Trace log level
// NOTE: Organized by priority level
typedef enum {
	RL_LOG_ALL = 0, // Display all logs
	RL_LOG_TRACE, // Trace logging, intended for internal use only
	RL_LOG_DEBUG, // Debug logging, used for internal debugging, it should be disabled on release builds
	RL_LOG_INFO, // Info logging, used for program execution info
	RL_LOG_WARNING, // Warning logging, used on recoverable failures
	RL_LOG_ERROR, // Error logging, used on unrecoverable failures
	RL_LOG_FATAL, // Fatal logging, used to abort program: exit(EXIT_FAILURE)
	RL_LOG_NONE // Disable logging
} rlTraceLogLevel;

// Texture pixel formats
// NOTE: Support depends on OpenGL version
typedef enum {
	RL_PIXELFORMAT_UNCOMPRESSED_GRAYSCALE = 1, // 8 bit per pixel (no alpha)
	RL_PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA, // 8*2 bpp (2 channels)
	RL_PIXELFORMAT_UNCOMPRESSED_R5G6B5, // 16 bpp
	RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8, // 24 bpp
	RL_PIXELFORMAT_UNCOMPRESSED_R5G5B5A1, // 16 bpp (1 bit alpha)
	RL_PIXELFORMAT_UNCOMPRESSED_R4G4B4A4, // 16 bpp (4 bit alpha)
	RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, // 32 bpp
	RL_PIXELFORMAT_UNCOMPRESSED_R32, // 32 bpp (1 channel - float)
	RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32, // 32*3 bpp (3 channels - float)
	RL_PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, // 32*4 bpp (4 channels - float)
	RL_PIXELFORMAT_UNCOMPRESSED_R16, // 16 bpp (1 channel - half float)
	RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16, // 16*3 bpp (3 channels - half float)
	RL_PIXELFORMAT_UNCOMPRESSED_R16G16B16A16, // 16*4 bpp (4 channels - half float)
	RL_PIXELFORMAT_COMPRESSED_DXT1_RGB, // 4 bpp (no alpha)
	RL_PIXELFORMAT_COMPRESSED_DXT1_RGBA, // 4 bpp (1 bit alpha)
	RL_PIXELFORMAT_COMPRESSED_DXT3_RGBA, // 8 bpp
	RL_PIXELFORMAT_COMPRESSED_DXT5_RGBA, // 8 bpp
	RL_PIXELFORMAT_COMPRESSED_ETC1_RGB, // 4 bpp
	RL_PIXELFORMAT_COMPRESSED_ETC2_RGB, // 4 bpp
	RL_PIXELFORMAT_COMPRESSED_ETC2_EAC_RGBA, // 8 bpp
	RL_PIXELFORMAT_COMPRESSED_PVRT_RGB, // 4 bpp
	RL_PIXELFORMAT_COMPRESSED_PVRT_RGBA, // 4 bpp
	RL_PIXELFORMAT_COMPRESSED_ASTC_4x4_RGBA, // 8 bpp
	RL_PIXELFORMAT_COMPRESSED_ASTC_8x8_RGBA // 2 bpp
} rlPixelFormat;

// Texture parameters: filter mode
// NOTE 1: Filtering considers mipmaps if available in the texture
// NOTE 2: Filter is accordingly set for minification and magnification
typedef enum {
	RL_TEXTURE_FILTER_POINT = 0, // No filter, just pixel approximation
	RL_TEXTURE_FILTER_BILINEAR, // Linear filtering
	RL_TEXTURE_FILTER_TRILINEAR, // Trilinear filtering (linear with mipmaps)
	RL_TEXTURE_FILTER_ANISOTROPIC_4X, // Anisotropic filtering 4x
	RL_TEXTURE_FILTER_ANISOTROPIC_8X, // Anisotropic filtering 8x
	RL_TEXTURE_FILTER_ANISOTROPIC_16X, // Anisotropic filtering 16x
} rlTextureFilter;

// Color blending modes (pre-defined)
typedef enum {
	RL_BLEND_ALPHA = 0, // Blend textures considering alpha (default)
	RL_BLEND_BURN,
	RL_BLEND_DODGE,
	RL_BLEND_SCREEN,
	RL_BLEND_HSL_COLOR,
	RL_BLEND_HSL_SATURATION,
	RL_BLEND_HSL_LUMINOSITY,
	RL_BLEND_HSL_HUE,
	RL_BLEND_ADDITIVE, // Blend textures adding colors
	RL_BLEND_MULTIPLIED, // Blend textures multiplying colors
	RL_BLEND_ADD_COLORS, // Blend textures adding colors (alternative)
	RL_BLEND_SUBTRACT_COLORS, // Blend textures subtracting colors (alternative)
	RL_BLEND_ALPHA_PREMULTIPLY, // Blend premultiplied textures considering alpha
	RL_BLEND_CUSTOM, // Blend textures using custom src/dst factors (use rlSetBlendFactors())
	RL_BLEND_CUSTOM_SEPARATE // Blend textures using custom src/dst factors (use rlSetBlendFactorsSeparate())
} rlBlendMode;

// Shader location point type
typedef enum {
	RL_SHADER_LOC_VERTEX_POSITION = 0, // Shader location: vertex attribute: position
	RL_SHADER_LOC_VERTEX_TEXCOORD01, // Shader location: vertex attribute: texcoord01
	RL_SHADER_LOC_VERTEX_TEXCOORD02, // Shader location: vertex attribute: texcoord02
	RL_SHADER_LOC_VERTEX_NORMAL, // Shader location: vertex attribute: normal
	RL_SHADER_LOC_VERTEX_TANGENT, // Shader location: vertex attribute: tangent
	RL_SHADER_LOC_VERTEX_COLOR, // Shader location: vertex attribute: color
	RL_SHADER_LOC_MATRIX_MVP, // Shader location: matrix uniform: model-view-projection
	RL_SHADER_LOC_MATRIX_VIEW, // Shader location: matrix uniform: view (camera transform)
	RL_SHADER_LOC_MATRIX_PROJECTION, // Shader location: matrix uniform: projection
	RL_SHADER_LOC_MATRIX_MODEL, // Shader location: matrix uniform: model (transform)
	RL_SHADER_LOC_MATRIX_NORMAL, // Shader location: matrix uniform: normal
	RL_SHADER_LOC_VECTOR_VIEW, // Shader location: vector uniform: view
	RL_SHADER_LOC_COLOR_DIFFUSE, // Shader location: vector uniform: diffuse color
	RL_SHADER_LOC_COLOR_SPECULAR, // Shader location: vector uniform: specular color
	RL_SHADER_LOC_COLOR_AMBIENT, // Shader location: vector uniform: ambient color
	RL_SHADER_LOC_MAP_ALBEDO, // Shader location: sampler2d texture: albedo (same as: RL_SHADER_LOC_MAP_DIFFUSE)
	RL_SHADER_LOC_MAP_METALNESS, // Shader location: sampler2d texture: metalness (same as: RL_SHADER_LOC_MAP_SPECULAR)
	RL_SHADER_LOC_MAP_NORMAL, // Shader location: sampler2d texture: normal
	RL_SHADER_LOC_MAP_ROUGHNESS, // Shader location: sampler2d texture: roughness
	RL_SHADER_LOC_MAP_OCCLUSION, // Shader location: sampler2d texture: occlusion
	RL_SHADER_LOC_MAP_EMISSION, // Shader location: sampler2d texture: emission
	RL_SHADER_LOC_MAP_HEIGHT, // Shader location: sampler2d texture: height
	RL_SHADER_LOC_MAP_CUBEMAP, // Shader location: samplerCube texture: cubemap
	RL_SHADER_LOC_MAP_IRRADIANCE, // Shader location: samplerCube texture: irradiance
	RL_SHADER_LOC_MAP_PREFILTER, // Shader location: samplerCube texture: prefilter
	RL_SHADER_LOC_MAP_BRDF, // Shader location: sampler2d texture: brdf
	RL_SHADER_LOC_COUNT
} rlShaderLocationIndex;

#define RL_SHADER_LOC_MAP_DIFFUSE RL_SHADER_LOC_MAP_ALBEDO
#define RL_SHADER_LOC_MAP_SPECULAR RL_SHADER_LOC_MAP_METALNESS

// Shader uniform data type
typedef enum {
	RL_SHADER_UNIFORM_INVALID = -1,
	RL_SHADER_UNIFORM_FLOAT = 0, // Shader uniform type: float
	RL_SHADER_UNIFORM_VEC2, // Shader uniform type: vec2 (2 float)
	RL_SHADER_UNIFORM_VEC3, // Shader uniform type: vec3 (3 float)
	RL_SHADER_UNIFORM_VEC4, // Shader uniform type: vec4 (4 float)
	RL_SHADER_UNIFORM_INT, // Shader uniform type: int
	RL_SHADER_UNIFORM_IVEC2, // Shader uniform type: ivec2 (2 int)
	RL_SHADER_UNIFORM_IVEC3, // Shader uniform type: ivec3 (3 int)
	RL_SHADER_UNIFORM_IVEC4, // Shader uniform type: ivec4 (4 int)
	RL_SHADER_UNIFORM_UINT, // Shader uniform type: unsigned int
	RL_SHADER_UNIFORM_UIVEC2, // Shader uniform type: uivec2 (2 unsigned int)
	RL_SHADER_UNIFORM_UIVEC3, // Shader uniform type: uivec3 (3 unsigned int)
	RL_SHADER_UNIFORM_UIVEC4, // Shader uniform type: uivec4 (4 unsigned int)
	RL_SHADER_UNIFORM_SAMPLER2D // Shader uniform type: sampler2d
} rlShaderUniformDataType;

// Shader attribute data types
typedef enum {
	RL_SHADER_ATTRIB_FLOAT = 0, // Shader attribute type: float
	RL_SHADER_ATTRIB_VEC2, // Shader attribute type: vec2 (2 float)
	RL_SHADER_ATTRIB_VEC3, // Shader attribute type: vec3 (3 float)
	RL_SHADER_ATTRIB_VEC4 // Shader attribute type: vec4 (4 float)
} rlShaderAttributeDataType;

// Framebuffer attachment type
// NOTE: By default up to 8 color channels defined, but it can be more
typedef enum {
	RL_ATTACHMENT_COLOR_CHANNEL0 = 0, // Framebuffer attachment type: color 0
	RL_ATTACHMENT_COLOR_CHANNEL1 = 1, // Framebuffer attachment type: color 1
	RL_ATTACHMENT_COLOR_CHANNEL2 = 2, // Framebuffer attachment type: color 2
	RL_ATTACHMENT_COLOR_CHANNEL3 = 3, // Framebuffer attachment type: color 3
	RL_ATTACHMENT_COLOR_CHANNEL4 = 4, // Framebuffer attachment type: color 4
	RL_ATTACHMENT_COLOR_CHANNEL5 = 5, // Framebuffer attachment type: color 5
	RL_ATTACHMENT_COLOR_CHANNEL6 = 6, // Framebuffer attachment type: color 6
	RL_ATTACHMENT_COLOR_CHANNEL7 = 7, // Framebuffer attachment type: color 7
	RL_ATTACHMENT_DEPTH = 100, // Framebuffer attachment type: depth
	RL_ATTACHMENT_STENCIL = 200, // Framebuffer attachment type: stencil
} rlFramebufferAttachType;

// Framebuffer texture attachment type
typedef enum {
	RL_ATTACHMENT_CUBEMAP_POSITIVE_X = 0, // Framebuffer texture attachment type: cubemap, +X side
	RL_ATTACHMENT_CUBEMAP_NEGATIVE_X = 1, // Framebuffer texture attachment type: cubemap, -X side
	RL_ATTACHMENT_CUBEMAP_POSITIVE_Y = 2, // Framebuffer texture attachment type: cubemap, +Y side
	RL_ATTACHMENT_CUBEMAP_NEGATIVE_Y = 3, // Framebuffer texture attachment type: cubemap, -Y side
	RL_ATTACHMENT_CUBEMAP_POSITIVE_Z = 4, // Framebuffer texture attachment type: cubemap, +Z side
	RL_ATTACHMENT_CUBEMAP_NEGATIVE_Z = 5, // Framebuffer texture attachment type: cubemap, -Z side
	RL_ATTACHMENT_TEXTURE2D = 100, // Framebuffer texture attachment type: texture2d
	RL_ATTACHMENT_RENDERBUFFER = 200, // Framebuffer texture attachment type: renderbuffer
} rlFramebufferAttachTextureType;

// Face culling mode
typedef enum {
	RL_CULL_FACE_FRONT = 0,
	RL_CULL_FACE_BACK
} rlCullMode;

// Dynamic vertex buffers (position + texcoords + colors + indices arrays)
typedef struct rlVertexBuffer {
	int elementCount; // Number of elements in the buffer (QUADS)

	float* vertices; // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
	float* texcoords; // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
	float* normals; // Vertex normal (XYZ - 3 components per vertex) (shader-location = 2)
	unsigned char* colors; // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
#if defined(GRAPHICS_API_OPENGL_11) || defined(GRAPHICS_API_OPENGL_33)
	unsigned int* indices; // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
#if defined(GRAPHICS_API_OPENGL_ES2)
	unsigned short* indices; // Vertex indices (in case vertex data comes indexed) (6 indices per quad)
#endif
	unsigned int vaoId; // OpenGL Vertex Array Object id
	unsigned int vboId[5]; // OpenGL Vertex Buffer Objects id (5 types of vertex data)
} rlVertexBuffer;

typedef struct rlUniformValue {
	union {
		RLMatrix matrixValue;
		float vectorValue[4];
		int integerValue[4];
		unsigned int unsignedValue[4];
	};
	rlShaderUniformDataType uniformType;
	int location;
} rlUniformValue;

// Draw call type
// NOTE: Only texture changes register a new draw, other state-change-related elements are not
// used at this moment (vaoId, shaderId, matrices), raylib just forces a batch draw call if any
// of those state-change happens (this is done in core module)
typedef struct rlDrawCall {
	int mode; // Drawing mode: LINES, TRIANGLES, QUADS
	int vertexCount; // Number of vertex of the draw
	int vertexAlignment; // Number of vertex required for index alignment (LINES, TRIANGLES)
	// unsigned int vaoId;       // Vertex array id to be used on the draw -> Using RLGL.currentBatch->vertexBuffer.vaoId
	// unsigned int shaderId; // Shader id to be used on the draw -> Using RLGL.currentShaderId
	// rlUniformValue* shaderUniforms;
	unsigned int textureId; // Texture id to be used on the draw -> Use to create new draw call if changes

	// Blending variables
	int currentBlendMode; // Blending mode active
	int BlendSrcFactor; // Blending source factor
	int BlendDstFactor; // Blending destination factor
	int BlendEquation; // Blending equation
	int BlendSrcFactorRGB; // Blending source RGB factor
	int BlendDestFactorRGB; // Blending destination RGB factor
	int BlendSrcFactorAlpha; // Blending source alpha factor
	int BlendDestFactorAlpha; // Blending destination alpha factor
	int BlendEquationRGB; // Blending equation for RGB
	int BlendEquationAlpha; // Blending equation for alpha

	// Matrix projection;        // Projection matrix for this draw -> Using RLGL.projection by default
	// Matrix modelview;         // Modelview matrix for this draw -> Using RLGL.modelview by default
} rlDrawCall;

// rlRenderBatch type
typedef struct rlRenderBatch {
	int bufferCount; // Number of vertex buffers (multi-buffering support)
	int currentBuffer; // Current buffer tracking in case of multi-buffering
	rlVertexBuffer* vertexBuffer; // Dynamic buffer(s) for vertex data

	rlDrawCall* draws; // Draw calls array, depends on textureId
	int drawCounter; // Draw calls counter
	float currentDepth; // Current depth value for next draw
	float currentZ; // Current z order added to depth.
} rlRenderBatch;

//------------------------------------------------------------------------------------
// Functions Declaration - RLMatrix operations
//------------------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C" { // Prevents name mangling of functions
#endif

RLAPI void rlMatrixMode(int mode); // Choose the current matrix to be transformed
RLAPI void rlPushMatrix(void); // Push the current matrix to stack
RLAPI void rlPopMatrix(void); // Pop latest inserted matrix from stack
RLAPI void rlLoadIdentity(void); // Reset current matrix to identity matrix
RLAPI void rlTranslatef(float x, float y, float z); // Multiply the current matrix by a translation matrix
RLAPI void rlRotatef(float angle, float x, float y, float z); // Multiply the current matrix by a rotation matrix
RLAPI void rlScalef(float x, float y, float z); // Multiply the current matrix by a scaling matrix
RLAPI void rlMultMatrixf(const float* matf); // Multiply the current matrix by another matrix
RLAPI void rlFrustum(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlOrtho(double left, double right, double bottom, double top, double znear, double zfar);
RLAPI void rlViewport(int x, int y, int width, int height); // Set the viewport area
RLAPI void rlSetClipPlanes(double nearPlane, double farPlane); // Set clip planes distances
RLAPI double rlGetCullDistanceNear(void); // Get cull plane distance near
RLAPI double rlGetCullDistanceFar(void); // Get cull plane distance far

//------------------------------------------------------------------------------------
// Functions Declaration - Vertex level operations
//------------------------------------------------------------------------------------
RLAPI void rlBegin(int mode); // Initialize drawing mode (how to organize vertex)
RLAPI void rlEnd(void); // Finish vertex providing
RLAPI void rlVertex2i(int x, int y); // Define one vertex (position) - 2 int
RLAPI void rlVertex2f(float x, float y); // Define one vertex (position) - 2 float
RLAPI void rlVertex3f(float x, float y, float z); // Define one vertex (position) - 3 float
RLAPI void rlTexCoord2f(float x, float y); // Define one vertex (texture coordinate) - 2 float
RLAPI void rlNormal3f(float x, float y, float z); // Define one vertex (normal) - 3 float
RLAPI void rlColor4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a); // Define one vertex (color) - 4 byte
RLAPI void rlColor3f(float x, float y, float z); // Define one vertex (color) - 3 float
RLAPI void rlColor4f(float x, float y, float z, float w); // Define one vertex (color) - 4 float
RLAPI void rlZDepth(float z); // Set current draw depth for Vertex2(i,f).

//------------------------------------------------------------------------------------
// Functions Declaration - OpenGL style functions (common to 1.1, 3.3+, ES2)
// NOTE: This functions are used to completely abstract raylib code from OpenGL layer,
// some of them are direct wrappers over OpenGL calls, some others are custom
//------------------------------------------------------------------------------------

// Vertex buffers state
RLAPI bool rlEnableVertexArray(unsigned int vaoId); // Enable vertex array (VAO, if supported)
RLAPI void rlDisableVertexArray(void); // Disable vertex array (VAO, if supported)
RLAPI void rlEnableVertexBuffer(unsigned int id); // Enable vertex buffer (VBO)
RLAPI void rlDisableVertexBuffer(void); // Disable vertex buffer (VBO)
RLAPI void rlEnableVertexBufferElement(unsigned int id); // Enable vertex buffer element (VBO element)
RLAPI void rlDisableVertexBufferElement(void); // Disable vertex buffer element (VBO element)
RLAPI void rlEnableVertexAttribute(unsigned int index); // Enable vertex attribute index
RLAPI void rlDisableVertexAttribute(unsigned int index); // Disable vertex attribute index
#if defined(GRAPHICS_API_OPENGL_11)
RLAPI void rlEnableStatePointer(int vertexAttribType, void* buffer); // Enable attribute state pointer
RLAPI void rlDisableStatePointer(int vertexAttribType); // Disable attribute state pointer
#endif

// Textures state
RLAPI void rlActiveTextureSlot(int slot); // Select and active a texture slot
RLAPI void rlEnableTexture(unsigned int id); // Enable texture
RLAPI void rlDisableTexture(void); // Disable texture
RLAPI void rlEnableTextureCubemap(unsigned int id); // Enable texture cubemap
RLAPI void rlDisableTextureCubemap(void); // Disable texture cubemap
RLAPI void rlTextureParameters(unsigned int id, int param, int value); // Set texture parameters (filter, wrap)
RLAPI void rlCubemapParameters(unsigned int id, int param, int value); // Set cubemap parameters (filter, wrap)

// Shader state
RLAPI void rlEnableShader(unsigned int id); // Enable shader program
RLAPI void rlDisableShader(void); // Disable shader program

// Framebuffer state
RLAPI void rlEnableFramebuffer(unsigned int id); // Enable render texture (fbo)
RLAPI void rlDisableFramebuffer(void); // Disable render texture (fbo), return to default framebuffer
RLAPI unsigned int rlGetActiveFramebuffer(void); // Get the currently active render texture (fbo), 0 for default framebuffer
RLAPI void rlActiveDrawBuffers(int count); // Activate multiple draw color buffers
RLAPI void rlBlitFramebuffer(int srcX, int srcY, int srcWidth, int srcHeight, int dstX, int dstY, int dstWidth, int dstHeight, int bufferMask); // Blit active framebuffer to main framebuffer
RLAPI void rlBindFramebuffer(unsigned int target, unsigned int framebuffer); // Bind framebuffer (FBO)

// General render state
RLAPI void rlEnableAdvancedColorBlend(void);
RLAPI void rlDisableAdvancedColorBlend(void);
RLAPI void rlEnableColorBlend(void); // Enable color blending
RLAPI void rlDisableColorBlend(void); // Disable color blending
RLAPI void rlEnableDepthTest(void); // Enable depth test
RLAPI void rlDisableDepthTest(void); // Disable depth test
RLAPI void rlEnableDepthMask(void); // Enable depth write
RLAPI void rlDisableDepthMask(void); // Disable depth write
RLAPI void rlEnableBackfaceCulling(void); // Enable backface culling
RLAPI void rlDisableBackfaceCulling(void); // Disable backface culling
RLAPI void rlColorMask(bool r, bool g, bool b, bool a); // Color mask control
RLAPI void rlSetCullFace(int mode); // Set face culling mode
RLAPI void rlEnableScissorTest(void); // Enable scissor test
RLAPI void rlDisableScissorTest(void); // Disable scissor test
RLAPI void rlScissor(int x, int y, int width, int height); // Scissor test
RLAPI void rlEnableWireMode(void); // Enable wire mode
RLAPI void rlEnablePointMode(void); // Enable point mode
RLAPI void rlDisableWireMode(void); // Disable wire (and point) mode
RLAPI void rlSetLineWidth(float width); // Set the line drawing width
RLAPI float rlGetLineWidth(void); // Get the line drawing width
RLAPI void rlEnableSmoothLines(void); // Enable line aliasing
RLAPI void rlDisableSmoothLines(void); // Disable line aliasing
RLAPI void rlEnableStereoRender(void); // Enable stereo rendering
RLAPI void rlDisableStereoRender(void); // Disable stereo rendering
RLAPI bool rlIsStereoRenderEnabled(void); // Check if stereo render is enabled

RLAPI void rlClearColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a); // Clear color buffer with color
RLAPI void rlClearScreenBuffers(void); // Clear used screen buffers (color and depth)
RLAPI void rlCheckErrors(void); // Check and log OpenGL error codes
RLAPI void rlSetBlendMode(int mode); // Set blending mode
RLAPI void rlSetBlendFactors(int glSrcFactor, int glDstFactor, int glEquation); // Set blending mode factor and equation (using OpenGL factors)
RLAPI void rlSetBlendFactorsSeparate(int glSrcRGB, int glDstRGB, int glSrcAlpha, int glDstAlpha, int glEqRGB, int glEqAlpha); // Set blending mode factors and equations separately (using OpenGL factors)

//------------------------------------------------------------------------------------
// Functions Declaration - rlgl functionality
//------------------------------------------------------------------------------------
// rlgl initialization functions
RLAPI void rlglInit(int width, int height); // Initialize rlgl (buffers, shaders, textures, states)
RLAPI void rlglClose(void); // De-initialize rlgl (buffers, shaders, textures)
RLAPI void rlLoadExtensions(void* loader); // Load OpenGL extensions (loader function required)
RLAPI int rlGetVersion(void); // Get current OpenGL version
RLAPI void rlSetFramebufferWidth(int width); // Set current framebuffer width
RLAPI int rlGetFramebufferWidth(void); // Get default framebuffer width
RLAPI void rlSetFramebufferHeight(int height); // Set current framebuffer height
RLAPI int rlGetFramebufferHeight(void); // Get default framebuffer height

RLAPI unsigned int rlGetTextureIdDefault(void); // Get default texture id
RLAPI unsigned int rlGetShaderIdDefault(void); // Get default shader id
RLAPI int* rlGetShaderLocsDefault(void); // Get default shader locations

// Render batch management
// NOTE: rlgl provides a default render batch to behave like OpenGL 1.1 immediate mode
// but this render batch API is exposed in case of custom batches are required
RLAPI rlRenderBatch rlLoadRenderBatch(int numBuffers, int bufferElements); // Load a render batch system
RLAPI void rlUnloadRenderBatch(rlRenderBatch batch); // Unload render batch system
RLAPI void rlDrawRenderBatch(rlRenderBatch* batch); // Draw render batch data (Update->Draw->Reset)
RLAPI void rlSetRenderBatchActive(rlRenderBatch* batch); // Set the active render batch for rlgl (NULL for default internal)
RLAPI void rlDrawRenderBatchActive(void); // Update and draw internal render batch
RLAPI bool rlCheckRenderBatchLimit(int vCount); // Check internal buffer overflow for a given number of vertex

RLAPI void rlSetTexture(unsigned int id); // Set current texture for render batch and check buffers limits
RLAPI void rlClearActiveTextures();
RLAPI void rlResetDrawDepth();

//------------------------------------------------------------------------------------------------------------------------

// Vertex buffers management
RLAPI unsigned int rlLoadVertexArray(void); // Load vertex array (vao) if supported
RLAPI unsigned int rlLoadVertexBuffer(const void* buffer, int size, bool dynamic); // Load a vertex buffer object
RLAPI unsigned int rlLoadVertexBufferElement(const void* buffer, int size, bool dynamic); // Load vertex buffer elements object
RLAPI void rlUpdateVertexBuffer(unsigned int bufferId, const void* data, int dataSize, int offset); // Update vertex buffer object data on GPU buffer
RLAPI void rlUpdateVertexBufferElements(unsigned int id, const void* data, int dataSize, int offset); // Update vertex buffer elements data on GPU buffer
RLAPI void rlUnloadVertexArray(unsigned int vaoId); // Unload vertex array (vao)
RLAPI void rlUnloadVertexBuffer(unsigned int vboId); // Unload vertex buffer object
RLAPI void rlSetVertexAttribute(unsigned int index, int compSize, int type, bool normalized, int stride, int offset); // Set vertex attribute data configuration
RLAPI void rlSetVertexAttributeDivisor(unsigned int index, int divisor); // Set vertex attribute data divisor
RLAPI void rlSetVertexAttributeDefault(int locIndex, const void* value, int attribType, int count); // Set vertex attribute default value, when attribute to provided
RLAPI void rlDrawVertexArray(int offset, int count); // Draw vertex array (currently active vao)
RLAPI void rlDrawVertexArrayElements(int offset, int count, const void* buffer); // Draw vertex array elements
RLAPI void rlDrawVertexArrayInstanced(int offset, int count, int instances); // Draw vertex array (currently active vao) with instancing
RLAPI void rlDrawVertexArrayElementsInstanced(int offset, int count, const void* buffer, int instances); // Draw vertex array elements with instancing

// Textures management
RLAPI unsigned int rlLoadTexture(const void* data, int width, int height, int format, int mipmapCount); // Load texture data
RLAPI unsigned int rlLoadTextureDepth(int width, int height, bool useRenderBuffer); // Load depth texture/renderbuffer (to be attached to fbo)
RLAPI unsigned int rlLoadTextureCubemap(const void* data, int size, int format, int mipmapCount); // Load texture cubemap data
RLAPI void rlUpdateTexture(unsigned int id, int offsetX, int offsetY, int width, int height, int format, const void* data); // Update texture with new data on GPU
RLAPI void rlGetGlTextureFormats(int format, unsigned int* glInternalFormat, unsigned int* glFormat, unsigned int* glType); // Get OpenGL internal formats
RLAPI const char* rlGetPixelFormatName(unsigned int format); // Get name string for pixel format
RLAPI void rlUnloadTexture(unsigned int id); // Unload texture from GPU memory
RLAPI void rlGenTextureMipmaps(unsigned int id, int width, int height, int format, int* mipmaps); // Generate mipmap data for selected texture
RLAPI void* rlReadTexturePixels(unsigned int id, int width, int height, int format); // Read texture pixel data
RLAPI unsigned char* rlReadScreenPixels(int width, int height); // Read screen pixel data (color buffer)

// Framebuffer management (fbo)
RLAPI unsigned int rlLoadFramebuffer(void); // Load an empty framebuffer
RLAPI void rlFramebufferAttach(unsigned int fboId, unsigned int texId, int attachType, int texType, int mipLevel); // Attach texture/renderbuffer to a framebuffer
RLAPI bool rlFramebufferComplete(unsigned int id); // Verify framebuffer is complete
RLAPI void rlUnloadFramebuffer(unsigned int id); // Delete framebuffer from GPU

// Shaders management
RLAPI unsigned int rlLoadShaderCode(const char* vsCode, const char* fsCode); // Load shader from code strings
RLAPI unsigned int rlCompileShader(const char* shaderCode, int type); // Compile custom shader and return shader id (type: RL_VERTEX_SHADER, RL_FRAGMENT_SHADER, RL_COMPUTE_SHADER)
RLAPI unsigned int rlLoadShaderProgram(unsigned int vShaderId, unsigned int fShaderId); // Load custom shader program
RLAPI void rlUnloadShaderProgram(unsigned int id); // Unload shader program
RLAPI int rlGetLocationUniform(unsigned int shaderId, const char* uniformName); // Get shader location uniform
RLAPI int rlGetLocationUniformCurrent(const char* uniformName);
RLAPI int rlGetLocationAttrib(unsigned int shaderId, const char* attribName); // Get shader location attribute
RLAPI void rlSetUniform(int locIndex, const void* value, int uniformType, int count); // Set shader value uniform
RLAPI void rlSetUniformMatrix(int locIndex, RLMatrix mat); // Set shader value matrix
RLAPI void rlSetUniformMatrices(int locIndex, const RLMatrix* mat, int count); // Set shader value matrices
RLAPI void rlSetUniformSampler(int locIndex, unsigned int textureId); // Set shader value sampler
RLAPI void rlSetShader(unsigned int id, const int* locs); // Set shader currently active (id and locations)
RLAPI unsigned int rlGetShaderCurrent();
// Compute shader management
RLAPI unsigned int rlLoadComputeShaderProgram(unsigned int shaderId); // Load compute shader program
RLAPI void rlComputeShaderDispatch(unsigned int groupX, unsigned int groupY, unsigned int groupZ); // Dispatch compute shader (equivalent to *draw* for graphics pipeline)

// Shader buffer storage object management (ssbo)
RLAPI unsigned int rlLoadShaderBuffer(unsigned int size, const void* data, int usageHint); // Load shader storage buffer object (SSBO)
RLAPI void rlUnloadShaderBuffer(unsigned int ssboId); // Unload shader storage buffer object (SSBO)
RLAPI void rlUpdateShaderBuffer(unsigned int id, const void* data, unsigned int dataSize, unsigned int offset); // Update SSBO buffer data
RLAPI void rlBindShaderBuffer(unsigned int id, unsigned int index); // Bind SSBO buffer
RLAPI void rlReadShaderBuffer(unsigned int id, void* dest, unsigned int count, unsigned int offset); // Read SSBO buffer data (GPU->CPU)
RLAPI void rlCopyShaderBuffer(unsigned int destId, unsigned int srcId, unsigned int destOffset, unsigned int srcOffset, unsigned int count); // Copy SSBO data between buffers
RLAPI unsigned int rlGetShaderBufferSize(unsigned int id); // Get SSBO buffer size

// Buffer management
RLAPI void rlBindImageTexture(unsigned int id, unsigned int index, int format, bool readonly); // Bind image texture

// RLMatrix state management
RLAPI RLMatrix rlGetMatrixModelview(void); // Get internal modelview matrix
RLAPI RLMatrix rlGetMatrixProjection(void); // Get internal projection matrix
RLAPI RLMatrix rlGetMatrixTransform(void); // Get internal accumulated transform matrix
RLAPI RLMatrix rlGetMatrixProjectionStereo(int eye); // Get internal projection matrix for stereo render (selected eye)
RLAPI RLMatrix rlGetMatrixViewOffsetStereo(int eye); // Get internal view offset matrix for stereo render (selected eye)
RLAPI void rlSetMatrixProjection(RLMatrix proj); // Set a custom projection matrix (replaces internal projection matrix)
RLAPI void rlSetMatrixModelview(RLMatrix view); // Set a custom modelview matrix (replaces internal modelview matrix)
RLAPI void rlSetMatrixProjectionStereo(RLMatrix right, RLMatrix left); // Set eyes projection matrices for stereo rendering
RLAPI void rlSetMatrixViewOffsetStereo(RLMatrix right, RLMatrix left); // Set eyes view offsets matrices for stereo rendering

// Quick and dirty cube/quad buffers load->draw->unload
RLAPI void rlLoadDrawCube(void); // Load and draw a cube
RLAPI void rlLoadDrawQuad(void); // Load and draw a quad

#if defined(__cplusplus)
}
#endif

#endif // RLGL_H