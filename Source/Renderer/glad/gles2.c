/**
 * SPDX-License-Identifier: (WTFPL OR CC0-1.0) AND Apache-2.0
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/gles2.h>

#ifndef GLAD_IMPL_UTIL_C_
#define GLAD_IMPL_UTIL_C_

#ifdef _MSC_VER
#define GLAD_IMPL_UTIL_SSCANF sscanf_s
#else
#define GLAD_IMPL_UTIL_SSCANF sscanf
#endif

#endif /* GLAD_IMPL_UTIL_C_ */

#ifdef __cplusplus
extern "C" {
#endif



int GLAD_GL_ES_VERSION_2_0 = 0;
int GLAD_GL_EXT_EGL_image_array = 0;
int GLAD_GL_EXT_EGL_image_storage = 0;
int GLAD_GL_EXT_EGL_image_storage_compression = 0;
int GLAD_GL_EXT_YUV_target = 0;
int GLAD_GL_EXT_base_instance = 0;
int GLAD_GL_EXT_blend_func_extended = 0;
int GLAD_GL_EXT_blend_minmax = 0;
int GLAD_GL_EXT_buffer_storage = 0;
int GLAD_GL_EXT_clear_texture = 0;
int GLAD_GL_EXT_clip_control = 0;
int GLAD_GL_EXT_clip_cull_distance = 0;
int GLAD_GL_EXT_color_buffer_float = 0;
int GLAD_GL_EXT_color_buffer_half_float = 0;
int GLAD_GL_EXT_conservative_depth = 0;
int GLAD_GL_EXT_copy_image = 0;
int GLAD_GL_EXT_debug_label = 0;
int GLAD_GL_EXT_debug_marker = 0;
int GLAD_GL_EXT_depth_clamp = 0;
int GLAD_GL_EXT_discard_framebuffer = 0;
int GLAD_GL_EXT_disjoint_timer_query = 0;
int GLAD_GL_EXT_draw_buffers = 0;
int GLAD_GL_EXT_draw_buffers_indexed = 0;
int GLAD_GL_EXT_draw_elements_base_vertex = 0;
int GLAD_GL_EXT_draw_instanced = 0;
int GLAD_GL_EXT_draw_transform_feedback = 0;
int GLAD_GL_EXT_external_buffer = 0;
int GLAD_GL_EXT_float_blend = 0;
int GLAD_GL_EXT_fragment_shading_rate = 0;
int GLAD_GL_EXT_geometry_point_size = 0;
int GLAD_GL_EXT_geometry_shader = 0;
int GLAD_GL_EXT_gpu_shader5 = 0;
int GLAD_GL_EXT_instanced_arrays = 0;
int GLAD_GL_EXT_map_buffer_range = 0;
int GLAD_GL_EXT_memory_object = 0;
int GLAD_GL_EXT_memory_object_fd = 0;
int GLAD_GL_EXT_memory_object_win32 = 0;
int GLAD_GL_EXT_multi_draw_arrays = 0;
int GLAD_GL_EXT_multi_draw_indirect = 0;
int GLAD_GL_EXT_multisampled_compatibility = 0;
int GLAD_GL_EXT_multisampled_render_to_texture = 0;
int GLAD_GL_EXT_multisampled_render_to_texture2 = 0;
int GLAD_GL_EXT_multiview_draw_buffers = 0;
int GLAD_GL_EXT_multiview_tessellation_geometry_shader = 0;
int GLAD_GL_EXT_multiview_texture_multisample = 0;
int GLAD_GL_EXT_multiview_timer_query = 0;
int GLAD_GL_EXT_occlusion_query_boolean = 0;
int GLAD_GL_EXT_polygon_offset_clamp = 0;
int GLAD_GL_EXT_post_depth_coverage = 0;
int GLAD_GL_EXT_primitive_bounding_box = 0;
int GLAD_GL_EXT_protected_textures = 0;
int GLAD_GL_EXT_pvrtc_sRGB = 0;
int GLAD_GL_EXT_raster_multisample = 0;
int GLAD_GL_EXT_read_format_bgra = 0;
int GLAD_GL_EXT_render_snorm = 0;
int GLAD_GL_EXT_robustness = 0;
int GLAD_GL_EXT_sRGB = 0;
int GLAD_GL_EXT_sRGB_write_control = 0;
int GLAD_GL_EXT_semaphore = 0;
int GLAD_GL_EXT_semaphore_fd = 0;
int GLAD_GL_EXT_semaphore_win32 = 0;
int GLAD_GL_EXT_separate_depth_stencil = 0;
int GLAD_GL_EXT_separate_shader_objects = 0;
int GLAD_GL_EXT_shader_framebuffer_fetch = 0;
int GLAD_GL_EXT_shader_framebuffer_fetch_non_coherent = 0;
int GLAD_GL_EXT_shader_group_vote = 0;
int GLAD_GL_EXT_shader_implicit_conversions = 0;
int GLAD_GL_EXT_shader_integer_mix = 0;
int GLAD_GL_EXT_shader_io_blocks = 0;
int GLAD_GL_EXT_shader_non_constant_global_initializers = 0;
int GLAD_GL_EXT_shader_pixel_local_storage = 0;
int GLAD_GL_EXT_shader_pixel_local_storage2 = 0;
int GLAD_GL_EXT_shader_samples_identical = 0;
int GLAD_GL_EXT_shader_texture_lod = 0;
int GLAD_GL_EXT_shadow_samplers = 0;
int GLAD_GL_EXT_sparse_texture = 0;
int GLAD_GL_EXT_sparse_texture2 = 0;
int GLAD_GL_EXT_tessellation_point_size = 0;
int GLAD_GL_EXT_tessellation_shader = 0;
int GLAD_GL_EXT_texture_border_clamp = 0;
int GLAD_GL_EXT_texture_buffer = 0;
int GLAD_GL_EXT_texture_compression_astc_decode_mode = 0;
int GLAD_GL_EXT_texture_compression_bptc = 0;
int GLAD_GL_EXT_texture_compression_dxt1 = 0;
int GLAD_GL_EXT_texture_compression_rgtc = 0;
int GLAD_GL_EXT_texture_compression_s3tc = 0;
int GLAD_GL_EXT_texture_compression_s3tc_srgb = 0;
int GLAD_GL_EXT_texture_cube_map_array = 0;
int GLAD_GL_EXT_texture_filter_anisotropic = 0;
int GLAD_GL_EXT_texture_filter_minmax = 0;
int GLAD_GL_EXT_texture_format_BGRA8888 = 0;
int GLAD_GL_EXT_texture_format_sRGB_override = 0;
int GLAD_GL_EXT_texture_mirror_clamp_to_edge = 0;
int GLAD_GL_EXT_texture_norm16 = 0;
int GLAD_GL_EXT_texture_query_lod = 0;
int GLAD_GL_EXT_texture_rg = 0;
int GLAD_GL_EXT_texture_sRGB_R8 = 0;
int GLAD_GL_EXT_texture_sRGB_RG8 = 0;
int GLAD_GL_EXT_texture_sRGB_decode = 0;
int GLAD_GL_EXT_texture_shadow_lod = 0;
int GLAD_GL_EXT_texture_storage = 0;
int GLAD_GL_EXT_texture_storage_compression = 0;
int GLAD_GL_EXT_texture_type_2_10_10_10_REV = 0;
int GLAD_GL_EXT_texture_view = 0;
int GLAD_GL_EXT_unpack_subimage = 0;
int GLAD_GL_EXT_win32_keyed_mutex = 0;
int GLAD_GL_EXT_window_rectangles = 0;
int GLAD_GL_KHR_blend_equation_advanced = 0;
int GLAD_GL_KHR_blend_equation_advanced_coherent = 0;
int GLAD_GL_KHR_context_flush_control = 0;
int GLAD_GL_KHR_debug = 0;
int GLAD_GL_KHR_no_error = 0;
int GLAD_GL_KHR_parallel_shader_compile = 0;
int GLAD_GL_KHR_robust_buffer_access_behavior = 0;
int GLAD_GL_KHR_robustness = 0;
int GLAD_GL_KHR_shader_subgroup = 0;
int GLAD_GL_KHR_texture_compression_astc_hdr = 0;
int GLAD_GL_KHR_texture_compression_astc_ldr = 0;
int GLAD_GL_KHR_texture_compression_astc_sliced_3d = 0;
int GLAD_GL_OES_EGL_image = 0;
int GLAD_GL_OES_EGL_image_external = 0;
int GLAD_GL_OES_EGL_image_external_essl3 = 0;
int GLAD_GL_OES_compressed_ETC1_RGB8_sub_texture = 0;
int GLAD_GL_OES_compressed_ETC1_RGB8_texture = 0;
int GLAD_GL_OES_compressed_paletted_texture = 0;
int GLAD_GL_OES_copy_image = 0;
int GLAD_GL_OES_depth24 = 0;
int GLAD_GL_OES_depth32 = 0;
int GLAD_GL_OES_depth_texture = 0;
int GLAD_GL_OES_draw_buffers_indexed = 0;
int GLAD_GL_OES_draw_elements_base_vertex = 0;
int GLAD_GL_OES_element_index_uint = 0;
int GLAD_GL_OES_fbo_render_mipmap = 0;
int GLAD_GL_OES_fragment_precision_high = 0;
int GLAD_GL_OES_geometry_point_size = 0;
int GLAD_GL_OES_geometry_shader = 0;
int GLAD_GL_OES_get_program_binary = 0;
int GLAD_GL_OES_gpu_shader5 = 0;
int GLAD_GL_OES_mapbuffer = 0;
int GLAD_GL_OES_packed_depth_stencil = 0;
int GLAD_GL_OES_primitive_bounding_box = 0;
int GLAD_GL_OES_required_internalformat = 0;
int GLAD_GL_OES_rgb8_rgba8 = 0;
int GLAD_GL_OES_sample_shading = 0;
int GLAD_GL_OES_sample_variables = 0;
int GLAD_GL_OES_shader_image_atomic = 0;
int GLAD_GL_OES_shader_io_blocks = 0;
int GLAD_GL_OES_shader_multisample_interpolation = 0;
int GLAD_GL_OES_standard_derivatives = 0;
int GLAD_GL_OES_stencil1 = 0;
int GLAD_GL_OES_stencil4 = 0;
int GLAD_GL_OES_surfaceless_context = 0;
int GLAD_GL_OES_tessellation_point_size = 0;
int GLAD_GL_OES_tessellation_shader = 0;
int GLAD_GL_OES_texture_3D = 0;
int GLAD_GL_OES_texture_border_clamp = 0;
int GLAD_GL_OES_texture_buffer = 0;
int GLAD_GL_OES_texture_compression_astc = 0;
int GLAD_GL_OES_texture_cube_map_array = 0;
int GLAD_GL_OES_texture_float = 0;
int GLAD_GL_OES_texture_float_linear = 0;
int GLAD_GL_OES_texture_half_float = 0;
int GLAD_GL_OES_texture_half_float_linear = 0;
int GLAD_GL_OES_texture_npot = 0;
int GLAD_GL_OES_texture_stencil8 = 0;
int GLAD_GL_OES_texture_storage_multisample_2d_array = 0;
int GLAD_GL_OES_texture_view = 0;
int GLAD_GL_OES_vertex_array_object = 0;
int GLAD_GL_OES_vertex_half_float = 0;
int GLAD_GL_OES_vertex_type_10_10_10_2 = 0;
int GLAD_GL_OES_viewport_array = 0;



PFNGLACQUIREKEYEDMUTEXWIN32EXTPROC glad_glAcquireKeyedMutexWin32EXT = NULL;
PFNGLACTIVESHADERPROGRAMEXTPROC glad_glActiveShaderProgramEXT = NULL;
PFNGLACTIVETEXTUREPROC glad_glActiveTexture = NULL;
PFNGLATTACHSHADERPROC glad_glAttachShader = NULL;
PFNGLBEGINQUERYEXTPROC glad_glBeginQueryEXT = NULL;
PFNGLBINDATTRIBLOCATIONPROC glad_glBindAttribLocation = NULL;
PFNGLBINDBUFFERPROC glad_glBindBuffer = NULL;
PFNGLBINDFRAGDATALOCATIONEXTPROC glad_glBindFragDataLocationEXT = NULL;
PFNGLBINDFRAGDATALOCATIONINDEXEDEXTPROC glad_glBindFragDataLocationIndexedEXT = NULL;
PFNGLBINDFRAMEBUFFERPROC glad_glBindFramebuffer = NULL;
PFNGLBINDPROGRAMPIPELINEEXTPROC glad_glBindProgramPipelineEXT = NULL;
PFNGLBINDRENDERBUFFERPROC glad_glBindRenderbuffer = NULL;
PFNGLBINDTEXTUREPROC glad_glBindTexture = NULL;
PFNGLBINDVERTEXARRAYOESPROC glad_glBindVertexArrayOES = NULL;
PFNGLBLENDBARRIERKHRPROC glad_glBlendBarrierKHR = NULL;
PFNGLBLENDCOLORPROC glad_glBlendColor = NULL;
PFNGLBLENDEQUATIONPROC glad_glBlendEquation = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC glad_glBlendEquationSeparate = NULL;
PFNGLBLENDEQUATIONSEPARATEIEXTPROC glad_glBlendEquationSeparateiEXT = NULL;
PFNGLBLENDEQUATIONSEPARATEIOESPROC glad_glBlendEquationSeparateiOES = NULL;
PFNGLBLENDEQUATIONIEXTPROC glad_glBlendEquationiEXT = NULL;
PFNGLBLENDEQUATIONIOESPROC glad_glBlendEquationiOES = NULL;
PFNGLBLENDFUNCPROC glad_glBlendFunc = NULL;
PFNGLBLENDFUNCSEPARATEPROC glad_glBlendFuncSeparate = NULL;
PFNGLBLENDFUNCSEPARATEIEXTPROC glad_glBlendFuncSeparateiEXT = NULL;
PFNGLBLENDFUNCSEPARATEIOESPROC glad_glBlendFuncSeparateiOES = NULL;
PFNGLBLENDFUNCIEXTPROC glad_glBlendFunciEXT = NULL;
PFNGLBLENDFUNCIOESPROC glad_glBlendFunciOES = NULL;
PFNGLBUFFERDATAPROC glad_glBufferData = NULL;
PFNGLBUFFERSTORAGEEXTPROC glad_glBufferStorageEXT = NULL;
PFNGLBUFFERSTORAGEEXTERNALEXTPROC glad_glBufferStorageExternalEXT = NULL;
PFNGLBUFFERSTORAGEMEMEXTPROC glad_glBufferStorageMemEXT = NULL;
PFNGLBUFFERSUBDATAPROC glad_glBufferSubData = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus = NULL;
PFNGLCLEARPROC glad_glClear = NULL;
PFNGLCLEARCOLORPROC glad_glClearColor = NULL;
PFNGLCLEARDEPTHFPROC glad_glClearDepthf = NULL;
PFNGLCLEARPIXELLOCALSTORAGEUIEXTPROC glad_glClearPixelLocalStorageuiEXT = NULL;
PFNGLCLEARSTENCILPROC glad_glClearStencil = NULL;
PFNGLCLEARTEXIMAGEEXTPROC glad_glClearTexImageEXT = NULL;
PFNGLCLEARTEXSUBIMAGEEXTPROC glad_glClearTexSubImageEXT = NULL;
PFNGLCLIPCONTROLEXTPROC glad_glClipControlEXT = NULL;
PFNGLCOLORMASKPROC glad_glColorMask = NULL;
PFNGLCOLORMASKIEXTPROC glad_glColorMaskiEXT = NULL;
PFNGLCOLORMASKIOESPROC glad_glColorMaskiOES = NULL;
PFNGLCOMPILESHADERPROC glad_glCompileShader = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC glad_glCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXIMAGE3DOESPROC glad_glCompressedTexImage3DOES = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glad_glCompressedTexSubImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC glad_glCompressedTexSubImage3DOES = NULL;
PFNGLCOPYIMAGESUBDATAEXTPROC glad_glCopyImageSubDataEXT = NULL;
PFNGLCOPYIMAGESUBDATAOESPROC glad_glCopyImageSubDataOES = NULL;
PFNGLCOPYTEXIMAGE2DPROC glad_glCopyTexImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE2DPROC glad_glCopyTexSubImage2D = NULL;
PFNGLCOPYTEXSUBIMAGE3DOESPROC glad_glCopyTexSubImage3DOES = NULL;
PFNGLCREATEMEMORYOBJECTSEXTPROC glad_glCreateMemoryObjectsEXT = NULL;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram = NULL;
PFNGLCREATESHADERPROC glad_glCreateShader = NULL;
PFNGLCREATESHADERPROGRAMVEXTPROC glad_glCreateShaderProgramvEXT = NULL;
PFNGLCULLFACEPROC glad_glCullFace = NULL;
PFNGLDEBUGMESSAGECALLBACKKHRPROC glad_glDebugMessageCallbackKHR = NULL;
PFNGLDEBUGMESSAGECONTROLKHRPROC glad_glDebugMessageControlKHR = NULL;
PFNGLDEBUGMESSAGEINSERTKHRPROC glad_glDebugMessageInsertKHR = NULL;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glad_glDeleteFramebuffers = NULL;
PFNGLDELETEMEMORYOBJECTSEXTPROC glad_glDeleteMemoryObjectsEXT = NULL;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram = NULL;
PFNGLDELETEPROGRAMPIPELINESEXTPROC glad_glDeleteProgramPipelinesEXT = NULL;
PFNGLDELETEQUERIESEXTPROC glad_glDeleteQueriesEXT = NULL;
PFNGLDELETERENDERBUFFERSPROC glad_glDeleteRenderbuffers = NULL;
PFNGLDELETESEMAPHORESEXTPROC glad_glDeleteSemaphoresEXT = NULL;
PFNGLDELETESHADERPROC glad_glDeleteShader = NULL;
PFNGLDELETETEXTURESPROC glad_glDeleteTextures = NULL;
PFNGLDELETEVERTEXARRAYSOESPROC glad_glDeleteVertexArraysOES = NULL;
PFNGLDEPTHFUNCPROC glad_glDepthFunc = NULL;
PFNGLDEPTHMASKPROC glad_glDepthMask = NULL;
PFNGLDEPTHRANGEARRAYFVOESPROC glad_glDepthRangeArrayfvOES = NULL;
PFNGLDEPTHRANGEINDEXEDFOESPROC glad_glDepthRangeIndexedfOES = NULL;
PFNGLDEPTHRANGEFPROC glad_glDepthRangef = NULL;
PFNGLDETACHSHADERPROC glad_glDetachShader = NULL;
PFNGLDISABLEPROC glad_glDisable = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glad_glDisableVertexAttribArray = NULL;
PFNGLDISABLEIEXTPROC glad_glDisableiEXT = NULL;
PFNGLDISABLEIOESPROC glad_glDisableiOES = NULL;
PFNGLDISCARDFRAMEBUFFEREXTPROC glad_glDiscardFramebufferEXT = NULL;
PFNGLDRAWARRAYSPROC glad_glDrawArrays = NULL;
PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEEXTPROC glad_glDrawArraysInstancedBaseInstanceEXT = NULL;
PFNGLDRAWARRAYSINSTANCEDEXTPROC glad_glDrawArraysInstancedEXT = NULL;
PFNGLDRAWBUFFERSEXTPROC glad_glDrawBuffersEXT = NULL;
PFNGLDRAWBUFFERSINDEXEDEXTPROC glad_glDrawBuffersIndexedEXT = NULL;
PFNGLDRAWELEMENTSPROC glad_glDrawElements = NULL;
PFNGLDRAWELEMENTSBASEVERTEXEXTPROC glad_glDrawElementsBaseVertexEXT = NULL;
PFNGLDRAWELEMENTSBASEVERTEXOESPROC glad_glDrawElementsBaseVertexOES = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEEXTPROC glad_glDrawElementsInstancedBaseInstanceEXT = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEEXTPROC glad_glDrawElementsInstancedBaseVertexBaseInstanceEXT = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXEXTPROC glad_glDrawElementsInstancedBaseVertexEXT = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXOESPROC glad_glDrawElementsInstancedBaseVertexOES = NULL;
PFNGLDRAWELEMENTSINSTANCEDEXTPROC glad_glDrawElementsInstancedEXT = NULL;
PFNGLDRAWRANGEELEMENTSBASEVERTEXEXTPROC glad_glDrawRangeElementsBaseVertexEXT = NULL;
PFNGLDRAWRANGEELEMENTSBASEVERTEXOESPROC glad_glDrawRangeElementsBaseVertexOES = NULL;
PFNGLDRAWTRANSFORMFEEDBACKEXTPROC glad_glDrawTransformFeedbackEXT = NULL;
PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDEXTPROC glad_glDrawTransformFeedbackInstancedEXT = NULL;
PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC glad_glEGLImageTargetRenderbufferStorageOES = NULL;
PFNGLEGLIMAGETARGETTEXSTORAGEEXTPROC glad_glEGLImageTargetTexStorageEXT = NULL;
PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glad_glEGLImageTargetTexture2DOES = NULL;
PFNGLEGLIMAGETARGETTEXTURESTORAGEEXTPROC glad_glEGLImageTargetTextureStorageEXT = NULL;
PFNGLENABLEPROC glad_glEnable = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray = NULL;
PFNGLENABLEIEXTPROC glad_glEnableiEXT = NULL;
PFNGLENABLEIOESPROC glad_glEnableiOES = NULL;
PFNGLENDQUERYEXTPROC glad_glEndQueryEXT = NULL;
PFNGLFINISHPROC glad_glFinish = NULL;
PFNGLFLUSHPROC glad_glFlush = NULL;
PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC glad_glFlushMappedBufferRangeEXT = NULL;
PFNGLFRAMEBUFFERFETCHBARRIEREXTPROC glad_glFramebufferFetchBarrierEXT = NULL;
PFNGLFRAMEBUFFERPIXELLOCALSTORAGESIZEEXTPROC glad_glFramebufferPixelLocalStorageSizeEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer = NULL;
PFNGLFRAMEBUFFERSHADINGRATEEXTPROC glad_glFramebufferShadingRateEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glad_glFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glad_glFramebufferTexture2DMultisampleEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE3DOESPROC glad_glFramebufferTexture3DOES = NULL;
PFNGLFRAMEBUFFERTEXTUREEXTPROC glad_glFramebufferTextureEXT = NULL;
PFNGLFRAMEBUFFERTEXTUREOESPROC glad_glFramebufferTextureOES = NULL;
PFNGLFRONTFACEPROC glad_glFrontFace = NULL;
PFNGLGENBUFFERSPROC glad_glGenBuffers = NULL;
PFNGLGENFRAMEBUFFERSPROC glad_glGenFramebuffers = NULL;
PFNGLGENPROGRAMPIPELINESEXTPROC glad_glGenProgramPipelinesEXT = NULL;
PFNGLGENQUERIESEXTPROC glad_glGenQueriesEXT = NULL;
PFNGLGENRENDERBUFFERSPROC glad_glGenRenderbuffers = NULL;
PFNGLGENSEMAPHORESEXTPROC glad_glGenSemaphoresEXT = NULL;
PFNGLGENTEXTURESPROC glad_glGenTextures = NULL;
PFNGLGENVERTEXARRAYSOESPROC glad_glGenVertexArraysOES = NULL;
PFNGLGENERATEMIPMAPPROC glad_glGenerateMipmap = NULL;
PFNGLGETACTIVEATTRIBPROC glad_glGetActiveAttrib = NULL;
PFNGLGETACTIVEUNIFORMPROC glad_glGetActiveUniform = NULL;
PFNGLGETATTACHEDSHADERSPROC glad_glGetAttachedShaders = NULL;
PFNGLGETATTRIBLOCATIONPROC glad_glGetAttribLocation = NULL;
PFNGLGETBOOLEANVPROC glad_glGetBooleanv = NULL;
PFNGLGETBUFFERPARAMETERIVPROC glad_glGetBufferParameteriv = NULL;
PFNGLGETBUFFERPOINTERVOESPROC glad_glGetBufferPointervOES = NULL;
PFNGLGETDEBUGMESSAGELOGKHRPROC glad_glGetDebugMessageLogKHR = NULL;
PFNGLGETERRORPROC glad_glGetError = NULL;
PFNGLGETFLOATI_VOESPROC glad_glGetFloati_vOES = NULL;
PFNGLGETFLOATVPROC glad_glGetFloatv = NULL;
PFNGLGETFRAGDATAINDEXEXTPROC glad_glGetFragDataIndexEXT = NULL;
PFNGLGETFRAGMENTSHADINGRATESEXTPROC glad_glGetFragmentShadingRatesEXT = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC glad_glGetFramebufferAttachmentParameteriv = NULL;
PFNGLGETFRAMEBUFFERPIXELLOCALSTORAGESIZEEXTPROC glad_glGetFramebufferPixelLocalStorageSizeEXT = NULL;
PFNGLGETGRAPHICSRESETSTATUSEXTPROC glad_glGetGraphicsResetStatusEXT = NULL;
PFNGLGETGRAPHICSRESETSTATUSKHRPROC glad_glGetGraphicsResetStatusKHR = NULL;
PFNGLGETINTEGER64VEXTPROC glad_glGetInteger64vEXT = NULL;
PFNGLGETINTEGERI_VEXTPROC glad_glGetIntegeri_vEXT = NULL;
PFNGLGETINTEGERVPROC glad_glGetIntegerv = NULL;
PFNGLGETMEMORYOBJECTPARAMETERIVEXTPROC glad_glGetMemoryObjectParameterivEXT = NULL;
PFNGLGETOBJECTLABELEXTPROC glad_glGetObjectLabelEXT = NULL;
PFNGLGETOBJECTLABELKHRPROC glad_glGetObjectLabelKHR = NULL;
PFNGLGETOBJECTPTRLABELKHRPROC glad_glGetObjectPtrLabelKHR = NULL;
PFNGLGETPOINTERVKHRPROC glad_glGetPointervKHR = NULL;
PFNGLGETPROGRAMBINARYOESPROC glad_glGetProgramBinaryOES = NULL;
PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog = NULL;
PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC glad_glGetProgramPipelineInfoLogEXT = NULL;
PFNGLGETPROGRAMPIPELINEIVEXTPROC glad_glGetProgramPipelineivEXT = NULL;
PFNGLGETPROGRAMRESOURCELOCATIONINDEXEXTPROC glad_glGetProgramResourceLocationIndexEXT = NULL;
PFNGLGETPROGRAMIVPROC glad_glGetProgramiv = NULL;
PFNGLGETQUERYOBJECTI64VEXTPROC glad_glGetQueryObjecti64vEXT = NULL;
PFNGLGETQUERYOBJECTIVEXTPROC glad_glGetQueryObjectivEXT = NULL;
PFNGLGETQUERYOBJECTUI64VEXTPROC glad_glGetQueryObjectui64vEXT = NULL;
PFNGLGETQUERYOBJECTUIVEXTPROC glad_glGetQueryObjectuivEXT = NULL;
PFNGLGETQUERYIVEXTPROC glad_glGetQueryivEXT = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glad_glGetRenderbufferParameteriv = NULL;
PFNGLGETSAMPLERPARAMETERIIVEXTPROC glad_glGetSamplerParameterIivEXT = NULL;
PFNGLGETSAMPLERPARAMETERIIVOESPROC glad_glGetSamplerParameterIivOES = NULL;
PFNGLGETSAMPLERPARAMETERIUIVEXTPROC glad_glGetSamplerParameterIuivEXT = NULL;
PFNGLGETSAMPLERPARAMETERIUIVOESPROC glad_glGetSamplerParameterIuivOES = NULL;
PFNGLGETSEMAPHOREPARAMETERUI64VEXTPROC glad_glGetSemaphoreParameterui64vEXT = NULL;
PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog = NULL;
PFNGLGETSHADERPRECISIONFORMATPROC glad_glGetShaderPrecisionFormat = NULL;
PFNGLGETSHADERSOURCEPROC glad_glGetShaderSource = NULL;
PFNGLGETSHADERIVPROC glad_glGetShaderiv = NULL;
PFNGLGETSTRINGPROC glad_glGetString = NULL;
PFNGLGETTEXPARAMETERIIVEXTPROC glad_glGetTexParameterIivEXT = NULL;
PFNGLGETTEXPARAMETERIIVOESPROC glad_glGetTexParameterIivOES = NULL;
PFNGLGETTEXPARAMETERIUIVEXTPROC glad_glGetTexParameterIuivEXT = NULL;
PFNGLGETTEXPARAMETERIUIVOESPROC glad_glGetTexParameterIuivOES = NULL;
PFNGLGETTEXPARAMETERFVPROC glad_glGetTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIVPROC glad_glGetTexParameteriv = NULL;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation = NULL;
PFNGLGETUNIFORMFVPROC glad_glGetUniformfv = NULL;
PFNGLGETUNIFORMIVPROC glad_glGetUniformiv = NULL;
PFNGLGETUNSIGNEDBYTEI_VEXTPROC glad_glGetUnsignedBytei_vEXT = NULL;
PFNGLGETUNSIGNEDBYTEVEXTPROC glad_glGetUnsignedBytevEXT = NULL;
PFNGLGETVERTEXATTRIBPOINTERVPROC glad_glGetVertexAttribPointerv = NULL;
PFNGLGETVERTEXATTRIBFVPROC glad_glGetVertexAttribfv = NULL;
PFNGLGETVERTEXATTRIBIVPROC glad_glGetVertexAttribiv = NULL;
PFNGLGETNUNIFORMFVEXTPROC glad_glGetnUniformfvEXT = NULL;
PFNGLGETNUNIFORMFVKHRPROC glad_glGetnUniformfvKHR = NULL;
PFNGLGETNUNIFORMIVEXTPROC glad_glGetnUniformivEXT = NULL;
PFNGLGETNUNIFORMIVKHRPROC glad_glGetnUniformivKHR = NULL;
PFNGLGETNUNIFORMUIVKHRPROC glad_glGetnUniformuivKHR = NULL;
PFNGLHINTPROC glad_glHint = NULL;
PFNGLIMPORTMEMORYFDEXTPROC glad_glImportMemoryFdEXT = NULL;
PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC glad_glImportMemoryWin32HandleEXT = NULL;
PFNGLIMPORTMEMORYWIN32NAMEEXTPROC glad_glImportMemoryWin32NameEXT = NULL;
PFNGLIMPORTSEMAPHOREFDEXTPROC glad_glImportSemaphoreFdEXT = NULL;
PFNGLIMPORTSEMAPHOREWIN32HANDLEEXTPROC glad_glImportSemaphoreWin32HandleEXT = NULL;
PFNGLIMPORTSEMAPHOREWIN32NAMEEXTPROC glad_glImportSemaphoreWin32NameEXT = NULL;
PFNGLINSERTEVENTMARKEREXTPROC glad_glInsertEventMarkerEXT = NULL;
PFNGLISBUFFERPROC glad_glIsBuffer = NULL;
PFNGLISENABLEDPROC glad_glIsEnabled = NULL;
PFNGLISENABLEDIEXTPROC glad_glIsEnablediEXT = NULL;
PFNGLISENABLEDIOESPROC glad_glIsEnablediOES = NULL;
PFNGLISFRAMEBUFFERPROC glad_glIsFramebuffer = NULL;
PFNGLISMEMORYOBJECTEXTPROC glad_glIsMemoryObjectEXT = NULL;
PFNGLISPROGRAMPROC glad_glIsProgram = NULL;
PFNGLISPROGRAMPIPELINEEXTPROC glad_glIsProgramPipelineEXT = NULL;
PFNGLISQUERYEXTPROC glad_glIsQueryEXT = NULL;
PFNGLISRENDERBUFFERPROC glad_glIsRenderbuffer = NULL;
PFNGLISSEMAPHOREEXTPROC glad_glIsSemaphoreEXT = NULL;
PFNGLISSHADERPROC glad_glIsShader = NULL;
PFNGLISTEXTUREPROC glad_glIsTexture = NULL;
PFNGLISVERTEXARRAYOESPROC glad_glIsVertexArrayOES = NULL;
PFNGLLABELOBJECTEXTPROC glad_glLabelObjectEXT = NULL;
PFNGLLINEWIDTHPROC glad_glLineWidth = NULL;
PFNGLLINKPROGRAMPROC glad_glLinkProgram = NULL;
PFNGLMAPBUFFEROESPROC glad_glMapBufferOES = NULL;
PFNGLMAPBUFFERRANGEEXTPROC glad_glMapBufferRangeEXT = NULL;
PFNGLMAXSHADERCOMPILERTHREADSKHRPROC glad_glMaxShaderCompilerThreadsKHR = NULL;
PFNGLMEMORYOBJECTPARAMETERIVEXTPROC glad_glMemoryObjectParameterivEXT = NULL;
PFNGLMINSAMPLESHADINGOESPROC glad_glMinSampleShadingOES = NULL;
PFNGLMULTIDRAWARRAYSEXTPROC glad_glMultiDrawArraysEXT = NULL;
PFNGLMULTIDRAWARRAYSINDIRECTEXTPROC glad_glMultiDrawArraysIndirectEXT = NULL;
PFNGLMULTIDRAWELEMENTSBASEVERTEXEXTPROC glad_glMultiDrawElementsBaseVertexEXT = NULL;
PFNGLMULTIDRAWELEMENTSEXTPROC glad_glMultiDrawElementsEXT = NULL;
PFNGLMULTIDRAWELEMENTSINDIRECTEXTPROC glad_glMultiDrawElementsIndirectEXT = NULL;
PFNGLNAMEDBUFFERSTORAGEEXTERNALEXTPROC glad_glNamedBufferStorageExternalEXT = NULL;
PFNGLNAMEDBUFFERSTORAGEMEMEXTPROC glad_glNamedBufferStorageMemEXT = NULL;
PFNGLOBJECTLABELKHRPROC glad_glObjectLabelKHR = NULL;
PFNGLOBJECTPTRLABELKHRPROC glad_glObjectPtrLabelKHR = NULL;
PFNGLPATCHPARAMETERIEXTPROC glad_glPatchParameteriEXT = NULL;
PFNGLPATCHPARAMETERIOESPROC glad_glPatchParameteriOES = NULL;
PFNGLPIXELSTOREIPROC glad_glPixelStorei = NULL;
PFNGLPOLYGONOFFSETPROC glad_glPolygonOffset = NULL;
PFNGLPOLYGONOFFSETCLAMPEXTPROC glad_glPolygonOffsetClampEXT = NULL;
PFNGLPOPDEBUGGROUPKHRPROC glad_glPopDebugGroupKHR = NULL;
PFNGLPOPGROUPMARKEREXTPROC glad_glPopGroupMarkerEXT = NULL;
PFNGLPRIMITIVEBOUNDINGBOXEXTPROC glad_glPrimitiveBoundingBoxEXT = NULL;
PFNGLPRIMITIVEBOUNDINGBOXOESPROC glad_glPrimitiveBoundingBoxOES = NULL;
PFNGLPROGRAMBINARYOESPROC glad_glProgramBinaryOES = NULL;
PFNGLPROGRAMPARAMETERIEXTPROC glad_glProgramParameteriEXT = NULL;
PFNGLPROGRAMUNIFORM1FEXTPROC glad_glProgramUniform1fEXT = NULL;
PFNGLPROGRAMUNIFORM1FVEXTPROC glad_glProgramUniform1fvEXT = NULL;
PFNGLPROGRAMUNIFORM1IEXTPROC glad_glProgramUniform1iEXT = NULL;
PFNGLPROGRAMUNIFORM1IVEXTPROC glad_glProgramUniform1ivEXT = NULL;
PFNGLPROGRAMUNIFORM1UIEXTPROC glad_glProgramUniform1uiEXT = NULL;
PFNGLPROGRAMUNIFORM1UIVEXTPROC glad_glProgramUniform1uivEXT = NULL;
PFNGLPROGRAMUNIFORM2FEXTPROC glad_glProgramUniform2fEXT = NULL;
PFNGLPROGRAMUNIFORM2FVEXTPROC glad_glProgramUniform2fvEXT = NULL;
PFNGLPROGRAMUNIFORM2IEXTPROC glad_glProgramUniform2iEXT = NULL;
PFNGLPROGRAMUNIFORM2IVEXTPROC glad_glProgramUniform2ivEXT = NULL;
PFNGLPROGRAMUNIFORM2UIEXTPROC glad_glProgramUniform2uiEXT = NULL;
PFNGLPROGRAMUNIFORM2UIVEXTPROC glad_glProgramUniform2uivEXT = NULL;
PFNGLPROGRAMUNIFORM3FEXTPROC glad_glProgramUniform3fEXT = NULL;
PFNGLPROGRAMUNIFORM3FVEXTPROC glad_glProgramUniform3fvEXT = NULL;
PFNGLPROGRAMUNIFORM3IEXTPROC glad_glProgramUniform3iEXT = NULL;
PFNGLPROGRAMUNIFORM3IVEXTPROC glad_glProgramUniform3ivEXT = NULL;
PFNGLPROGRAMUNIFORM3UIEXTPROC glad_glProgramUniform3uiEXT = NULL;
PFNGLPROGRAMUNIFORM3UIVEXTPROC glad_glProgramUniform3uivEXT = NULL;
PFNGLPROGRAMUNIFORM4FEXTPROC glad_glProgramUniform4fEXT = NULL;
PFNGLPROGRAMUNIFORM4FVEXTPROC glad_glProgramUniform4fvEXT = NULL;
PFNGLPROGRAMUNIFORM4IEXTPROC glad_glProgramUniform4iEXT = NULL;
PFNGLPROGRAMUNIFORM4IVEXTPROC glad_glProgramUniform4ivEXT = NULL;
PFNGLPROGRAMUNIFORM4UIEXTPROC glad_glProgramUniform4uiEXT = NULL;
PFNGLPROGRAMUNIFORM4UIVEXTPROC glad_glProgramUniform4uivEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC glad_glProgramUniformMatrix2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC glad_glProgramUniformMatrix2x3fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC glad_glProgramUniformMatrix2x4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC glad_glProgramUniformMatrix3fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC glad_glProgramUniformMatrix3x2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC glad_glProgramUniformMatrix3x4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC glad_glProgramUniformMatrix4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC glad_glProgramUniformMatrix4x2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC glad_glProgramUniformMatrix4x3fvEXT = NULL;
PFNGLPUSHDEBUGGROUPKHRPROC glad_glPushDebugGroupKHR = NULL;
PFNGLPUSHGROUPMARKEREXTPROC glad_glPushGroupMarkerEXT = NULL;
PFNGLQUERYCOUNTEREXTPROC glad_glQueryCounterEXT = NULL;
PFNGLRASTERSAMPLESEXTPROC glad_glRasterSamplesEXT = NULL;
PFNGLREADBUFFERINDEXEDEXTPROC glad_glReadBufferIndexedEXT = NULL;
PFNGLREADPIXELSPROC glad_glReadPixels = NULL;
PFNGLREADNPIXELSEXTPROC glad_glReadnPixelsEXT = NULL;
PFNGLREADNPIXELSKHRPROC glad_glReadnPixelsKHR = NULL;
PFNGLRELEASEKEYEDMUTEXWIN32EXTPROC glad_glReleaseKeyedMutexWin32EXT = NULL;
PFNGLRELEASESHADERCOMPILERPROC glad_glReleaseShaderCompiler = NULL;
PFNGLRENDERBUFFERSTORAGEPROC glad_glRenderbufferStorage = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glad_glRenderbufferStorageMultisampleEXT = NULL;
PFNGLSAMPLECOVERAGEPROC glad_glSampleCoverage = NULL;
PFNGLSAMPLERPARAMETERIIVEXTPROC glad_glSamplerParameterIivEXT = NULL;
PFNGLSAMPLERPARAMETERIIVOESPROC glad_glSamplerParameterIivOES = NULL;
PFNGLSAMPLERPARAMETERIUIVEXTPROC glad_glSamplerParameterIuivEXT = NULL;
PFNGLSAMPLERPARAMETERIUIVOESPROC glad_glSamplerParameterIuivOES = NULL;
PFNGLSCISSORPROC glad_glScissor = NULL;
PFNGLSCISSORARRAYVOESPROC glad_glScissorArrayvOES = NULL;
PFNGLSCISSORINDEXEDOESPROC glad_glScissorIndexedOES = NULL;
PFNGLSCISSORINDEXEDVOESPROC glad_glScissorIndexedvOES = NULL;
PFNGLSEMAPHOREPARAMETERUI64VEXTPROC glad_glSemaphoreParameterui64vEXT = NULL;
PFNGLSHADERBINARYPROC glad_glShaderBinary = NULL;
PFNGLSHADERSOURCEPROC glad_glShaderSource = NULL;
PFNGLSHADINGRATECOMBINEROPSEXTPROC glad_glShadingRateCombinerOpsEXT = NULL;
PFNGLSHADINGRATEEXTPROC glad_glShadingRateEXT = NULL;
PFNGLSIGNALSEMAPHOREEXTPROC glad_glSignalSemaphoreEXT = NULL;
PFNGLSTENCILFUNCPROC glad_glStencilFunc = NULL;
PFNGLSTENCILFUNCSEPARATEPROC glad_glStencilFuncSeparate = NULL;
PFNGLSTENCILMASKPROC glad_glStencilMask = NULL;
PFNGLSTENCILMASKSEPARATEPROC glad_glStencilMaskSeparate = NULL;
PFNGLSTENCILOPPROC glad_glStencilOp = NULL;
PFNGLSTENCILOPSEPARATEPROC glad_glStencilOpSeparate = NULL;
PFNGLTEXBUFFEREXTPROC glad_glTexBufferEXT = NULL;
PFNGLTEXBUFFEROESPROC glad_glTexBufferOES = NULL;
PFNGLTEXBUFFERRANGEEXTPROC glad_glTexBufferRangeEXT = NULL;
PFNGLTEXBUFFERRANGEOESPROC glad_glTexBufferRangeOES = NULL;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D = NULL;
PFNGLTEXIMAGE3DOESPROC glad_glTexImage3DOES = NULL;
PFNGLTEXPAGECOMMITMENTEXTPROC glad_glTexPageCommitmentEXT = NULL;
PFNGLTEXPARAMETERIIVEXTPROC glad_glTexParameterIivEXT = NULL;
PFNGLTEXPARAMETERIIVOESPROC glad_glTexParameterIivOES = NULL;
PFNGLTEXPARAMETERIUIVEXTPROC glad_glTexParameterIuivEXT = NULL;
PFNGLTEXPARAMETERIUIVOESPROC glad_glTexParameterIuivOES = NULL;
PFNGLTEXPARAMETERFPROC glad_glTexParameterf = NULL;
PFNGLTEXPARAMETERFVPROC glad_glTexParameterfv = NULL;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri = NULL;
PFNGLTEXPARAMETERIVPROC glad_glTexParameteriv = NULL;
PFNGLTEXSTORAGE1DEXTPROC glad_glTexStorage1DEXT = NULL;
PFNGLTEXSTORAGE2DEXTPROC glad_glTexStorage2DEXT = NULL;
PFNGLTEXSTORAGE3DEXTPROC glad_glTexStorage3DEXT = NULL;
PFNGLTEXSTORAGE3DMULTISAMPLEOESPROC glad_glTexStorage3DMultisampleOES = NULL;
PFNGLTEXSTORAGEATTRIBS2DEXTPROC glad_glTexStorageAttribs2DEXT = NULL;
PFNGLTEXSTORAGEATTRIBS3DEXTPROC glad_glTexStorageAttribs3DEXT = NULL;
PFNGLTEXSTORAGEMEM2DEXTPROC glad_glTexStorageMem2DEXT = NULL;
PFNGLTEXSTORAGEMEM2DMULTISAMPLEEXTPROC glad_glTexStorageMem2DMultisampleEXT = NULL;
PFNGLTEXSTORAGEMEM3DEXTPROC glad_glTexStorageMem3DEXT = NULL;
PFNGLTEXSTORAGEMEM3DMULTISAMPLEEXTPROC glad_glTexStorageMem3DMultisampleEXT = NULL;
PFNGLTEXSUBIMAGE2DPROC glad_glTexSubImage2D = NULL;
PFNGLTEXSUBIMAGE3DOESPROC glad_glTexSubImage3DOES = NULL;
PFNGLTEXTURESTORAGE1DEXTPROC glad_glTextureStorage1DEXT = NULL;
PFNGLTEXTURESTORAGE2DEXTPROC glad_glTextureStorage2DEXT = NULL;
PFNGLTEXTURESTORAGE3DEXTPROC glad_glTextureStorage3DEXT = NULL;
PFNGLTEXTURESTORAGEMEM2DEXTPROC glad_glTextureStorageMem2DEXT = NULL;
PFNGLTEXTURESTORAGEMEM2DMULTISAMPLEEXTPROC glad_glTextureStorageMem2DMultisampleEXT = NULL;
PFNGLTEXTURESTORAGEMEM3DEXTPROC glad_glTextureStorageMem3DEXT = NULL;
PFNGLTEXTURESTORAGEMEM3DMULTISAMPLEEXTPROC glad_glTextureStorageMem3DMultisampleEXT = NULL;
PFNGLTEXTUREVIEWEXTPROC glad_glTextureViewEXT = NULL;
PFNGLTEXTUREVIEWOESPROC glad_glTextureViewOES = NULL;
PFNGLUNIFORM1FPROC glad_glUniform1f = NULL;
PFNGLUNIFORM1FVPROC glad_glUniform1fv = NULL;
PFNGLUNIFORM1IPROC glad_glUniform1i = NULL;
PFNGLUNIFORM1IVPROC glad_glUniform1iv = NULL;
PFNGLUNIFORM2FPROC glad_glUniform2f = NULL;
PFNGLUNIFORM2FVPROC glad_glUniform2fv = NULL;
PFNGLUNIFORM2IPROC glad_glUniform2i = NULL;
PFNGLUNIFORM2IVPROC glad_glUniform2iv = NULL;
PFNGLUNIFORM3FPROC glad_glUniform3f = NULL;
PFNGLUNIFORM3FVPROC glad_glUniform3fv = NULL;
PFNGLUNIFORM3IPROC glad_glUniform3i = NULL;
PFNGLUNIFORM3IVPROC glad_glUniform3iv = NULL;
PFNGLUNIFORM4FPROC glad_glUniform4f = NULL;
PFNGLUNIFORM4FVPROC glad_glUniform4fv = NULL;
PFNGLUNIFORM4IPROC glad_glUniform4i = NULL;
PFNGLUNIFORM4IVPROC glad_glUniform4iv = NULL;
PFNGLUNIFORMMATRIX2FVPROC glad_glUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC glad_glUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv = NULL;
PFNGLUNMAPBUFFEROESPROC glad_glUnmapBufferOES = NULL;
PFNGLUSEPROGRAMPROC glad_glUseProgram = NULL;
PFNGLUSEPROGRAMSTAGESEXTPROC glad_glUseProgramStagesEXT = NULL;
PFNGLVALIDATEPROGRAMPROC glad_glValidateProgram = NULL;
PFNGLVALIDATEPROGRAMPIPELINEEXTPROC glad_glValidateProgramPipelineEXT = NULL;
PFNGLVERTEXATTRIB1FPROC glad_glVertexAttrib1f = NULL;
PFNGLVERTEXATTRIB1FVPROC glad_glVertexAttrib1fv = NULL;
PFNGLVERTEXATTRIB2FPROC glad_glVertexAttrib2f = NULL;
PFNGLVERTEXATTRIB2FVPROC glad_glVertexAttrib2fv = NULL;
PFNGLVERTEXATTRIB3FPROC glad_glVertexAttrib3f = NULL;
PFNGLVERTEXATTRIB3FVPROC glad_glVertexAttrib3fv = NULL;
PFNGLVERTEXATTRIB4FPROC glad_glVertexAttrib4f = NULL;
PFNGLVERTEXATTRIB4FVPROC glad_glVertexAttrib4fv = NULL;
PFNGLVERTEXATTRIBDIVISOREXTPROC glad_glVertexAttribDivisorEXT = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer = NULL;
PFNGLVIEWPORTPROC glad_glViewport = NULL;
PFNGLVIEWPORTARRAYVOESPROC glad_glViewportArrayvOES = NULL;
PFNGLVIEWPORTINDEXEDFOESPROC glad_glViewportIndexedfOES = NULL;
PFNGLVIEWPORTINDEXEDFVOESPROC glad_glViewportIndexedfvOES = NULL;
PFNGLWAITSEMAPHOREEXTPROC glad_glWaitSemaphoreEXT = NULL;
PFNGLWINDOWRECTANGLESEXTPROC glad_glWindowRectanglesEXT = NULL;


static void glad_gl_load_GL_ES_VERSION_2_0( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_ES_VERSION_2_0) return;
    glad_glActiveTexture = (PFNGLACTIVETEXTUREPROC) load(userptr, "glActiveTexture");
    glad_glAttachShader = (PFNGLATTACHSHADERPROC) load(userptr, "glAttachShader");
    glad_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) load(userptr, "glBindAttribLocation");
    glad_glBindBuffer = (PFNGLBINDBUFFERPROC) load(userptr, "glBindBuffer");
    glad_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) load(userptr, "glBindFramebuffer");
    glad_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) load(userptr, "glBindRenderbuffer");
    glad_glBindTexture = (PFNGLBINDTEXTUREPROC) load(userptr, "glBindTexture");
    glad_glBlendColor = (PFNGLBLENDCOLORPROC) load(userptr, "glBlendColor");
    glad_glBlendEquation = (PFNGLBLENDEQUATIONPROC) load(userptr, "glBlendEquation");
    glad_glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC) load(userptr, "glBlendEquationSeparate");
    glad_glBlendFunc = (PFNGLBLENDFUNCPROC) load(userptr, "glBlendFunc");
    glad_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) load(userptr, "glBlendFuncSeparate");
    glad_glBufferData = (PFNGLBUFFERDATAPROC) load(userptr, "glBufferData");
    glad_glBufferSubData = (PFNGLBUFFERSUBDATAPROC) load(userptr, "glBufferSubData");
    glad_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) load(userptr, "glCheckFramebufferStatus");
    glad_glClear = (PFNGLCLEARPROC) load(userptr, "glClear");
    glad_glClearColor = (PFNGLCLEARCOLORPROC) load(userptr, "glClearColor");
    glad_glClearDepthf = (PFNGLCLEARDEPTHFPROC) load(userptr, "glClearDepthf");
    glad_glClearStencil = (PFNGLCLEARSTENCILPROC) load(userptr, "glClearStencil");
    glad_glColorMask = (PFNGLCOLORMASKPROC) load(userptr, "glColorMask");
    glad_glCompileShader = (PFNGLCOMPILESHADERPROC) load(userptr, "glCompileShader");
    glad_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) load(userptr, "glCompressedTexImage2D");
    glad_glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC) load(userptr, "glCompressedTexSubImage2D");
    glad_glCopyTexImage2D = (PFNGLCOPYTEXIMAGE2DPROC) load(userptr, "glCopyTexImage2D");
    glad_glCopyTexSubImage2D = (PFNGLCOPYTEXSUBIMAGE2DPROC) load(userptr, "glCopyTexSubImage2D");
    glad_glCreateProgram = (PFNGLCREATEPROGRAMPROC) load(userptr, "glCreateProgram");
    glad_glCreateShader = (PFNGLCREATESHADERPROC) load(userptr, "glCreateShader");
    glad_glCullFace = (PFNGLCULLFACEPROC) load(userptr, "glCullFace");
    glad_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) load(userptr, "glDeleteBuffers");
    glad_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) load(userptr, "glDeleteFramebuffers");
    glad_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) load(userptr, "glDeleteProgram");
    glad_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) load(userptr, "glDeleteRenderbuffers");
    glad_glDeleteShader = (PFNGLDELETESHADERPROC) load(userptr, "glDeleteShader");
    glad_glDeleteTextures = (PFNGLDELETETEXTURESPROC) load(userptr, "glDeleteTextures");
    glad_glDepthFunc = (PFNGLDEPTHFUNCPROC) load(userptr, "glDepthFunc");
    glad_glDepthMask = (PFNGLDEPTHMASKPROC) load(userptr, "glDepthMask");
    glad_glDepthRangef = (PFNGLDEPTHRANGEFPROC) load(userptr, "glDepthRangef");
    glad_glDetachShader = (PFNGLDETACHSHADERPROC) load(userptr, "glDetachShader");
    glad_glDisable = (PFNGLDISABLEPROC) load(userptr, "glDisable");
    glad_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) load(userptr, "glDisableVertexAttribArray");
    glad_glDrawArrays = (PFNGLDRAWARRAYSPROC) load(userptr, "glDrawArrays");
    glad_glDrawElements = (PFNGLDRAWELEMENTSPROC) load(userptr, "glDrawElements");
    glad_glEnable = (PFNGLENABLEPROC) load(userptr, "glEnable");
    glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) load(userptr, "glEnableVertexAttribArray");
    glad_glFinish = (PFNGLFINISHPROC) load(userptr, "glFinish");
    glad_glFlush = (PFNGLFLUSHPROC) load(userptr, "glFlush");
    glad_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) load(userptr, "glFramebufferRenderbuffer");
    glad_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) load(userptr, "glFramebufferTexture2D");
    glad_glFrontFace = (PFNGLFRONTFACEPROC) load(userptr, "glFrontFace");
    glad_glGenBuffers = (PFNGLGENBUFFERSPROC) load(userptr, "glGenBuffers");
    glad_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) load(userptr, "glGenFramebuffers");
    glad_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) load(userptr, "glGenRenderbuffers");
    glad_glGenTextures = (PFNGLGENTEXTURESPROC) load(userptr, "glGenTextures");
    glad_glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) load(userptr, "glGenerateMipmap");
    glad_glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC) load(userptr, "glGetActiveAttrib");
    glad_glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC) load(userptr, "glGetActiveUniform");
    glad_glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC) load(userptr, "glGetAttachedShaders");
    glad_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) load(userptr, "glGetAttribLocation");
    glad_glGetBooleanv = (PFNGLGETBOOLEANVPROC) load(userptr, "glGetBooleanv");
    glad_glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC) load(userptr, "glGetBufferParameteriv");
    glad_glGetError = (PFNGLGETERRORPROC) load(userptr, "glGetError");
    glad_glGetFloatv = (PFNGLGETFLOATVPROC) load(userptr, "glGetFloatv");
    glad_glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC) load(userptr, "glGetFramebufferAttachmentParameteriv");
    glad_glGetIntegerv = (PFNGLGETINTEGERVPROC) load(userptr, "glGetIntegerv");
    glad_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) load(userptr, "glGetProgramInfoLog");
    glad_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) load(userptr, "glGetProgramiv");
    glad_glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC) load(userptr, "glGetRenderbufferParameteriv");
    glad_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) load(userptr, "glGetShaderInfoLog");
    glad_glGetShaderPrecisionFormat = (PFNGLGETSHADERPRECISIONFORMATPROC) load(userptr, "glGetShaderPrecisionFormat");
    glad_glGetShaderSource = (PFNGLGETSHADERSOURCEPROC) load(userptr, "glGetShaderSource");
    glad_glGetShaderiv = (PFNGLGETSHADERIVPROC) load(userptr, "glGetShaderiv");
    glad_glGetString = (PFNGLGETSTRINGPROC) load(userptr, "glGetString");
    glad_glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC) load(userptr, "glGetTexParameterfv");
    glad_glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC) load(userptr, "glGetTexParameteriv");
    glad_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) load(userptr, "glGetUniformLocation");
    glad_glGetUniformfv = (PFNGLGETUNIFORMFVPROC) load(userptr, "glGetUniformfv");
    glad_glGetUniformiv = (PFNGLGETUNIFORMIVPROC) load(userptr, "glGetUniformiv");
    glad_glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC) load(userptr, "glGetVertexAttribPointerv");
    glad_glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC) load(userptr, "glGetVertexAttribfv");
    glad_glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC) load(userptr, "glGetVertexAttribiv");
    glad_glHint = (PFNGLHINTPROC) load(userptr, "glHint");
    glad_glIsBuffer = (PFNGLISBUFFERPROC) load(userptr, "glIsBuffer");
    glad_glIsEnabled = (PFNGLISENABLEDPROC) load(userptr, "glIsEnabled");
    glad_glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC) load(userptr, "glIsFramebuffer");
    glad_glIsProgram = (PFNGLISPROGRAMPROC) load(userptr, "glIsProgram");
    glad_glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC) load(userptr, "glIsRenderbuffer");
    glad_glIsShader = (PFNGLISSHADERPROC) load(userptr, "glIsShader");
    glad_glIsTexture = (PFNGLISTEXTUREPROC) load(userptr, "glIsTexture");
    glad_glLineWidth = (PFNGLLINEWIDTHPROC) load(userptr, "glLineWidth");
    glad_glLinkProgram = (PFNGLLINKPROGRAMPROC) load(userptr, "glLinkProgram");
    glad_glPixelStorei = (PFNGLPIXELSTOREIPROC) load(userptr, "glPixelStorei");
    glad_glPolygonOffset = (PFNGLPOLYGONOFFSETPROC) load(userptr, "glPolygonOffset");
    glad_glReadPixels = (PFNGLREADPIXELSPROC) load(userptr, "glReadPixels");
    glad_glReleaseShaderCompiler = (PFNGLRELEASESHADERCOMPILERPROC) load(userptr, "glReleaseShaderCompiler");
    glad_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) load(userptr, "glRenderbufferStorage");
    glad_glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC) load(userptr, "glSampleCoverage");
    glad_glScissor = (PFNGLSCISSORPROC) load(userptr, "glScissor");
    glad_glShaderBinary = (PFNGLSHADERBINARYPROC) load(userptr, "glShaderBinary");
    glad_glShaderSource = (PFNGLSHADERSOURCEPROC) load(userptr, "glShaderSource");
    glad_glStencilFunc = (PFNGLSTENCILFUNCPROC) load(userptr, "glStencilFunc");
    glad_glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC) load(userptr, "glStencilFuncSeparate");
    glad_glStencilMask = (PFNGLSTENCILMASKPROC) load(userptr, "glStencilMask");
    glad_glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC) load(userptr, "glStencilMaskSeparate");
    glad_glStencilOp = (PFNGLSTENCILOPPROC) load(userptr, "glStencilOp");
    glad_glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC) load(userptr, "glStencilOpSeparate");
    glad_glTexImage2D = (PFNGLTEXIMAGE2DPROC) load(userptr, "glTexImage2D");
    glad_glTexParameterf = (PFNGLTEXPARAMETERFPROC) load(userptr, "glTexParameterf");
    glad_glTexParameterfv = (PFNGLTEXPARAMETERFVPROC) load(userptr, "glTexParameterfv");
    glad_glTexParameteri = (PFNGLTEXPARAMETERIPROC) load(userptr, "glTexParameteri");
    glad_glTexParameteriv = (PFNGLTEXPARAMETERIVPROC) load(userptr, "glTexParameteriv");
    glad_glTexSubImage2D = (PFNGLTEXSUBIMAGE2DPROC) load(userptr, "glTexSubImage2D");
    glad_glUniform1f = (PFNGLUNIFORM1FPROC) load(userptr, "glUniform1f");
    glad_glUniform1fv = (PFNGLUNIFORM1FVPROC) load(userptr, "glUniform1fv");
    glad_glUniform1i = (PFNGLUNIFORM1IPROC) load(userptr, "glUniform1i");
    glad_glUniform1iv = (PFNGLUNIFORM1IVPROC) load(userptr, "glUniform1iv");
    glad_glUniform2f = (PFNGLUNIFORM2FPROC) load(userptr, "glUniform2f");
    glad_glUniform2fv = (PFNGLUNIFORM2FVPROC) load(userptr, "glUniform2fv");
    glad_glUniform2i = (PFNGLUNIFORM2IPROC) load(userptr, "glUniform2i");
    glad_glUniform2iv = (PFNGLUNIFORM2IVPROC) load(userptr, "glUniform2iv");
    glad_glUniform3f = (PFNGLUNIFORM3FPROC) load(userptr, "glUniform3f");
    glad_glUniform3fv = (PFNGLUNIFORM3FVPROC) load(userptr, "glUniform3fv");
    glad_glUniform3i = (PFNGLUNIFORM3IPROC) load(userptr, "glUniform3i");
    glad_glUniform3iv = (PFNGLUNIFORM3IVPROC) load(userptr, "glUniform3iv");
    glad_glUniform4f = (PFNGLUNIFORM4FPROC) load(userptr, "glUniform4f");
    glad_glUniform4fv = (PFNGLUNIFORM4FVPROC) load(userptr, "glUniform4fv");
    glad_glUniform4i = (PFNGLUNIFORM4IPROC) load(userptr, "glUniform4i");
    glad_glUniform4iv = (PFNGLUNIFORM4IVPROC) load(userptr, "glUniform4iv");
    glad_glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC) load(userptr, "glUniformMatrix2fv");
    glad_glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC) load(userptr, "glUniformMatrix3fv");
    glad_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) load(userptr, "glUniformMatrix4fv");
    glad_glUseProgram = (PFNGLUSEPROGRAMPROC) load(userptr, "glUseProgram");
    glad_glValidateProgram = (PFNGLVALIDATEPROGRAMPROC) load(userptr, "glValidateProgram");
    glad_glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC) load(userptr, "glVertexAttrib1f");
    glad_glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC) load(userptr, "glVertexAttrib1fv");
    glad_glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC) load(userptr, "glVertexAttrib2f");
    glad_glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC) load(userptr, "glVertexAttrib2fv");
    glad_glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC) load(userptr, "glVertexAttrib3f");
    glad_glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC) load(userptr, "glVertexAttrib3fv");
    glad_glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC) load(userptr, "glVertexAttrib4f");
    glad_glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC) load(userptr, "glVertexAttrib4fv");
    glad_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) load(userptr, "glVertexAttribPointer");
    glad_glViewport = (PFNGLVIEWPORTPROC) load(userptr, "glViewport");
}
static void glad_gl_load_GL_EXT_EGL_image_storage( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_EGL_image_storage) return;
    glad_glEGLImageTargetTexStorageEXT = (PFNGLEGLIMAGETARGETTEXSTORAGEEXTPROC) load(userptr, "glEGLImageTargetTexStorageEXT");
    glad_glEGLImageTargetTextureStorageEXT = (PFNGLEGLIMAGETARGETTEXTURESTORAGEEXTPROC) load(userptr, "glEGLImageTargetTextureStorageEXT");
}
static void glad_gl_load_GL_EXT_base_instance( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_base_instance) return;
    glad_glDrawArraysInstancedBaseInstanceEXT = (PFNGLDRAWARRAYSINSTANCEDBASEINSTANCEEXTPROC) load(userptr, "glDrawArraysInstancedBaseInstanceEXT");
    glad_glDrawElementsInstancedBaseInstanceEXT = (PFNGLDRAWELEMENTSINSTANCEDBASEINSTANCEEXTPROC) load(userptr, "glDrawElementsInstancedBaseInstanceEXT");
    glad_glDrawElementsInstancedBaseVertexBaseInstanceEXT = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXBASEINSTANCEEXTPROC) load(userptr, "glDrawElementsInstancedBaseVertexBaseInstanceEXT");
}
static void glad_gl_load_GL_EXT_blend_func_extended( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_blend_func_extended) return;
    glad_glBindFragDataLocationEXT = (PFNGLBINDFRAGDATALOCATIONEXTPROC) load(userptr, "glBindFragDataLocationEXT");
    glad_glBindFragDataLocationIndexedEXT = (PFNGLBINDFRAGDATALOCATIONINDEXEDEXTPROC) load(userptr, "glBindFragDataLocationIndexedEXT");
    glad_glGetFragDataIndexEXT = (PFNGLGETFRAGDATAINDEXEXTPROC) load(userptr, "glGetFragDataIndexEXT");
    glad_glGetProgramResourceLocationIndexEXT = (PFNGLGETPROGRAMRESOURCELOCATIONINDEXEXTPROC) load(userptr, "glGetProgramResourceLocationIndexEXT");
}
static void glad_gl_load_GL_EXT_buffer_storage( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_buffer_storage) return;
    glad_glBufferStorageEXT = (PFNGLBUFFERSTORAGEEXTPROC) load(userptr, "glBufferStorageEXT");
}
static void glad_gl_load_GL_EXT_clear_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_clear_texture) return;
    glad_glClearTexImageEXT = (PFNGLCLEARTEXIMAGEEXTPROC) load(userptr, "glClearTexImageEXT");
    glad_glClearTexSubImageEXT = (PFNGLCLEARTEXSUBIMAGEEXTPROC) load(userptr, "glClearTexSubImageEXT");
}
static void glad_gl_load_GL_EXT_clip_control( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_clip_control) return;
    glad_glClipControlEXT = (PFNGLCLIPCONTROLEXTPROC) load(userptr, "glClipControlEXT");
}
static void glad_gl_load_GL_EXT_copy_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_copy_image) return;
    glad_glCopyImageSubDataEXT = (PFNGLCOPYIMAGESUBDATAEXTPROC) load(userptr, "glCopyImageSubDataEXT");
}
static void glad_gl_load_GL_EXT_debug_label( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_debug_label) return;
    glad_glGetObjectLabelEXT = (PFNGLGETOBJECTLABELEXTPROC) load(userptr, "glGetObjectLabelEXT");
    glad_glLabelObjectEXT = (PFNGLLABELOBJECTEXTPROC) load(userptr, "glLabelObjectEXT");
}
static void glad_gl_load_GL_EXT_debug_marker( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_debug_marker) return;
    glad_glInsertEventMarkerEXT = (PFNGLINSERTEVENTMARKEREXTPROC) load(userptr, "glInsertEventMarkerEXT");
    glad_glPopGroupMarkerEXT = (PFNGLPOPGROUPMARKEREXTPROC) load(userptr, "glPopGroupMarkerEXT");
    glad_glPushGroupMarkerEXT = (PFNGLPUSHGROUPMARKEREXTPROC) load(userptr, "glPushGroupMarkerEXT");
}
static void glad_gl_load_GL_EXT_discard_framebuffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_discard_framebuffer) return;
    glad_glDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC) load(userptr, "glDiscardFramebufferEXT");
}
static void glad_gl_load_GL_EXT_disjoint_timer_query( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_disjoint_timer_query) return;
    glad_glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC) load(userptr, "glBeginQueryEXT");
    glad_glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC) load(userptr, "glDeleteQueriesEXT");
    glad_glEndQueryEXT = (PFNGLENDQUERYEXTPROC) load(userptr, "glEndQueryEXT");
    glad_glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC) load(userptr, "glGenQueriesEXT");
    glad_glGetInteger64vEXT = (PFNGLGETINTEGER64VEXTPROC) load(userptr, "glGetInteger64vEXT");
    glad_glGetQueryObjecti64vEXT = (PFNGLGETQUERYOBJECTI64VEXTPROC) load(userptr, "glGetQueryObjecti64vEXT");
    glad_glGetQueryObjectivEXT = (PFNGLGETQUERYOBJECTIVEXTPROC) load(userptr, "glGetQueryObjectivEXT");
    glad_glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VEXTPROC) load(userptr, "glGetQueryObjectui64vEXT");
    glad_glGetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC) load(userptr, "glGetQueryObjectuivEXT");
    glad_glGetQueryivEXT = (PFNGLGETQUERYIVEXTPROC) load(userptr, "glGetQueryivEXT");
    glad_glIsQueryEXT = (PFNGLISQUERYEXTPROC) load(userptr, "glIsQueryEXT");
    glad_glQueryCounterEXT = (PFNGLQUERYCOUNTEREXTPROC) load(userptr, "glQueryCounterEXT");
}
static void glad_gl_load_GL_EXT_draw_buffers( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_buffers) return;
    glad_glDrawBuffersEXT = (PFNGLDRAWBUFFERSEXTPROC) load(userptr, "glDrawBuffersEXT");
}
static void glad_gl_load_GL_EXT_draw_buffers_indexed( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_buffers_indexed) return;
    glad_glBlendEquationSeparateiEXT = (PFNGLBLENDEQUATIONSEPARATEIEXTPROC) load(userptr, "glBlendEquationSeparateiEXT");
    glad_glBlendEquationiEXT = (PFNGLBLENDEQUATIONIEXTPROC) load(userptr, "glBlendEquationiEXT");
    glad_glBlendFuncSeparateiEXT = (PFNGLBLENDFUNCSEPARATEIEXTPROC) load(userptr, "glBlendFuncSeparateiEXT");
    glad_glBlendFunciEXT = (PFNGLBLENDFUNCIEXTPROC) load(userptr, "glBlendFunciEXT");
    glad_glColorMaskiEXT = (PFNGLCOLORMASKIEXTPROC) load(userptr, "glColorMaskiEXT");
    glad_glDisableiEXT = (PFNGLDISABLEIEXTPROC) load(userptr, "glDisableiEXT");
    glad_glEnableiEXT = (PFNGLENABLEIEXTPROC) load(userptr, "glEnableiEXT");
    glad_glIsEnablediEXT = (PFNGLISENABLEDIEXTPROC) load(userptr, "glIsEnablediEXT");
}
static void glad_gl_load_GL_EXT_draw_elements_base_vertex( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_elements_base_vertex) return;
    glad_glDrawElementsBaseVertexEXT = (PFNGLDRAWELEMENTSBASEVERTEXEXTPROC) load(userptr, "glDrawElementsBaseVertexEXT");
    glad_glDrawElementsInstancedBaseVertexEXT = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXEXTPROC) load(userptr, "glDrawElementsInstancedBaseVertexEXT");
    glad_glDrawRangeElementsBaseVertexEXT = (PFNGLDRAWRANGEELEMENTSBASEVERTEXEXTPROC) load(userptr, "glDrawRangeElementsBaseVertexEXT");
    glad_glMultiDrawElementsBaseVertexEXT = (PFNGLMULTIDRAWELEMENTSBASEVERTEXEXTPROC) load(userptr, "glMultiDrawElementsBaseVertexEXT");
}
static void glad_gl_load_GL_EXT_draw_instanced( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_instanced) return;
    glad_glDrawArraysInstancedEXT = (PFNGLDRAWARRAYSINSTANCEDEXTPROC) load(userptr, "glDrawArraysInstancedEXT");
    glad_glDrawElementsInstancedEXT = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC) load(userptr, "glDrawElementsInstancedEXT");
}
static void glad_gl_load_GL_EXT_draw_transform_feedback( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_draw_transform_feedback) return;
    glad_glDrawTransformFeedbackEXT = (PFNGLDRAWTRANSFORMFEEDBACKEXTPROC) load(userptr, "glDrawTransformFeedbackEXT");
    glad_glDrawTransformFeedbackInstancedEXT = (PFNGLDRAWTRANSFORMFEEDBACKINSTANCEDEXTPROC) load(userptr, "glDrawTransformFeedbackInstancedEXT");
}
static void glad_gl_load_GL_EXT_external_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_external_buffer) return;
    glad_glBufferStorageExternalEXT = (PFNGLBUFFERSTORAGEEXTERNALEXTPROC) load(userptr, "glBufferStorageExternalEXT");
    glad_glNamedBufferStorageExternalEXT = (PFNGLNAMEDBUFFERSTORAGEEXTERNALEXTPROC) load(userptr, "glNamedBufferStorageExternalEXT");
}
static void glad_gl_load_GL_EXT_fragment_shading_rate( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_fragment_shading_rate) return;
    glad_glFramebufferShadingRateEXT = (PFNGLFRAMEBUFFERSHADINGRATEEXTPROC) load(userptr, "glFramebufferShadingRateEXT");
    glad_glGetFragmentShadingRatesEXT = (PFNGLGETFRAGMENTSHADINGRATESEXTPROC) load(userptr, "glGetFragmentShadingRatesEXT");
    glad_glShadingRateCombinerOpsEXT = (PFNGLSHADINGRATECOMBINEROPSEXTPROC) load(userptr, "glShadingRateCombinerOpsEXT");
    glad_glShadingRateEXT = (PFNGLSHADINGRATEEXTPROC) load(userptr, "glShadingRateEXT");
}
static void glad_gl_load_GL_EXT_geometry_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_geometry_shader) return;
    glad_glFramebufferTextureEXT = (PFNGLFRAMEBUFFERTEXTUREEXTPROC) load(userptr, "glFramebufferTextureEXT");
}
static void glad_gl_load_GL_EXT_instanced_arrays( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_instanced_arrays) return;
    glad_glDrawArraysInstancedEXT = (PFNGLDRAWARRAYSINSTANCEDEXTPROC) load(userptr, "glDrawArraysInstancedEXT");
    glad_glDrawElementsInstancedEXT = (PFNGLDRAWELEMENTSINSTANCEDEXTPROC) load(userptr, "glDrawElementsInstancedEXT");
    glad_glVertexAttribDivisorEXT = (PFNGLVERTEXATTRIBDIVISOREXTPROC) load(userptr, "glVertexAttribDivisorEXT");
}
static void glad_gl_load_GL_EXT_map_buffer_range( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_map_buffer_range) return;
    glad_glFlushMappedBufferRangeEXT = (PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC) load(userptr, "glFlushMappedBufferRangeEXT");
    glad_glMapBufferRangeEXT = (PFNGLMAPBUFFERRANGEEXTPROC) load(userptr, "glMapBufferRangeEXT");
}
static void glad_gl_load_GL_EXT_memory_object( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_memory_object) return;
    glad_glBufferStorageMemEXT = (PFNGLBUFFERSTORAGEMEMEXTPROC) load(userptr, "glBufferStorageMemEXT");
    glad_glCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC) load(userptr, "glCreateMemoryObjectsEXT");
    glad_glDeleteMemoryObjectsEXT = (PFNGLDELETEMEMORYOBJECTSEXTPROC) load(userptr, "glDeleteMemoryObjectsEXT");
    glad_glGetMemoryObjectParameterivEXT = (PFNGLGETMEMORYOBJECTPARAMETERIVEXTPROC) load(userptr, "glGetMemoryObjectParameterivEXT");
    glad_glGetUnsignedBytei_vEXT = (PFNGLGETUNSIGNEDBYTEI_VEXTPROC) load(userptr, "glGetUnsignedBytei_vEXT");
    glad_glGetUnsignedBytevEXT = (PFNGLGETUNSIGNEDBYTEVEXTPROC) load(userptr, "glGetUnsignedBytevEXT");
    glad_glIsMemoryObjectEXT = (PFNGLISMEMORYOBJECTEXTPROC) load(userptr, "glIsMemoryObjectEXT");
    glad_glMemoryObjectParameterivEXT = (PFNGLMEMORYOBJECTPARAMETERIVEXTPROC) load(userptr, "glMemoryObjectParameterivEXT");
    glad_glNamedBufferStorageMemEXT = (PFNGLNAMEDBUFFERSTORAGEMEMEXTPROC) load(userptr, "glNamedBufferStorageMemEXT");
    glad_glTexStorageMem2DEXT = (PFNGLTEXSTORAGEMEM2DEXTPROC) load(userptr, "glTexStorageMem2DEXT");
    glad_glTexStorageMem2DMultisampleEXT = (PFNGLTEXSTORAGEMEM2DMULTISAMPLEEXTPROC) load(userptr, "glTexStorageMem2DMultisampleEXT");
    glad_glTexStorageMem3DEXT = (PFNGLTEXSTORAGEMEM3DEXTPROC) load(userptr, "glTexStorageMem3DEXT");
    glad_glTexStorageMem3DMultisampleEXT = (PFNGLTEXSTORAGEMEM3DMULTISAMPLEEXTPROC) load(userptr, "glTexStorageMem3DMultisampleEXT");
    glad_glTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC) load(userptr, "glTextureStorageMem2DEXT");
    glad_glTextureStorageMem2DMultisampleEXT = (PFNGLTEXTURESTORAGEMEM2DMULTISAMPLEEXTPROC) load(userptr, "glTextureStorageMem2DMultisampleEXT");
    glad_glTextureStorageMem3DEXT = (PFNGLTEXTURESTORAGEMEM3DEXTPROC) load(userptr, "glTextureStorageMem3DEXT");
    glad_glTextureStorageMem3DMultisampleEXT = (PFNGLTEXTURESTORAGEMEM3DMULTISAMPLEEXTPROC) load(userptr, "glTextureStorageMem3DMultisampleEXT");
}
static void glad_gl_load_GL_EXT_memory_object_fd( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_memory_object_fd) return;
    glad_glImportMemoryFdEXT = (PFNGLIMPORTMEMORYFDEXTPROC) load(userptr, "glImportMemoryFdEXT");
}
static void glad_gl_load_GL_EXT_memory_object_win32( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_memory_object_win32) return;
    glad_glImportMemoryWin32HandleEXT = (PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC) load(userptr, "glImportMemoryWin32HandleEXT");
    glad_glImportMemoryWin32NameEXT = (PFNGLIMPORTMEMORYWIN32NAMEEXTPROC) load(userptr, "glImportMemoryWin32NameEXT");
}
static void glad_gl_load_GL_EXT_multi_draw_arrays( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multi_draw_arrays) return;
    glad_glMultiDrawArraysEXT = (PFNGLMULTIDRAWARRAYSEXTPROC) load(userptr, "glMultiDrawArraysEXT");
    glad_glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC) load(userptr, "glMultiDrawElementsEXT");
}
static void glad_gl_load_GL_EXT_multi_draw_indirect( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multi_draw_indirect) return;
    glad_glMultiDrawArraysIndirectEXT = (PFNGLMULTIDRAWARRAYSINDIRECTEXTPROC) load(userptr, "glMultiDrawArraysIndirectEXT");
    glad_glMultiDrawElementsIndirectEXT = (PFNGLMULTIDRAWELEMENTSINDIRECTEXTPROC) load(userptr, "glMultiDrawElementsIndirectEXT");
}
static void glad_gl_load_GL_EXT_multisampled_render_to_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multisampled_render_to_texture) return;
    glad_glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC) load(userptr, "glFramebufferTexture2DMultisampleEXT");
    glad_glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC) load(userptr, "glRenderbufferStorageMultisampleEXT");
}
static void glad_gl_load_GL_EXT_multiview_draw_buffers( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_multiview_draw_buffers) return;
    glad_glDrawBuffersIndexedEXT = (PFNGLDRAWBUFFERSINDEXEDEXTPROC) load(userptr, "glDrawBuffersIndexedEXT");
    glad_glGetIntegeri_vEXT = (PFNGLGETINTEGERI_VEXTPROC) load(userptr, "glGetIntegeri_vEXT");
    glad_glReadBufferIndexedEXT = (PFNGLREADBUFFERINDEXEDEXTPROC) load(userptr, "glReadBufferIndexedEXT");
}
static void glad_gl_load_GL_EXT_occlusion_query_boolean( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_occlusion_query_boolean) return;
    glad_glBeginQueryEXT = (PFNGLBEGINQUERYEXTPROC) load(userptr, "glBeginQueryEXT");
    glad_glDeleteQueriesEXT = (PFNGLDELETEQUERIESEXTPROC) load(userptr, "glDeleteQueriesEXT");
    glad_glEndQueryEXT = (PFNGLENDQUERYEXTPROC) load(userptr, "glEndQueryEXT");
    glad_glGenQueriesEXT = (PFNGLGENQUERIESEXTPROC) load(userptr, "glGenQueriesEXT");
    glad_glGetQueryObjectuivEXT = (PFNGLGETQUERYOBJECTUIVEXTPROC) load(userptr, "glGetQueryObjectuivEXT");
    glad_glGetQueryivEXT = (PFNGLGETQUERYIVEXTPROC) load(userptr, "glGetQueryivEXT");
    glad_glIsQueryEXT = (PFNGLISQUERYEXTPROC) load(userptr, "glIsQueryEXT");
}
static void glad_gl_load_GL_EXT_polygon_offset_clamp( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_polygon_offset_clamp) return;
    glad_glPolygonOffsetClampEXT = (PFNGLPOLYGONOFFSETCLAMPEXTPROC) load(userptr, "glPolygonOffsetClampEXT");
}
static void glad_gl_load_GL_EXT_primitive_bounding_box( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_primitive_bounding_box) return;
    glad_glPrimitiveBoundingBoxEXT = (PFNGLPRIMITIVEBOUNDINGBOXEXTPROC) load(userptr, "glPrimitiveBoundingBoxEXT");
}
static void glad_gl_load_GL_EXT_raster_multisample( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_raster_multisample) return;
    glad_glRasterSamplesEXT = (PFNGLRASTERSAMPLESEXTPROC) load(userptr, "glRasterSamplesEXT");
}
static void glad_gl_load_GL_EXT_robustness( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_robustness) return;
    glad_glGetGraphicsResetStatusEXT = (PFNGLGETGRAPHICSRESETSTATUSEXTPROC) load(userptr, "glGetGraphicsResetStatusEXT");
    glad_glGetnUniformfvEXT = (PFNGLGETNUNIFORMFVEXTPROC) load(userptr, "glGetnUniformfvEXT");
    glad_glGetnUniformivEXT = (PFNGLGETNUNIFORMIVEXTPROC) load(userptr, "glGetnUniformivEXT");
    glad_glReadnPixelsEXT = (PFNGLREADNPIXELSEXTPROC) load(userptr, "glReadnPixelsEXT");
}
static void glad_gl_load_GL_EXT_semaphore( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_semaphore) return;
    glad_glDeleteSemaphoresEXT = (PFNGLDELETESEMAPHORESEXTPROC) load(userptr, "glDeleteSemaphoresEXT");
    glad_glGenSemaphoresEXT = (PFNGLGENSEMAPHORESEXTPROC) load(userptr, "glGenSemaphoresEXT");
    glad_glGetSemaphoreParameterui64vEXT = (PFNGLGETSEMAPHOREPARAMETERUI64VEXTPROC) load(userptr, "glGetSemaphoreParameterui64vEXT");
    glad_glGetUnsignedBytei_vEXT = (PFNGLGETUNSIGNEDBYTEI_VEXTPROC) load(userptr, "glGetUnsignedBytei_vEXT");
    glad_glGetUnsignedBytevEXT = (PFNGLGETUNSIGNEDBYTEVEXTPROC) load(userptr, "glGetUnsignedBytevEXT");
    glad_glIsSemaphoreEXT = (PFNGLISSEMAPHOREEXTPROC) load(userptr, "glIsSemaphoreEXT");
    glad_glSemaphoreParameterui64vEXT = (PFNGLSEMAPHOREPARAMETERUI64VEXTPROC) load(userptr, "glSemaphoreParameterui64vEXT");
    glad_glSignalSemaphoreEXT = (PFNGLSIGNALSEMAPHOREEXTPROC) load(userptr, "glSignalSemaphoreEXT");
    glad_glWaitSemaphoreEXT = (PFNGLWAITSEMAPHOREEXTPROC) load(userptr, "glWaitSemaphoreEXT");
}
static void glad_gl_load_GL_EXT_semaphore_fd( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_semaphore_fd) return;
    glad_glImportSemaphoreFdEXT = (PFNGLIMPORTSEMAPHOREFDEXTPROC) load(userptr, "glImportSemaphoreFdEXT");
}
static void glad_gl_load_GL_EXT_semaphore_win32( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_semaphore_win32) return;
    glad_glImportSemaphoreWin32HandleEXT = (PFNGLIMPORTSEMAPHOREWIN32HANDLEEXTPROC) load(userptr, "glImportSemaphoreWin32HandleEXT");
    glad_glImportSemaphoreWin32NameEXT = (PFNGLIMPORTSEMAPHOREWIN32NAMEEXTPROC) load(userptr, "glImportSemaphoreWin32NameEXT");
}
static void glad_gl_load_GL_EXT_separate_shader_objects( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_separate_shader_objects) return;
    glad_glActiveShaderProgramEXT = (PFNGLACTIVESHADERPROGRAMEXTPROC) load(userptr, "glActiveShaderProgramEXT");
    glad_glBindProgramPipelineEXT = (PFNGLBINDPROGRAMPIPELINEEXTPROC) load(userptr, "glBindProgramPipelineEXT");
    glad_glCreateShaderProgramvEXT = (PFNGLCREATESHADERPROGRAMVEXTPROC) load(userptr, "glCreateShaderProgramvEXT");
    glad_glDeleteProgramPipelinesEXT = (PFNGLDELETEPROGRAMPIPELINESEXTPROC) load(userptr, "glDeleteProgramPipelinesEXT");
    glad_glGenProgramPipelinesEXT = (PFNGLGENPROGRAMPIPELINESEXTPROC) load(userptr, "glGenProgramPipelinesEXT");
    glad_glGetProgramPipelineInfoLogEXT = (PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC) load(userptr, "glGetProgramPipelineInfoLogEXT");
    glad_glGetProgramPipelineivEXT = (PFNGLGETPROGRAMPIPELINEIVEXTPROC) load(userptr, "glGetProgramPipelineivEXT");
    glad_glIsProgramPipelineEXT = (PFNGLISPROGRAMPIPELINEEXTPROC) load(userptr, "glIsProgramPipelineEXT");
    glad_glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC) load(userptr, "glProgramParameteriEXT");
    glad_glProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC) load(userptr, "glProgramUniform1fEXT");
    glad_glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC) load(userptr, "glProgramUniform1fvEXT");
    glad_glProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC) load(userptr, "glProgramUniform1iEXT");
    glad_glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC) load(userptr, "glProgramUniform1ivEXT");
    glad_glProgramUniform1uiEXT = (PFNGLPROGRAMUNIFORM1UIEXTPROC) load(userptr, "glProgramUniform1uiEXT");
    glad_glProgramUniform1uivEXT = (PFNGLPROGRAMUNIFORM1UIVEXTPROC) load(userptr, "glProgramUniform1uivEXT");
    glad_glProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC) load(userptr, "glProgramUniform2fEXT");
    glad_glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC) load(userptr, "glProgramUniform2fvEXT");
    glad_glProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC) load(userptr, "glProgramUniform2iEXT");
    glad_glProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC) load(userptr, "glProgramUniform2ivEXT");
    glad_glProgramUniform2uiEXT = (PFNGLPROGRAMUNIFORM2UIEXTPROC) load(userptr, "glProgramUniform2uiEXT");
    glad_glProgramUniform2uivEXT = (PFNGLPROGRAMUNIFORM2UIVEXTPROC) load(userptr, "glProgramUniform2uivEXT");
    glad_glProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC) load(userptr, "glProgramUniform3fEXT");
    glad_glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC) load(userptr, "glProgramUniform3fvEXT");
    glad_glProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC) load(userptr, "glProgramUniform3iEXT");
    glad_glProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC) load(userptr, "glProgramUniform3ivEXT");
    glad_glProgramUniform3uiEXT = (PFNGLPROGRAMUNIFORM3UIEXTPROC) load(userptr, "glProgramUniform3uiEXT");
    glad_glProgramUniform3uivEXT = (PFNGLPROGRAMUNIFORM3UIVEXTPROC) load(userptr, "glProgramUniform3uivEXT");
    glad_glProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC) load(userptr, "glProgramUniform4fEXT");
    glad_glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC) load(userptr, "glProgramUniform4fvEXT");
    glad_glProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC) load(userptr, "glProgramUniform4iEXT");
    glad_glProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC) load(userptr, "glProgramUniform4ivEXT");
    glad_glProgramUniform4uiEXT = (PFNGLPROGRAMUNIFORM4UIEXTPROC) load(userptr, "glProgramUniform4uiEXT");
    glad_glProgramUniform4uivEXT = (PFNGLPROGRAMUNIFORM4UIVEXTPROC) load(userptr, "glProgramUniform4uivEXT");
    glad_glProgramUniformMatrix2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC) load(userptr, "glProgramUniformMatrix2fvEXT");
    glad_glProgramUniformMatrix2x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC) load(userptr, "glProgramUniformMatrix2x3fvEXT");
    glad_glProgramUniformMatrix2x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC) load(userptr, "glProgramUniformMatrix2x4fvEXT");
    glad_glProgramUniformMatrix3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC) load(userptr, "glProgramUniformMatrix3fvEXT");
    glad_glProgramUniformMatrix3x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC) load(userptr, "glProgramUniformMatrix3x2fvEXT");
    glad_glProgramUniformMatrix3x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC) load(userptr, "glProgramUniformMatrix3x4fvEXT");
    glad_glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC) load(userptr, "glProgramUniformMatrix4fvEXT");
    glad_glProgramUniformMatrix4x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC) load(userptr, "glProgramUniformMatrix4x2fvEXT");
    glad_glProgramUniformMatrix4x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC) load(userptr, "glProgramUniformMatrix4x3fvEXT");
    glad_glUseProgramStagesEXT = (PFNGLUSEPROGRAMSTAGESEXTPROC) load(userptr, "glUseProgramStagesEXT");
    glad_glValidateProgramPipelineEXT = (PFNGLVALIDATEPROGRAMPIPELINEEXTPROC) load(userptr, "glValidateProgramPipelineEXT");
}
static void glad_gl_load_GL_EXT_shader_framebuffer_fetch_non_coherent( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_shader_framebuffer_fetch_non_coherent) return;
    glad_glFramebufferFetchBarrierEXT = (PFNGLFRAMEBUFFERFETCHBARRIEREXTPROC) load(userptr, "glFramebufferFetchBarrierEXT");
}
static void glad_gl_load_GL_EXT_shader_pixel_local_storage2( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_shader_pixel_local_storage2) return;
    glad_glClearPixelLocalStorageuiEXT = (PFNGLCLEARPIXELLOCALSTORAGEUIEXTPROC) load(userptr, "glClearPixelLocalStorageuiEXT");
    glad_glFramebufferPixelLocalStorageSizeEXT = (PFNGLFRAMEBUFFERPIXELLOCALSTORAGESIZEEXTPROC) load(userptr, "glFramebufferPixelLocalStorageSizeEXT");
    glad_glGetFramebufferPixelLocalStorageSizeEXT = (PFNGLGETFRAMEBUFFERPIXELLOCALSTORAGESIZEEXTPROC) load(userptr, "glGetFramebufferPixelLocalStorageSizeEXT");
}
static void glad_gl_load_GL_EXT_sparse_texture( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_sparse_texture) return;
    glad_glTexPageCommitmentEXT = (PFNGLTEXPAGECOMMITMENTEXTPROC) load(userptr, "glTexPageCommitmentEXT");
}
static void glad_gl_load_GL_EXT_tessellation_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_tessellation_shader) return;
    glad_glPatchParameteriEXT = (PFNGLPATCHPARAMETERIEXTPROC) load(userptr, "glPatchParameteriEXT");
}
static void glad_gl_load_GL_EXT_texture_border_clamp( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_texture_border_clamp) return;
    glad_glGetSamplerParameterIivEXT = (PFNGLGETSAMPLERPARAMETERIIVEXTPROC) load(userptr, "glGetSamplerParameterIivEXT");
    glad_glGetSamplerParameterIuivEXT = (PFNGLGETSAMPLERPARAMETERIUIVEXTPROC) load(userptr, "glGetSamplerParameterIuivEXT");
    glad_glGetTexParameterIivEXT = (PFNGLGETTEXPARAMETERIIVEXTPROC) load(userptr, "glGetTexParameterIivEXT");
    glad_glGetTexParameterIuivEXT = (PFNGLGETTEXPARAMETERIUIVEXTPROC) load(userptr, "glGetTexParameterIuivEXT");
    glad_glSamplerParameterIivEXT = (PFNGLSAMPLERPARAMETERIIVEXTPROC) load(userptr, "glSamplerParameterIivEXT");
    glad_glSamplerParameterIuivEXT = (PFNGLSAMPLERPARAMETERIUIVEXTPROC) load(userptr, "glSamplerParameterIuivEXT");
    glad_glTexParameterIivEXT = (PFNGLTEXPARAMETERIIVEXTPROC) load(userptr, "glTexParameterIivEXT");
    glad_glTexParameterIuivEXT = (PFNGLTEXPARAMETERIUIVEXTPROC) load(userptr, "glTexParameterIuivEXT");
}
static void glad_gl_load_GL_EXT_texture_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_texture_buffer) return;
    glad_glTexBufferEXT = (PFNGLTEXBUFFEREXTPROC) load(userptr, "glTexBufferEXT");
    glad_glTexBufferRangeEXT = (PFNGLTEXBUFFERRANGEEXTPROC) load(userptr, "glTexBufferRangeEXT");
}
static void glad_gl_load_GL_EXT_texture_storage( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_texture_storage) return;
    glad_glTexStorage1DEXT = (PFNGLTEXSTORAGE1DEXTPROC) load(userptr, "glTexStorage1DEXT");
    glad_glTexStorage2DEXT = (PFNGLTEXSTORAGE2DEXTPROC) load(userptr, "glTexStorage2DEXT");
    glad_glTexStorage3DEXT = (PFNGLTEXSTORAGE3DEXTPROC) load(userptr, "glTexStorage3DEXT");
    glad_glTextureStorage1DEXT = (PFNGLTEXTURESTORAGE1DEXTPROC) load(userptr, "glTextureStorage1DEXT");
    glad_glTextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC) load(userptr, "glTextureStorage2DEXT");
    glad_glTextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC) load(userptr, "glTextureStorage3DEXT");
}
static void glad_gl_load_GL_EXT_texture_storage_compression( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_texture_storage_compression) return;
    glad_glTexStorageAttribs2DEXT = (PFNGLTEXSTORAGEATTRIBS2DEXTPROC) load(userptr, "glTexStorageAttribs2DEXT");
    glad_glTexStorageAttribs3DEXT = (PFNGLTEXSTORAGEATTRIBS3DEXTPROC) load(userptr, "glTexStorageAttribs3DEXT");
}
static void glad_gl_load_GL_EXT_texture_view( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_texture_view) return;
    glad_glTextureViewEXT = (PFNGLTEXTUREVIEWEXTPROC) load(userptr, "glTextureViewEXT");
}
static void glad_gl_load_GL_EXT_win32_keyed_mutex( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_win32_keyed_mutex) return;
    glad_glAcquireKeyedMutexWin32EXT = (PFNGLACQUIREKEYEDMUTEXWIN32EXTPROC) load(userptr, "glAcquireKeyedMutexWin32EXT");
    glad_glReleaseKeyedMutexWin32EXT = (PFNGLRELEASEKEYEDMUTEXWIN32EXTPROC) load(userptr, "glReleaseKeyedMutexWin32EXT");
}
static void glad_gl_load_GL_EXT_window_rectangles( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_EXT_window_rectangles) return;
    glad_glWindowRectanglesEXT = (PFNGLWINDOWRECTANGLESEXTPROC) load(userptr, "glWindowRectanglesEXT");
}
static void glad_gl_load_GL_KHR_blend_equation_advanced( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_KHR_blend_equation_advanced) return;
    glad_glBlendBarrierKHR = (PFNGLBLENDBARRIERKHRPROC) load(userptr, "glBlendBarrierKHR");
}
static void glad_gl_load_GL_KHR_debug( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_KHR_debug) return;
    glad_glDebugMessageCallbackKHR = (PFNGLDEBUGMESSAGECALLBACKKHRPROC) load(userptr, "glDebugMessageCallbackKHR");
    glad_glDebugMessageControlKHR = (PFNGLDEBUGMESSAGECONTROLKHRPROC) load(userptr, "glDebugMessageControlKHR");
    glad_glDebugMessageInsertKHR = (PFNGLDEBUGMESSAGEINSERTKHRPROC) load(userptr, "glDebugMessageInsertKHR");
    glad_glGetDebugMessageLogKHR = (PFNGLGETDEBUGMESSAGELOGKHRPROC) load(userptr, "glGetDebugMessageLogKHR");
    glad_glGetObjectLabelKHR = (PFNGLGETOBJECTLABELKHRPROC) load(userptr, "glGetObjectLabelKHR");
    glad_glGetObjectPtrLabelKHR = (PFNGLGETOBJECTPTRLABELKHRPROC) load(userptr, "glGetObjectPtrLabelKHR");
    glad_glGetPointervKHR = (PFNGLGETPOINTERVKHRPROC) load(userptr, "glGetPointervKHR");
    glad_glObjectLabelKHR = (PFNGLOBJECTLABELKHRPROC) load(userptr, "glObjectLabelKHR");
    glad_glObjectPtrLabelKHR = (PFNGLOBJECTPTRLABELKHRPROC) load(userptr, "glObjectPtrLabelKHR");
    glad_glPopDebugGroupKHR = (PFNGLPOPDEBUGGROUPKHRPROC) load(userptr, "glPopDebugGroupKHR");
    glad_glPushDebugGroupKHR = (PFNGLPUSHDEBUGGROUPKHRPROC) load(userptr, "glPushDebugGroupKHR");
}
static void glad_gl_load_GL_KHR_parallel_shader_compile( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_KHR_parallel_shader_compile) return;
    glad_glMaxShaderCompilerThreadsKHR = (PFNGLMAXSHADERCOMPILERTHREADSKHRPROC) load(userptr, "glMaxShaderCompilerThreadsKHR");
}
static void glad_gl_load_GL_KHR_robustness( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_KHR_robustness) return;
    glad_glGetGraphicsResetStatusKHR = (PFNGLGETGRAPHICSRESETSTATUSKHRPROC) load(userptr, "glGetGraphicsResetStatusKHR");
    glad_glGetnUniformfvKHR = (PFNGLGETNUNIFORMFVKHRPROC) load(userptr, "glGetnUniformfvKHR");
    glad_glGetnUniformivKHR = (PFNGLGETNUNIFORMIVKHRPROC) load(userptr, "glGetnUniformivKHR");
    glad_glGetnUniformuivKHR = (PFNGLGETNUNIFORMUIVKHRPROC) load(userptr, "glGetnUniformuivKHR");
    glad_glReadnPixelsKHR = (PFNGLREADNPIXELSKHRPROC) load(userptr, "glReadnPixelsKHR");
}
static void glad_gl_load_GL_OES_EGL_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_EGL_image) return;
    glad_glEGLImageTargetRenderbufferStorageOES = (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC) load(userptr, "glEGLImageTargetRenderbufferStorageOES");
    glad_glEGLImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) load(userptr, "glEGLImageTargetTexture2DOES");
}
static void glad_gl_load_GL_OES_copy_image( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_copy_image) return;
    glad_glCopyImageSubDataOES = (PFNGLCOPYIMAGESUBDATAOESPROC) load(userptr, "glCopyImageSubDataOES");
}
static void glad_gl_load_GL_OES_draw_buffers_indexed( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_draw_buffers_indexed) return;
    glad_glBlendEquationSeparateiOES = (PFNGLBLENDEQUATIONSEPARATEIOESPROC) load(userptr, "glBlendEquationSeparateiOES");
    glad_glBlendEquationiOES = (PFNGLBLENDEQUATIONIOESPROC) load(userptr, "glBlendEquationiOES");
    glad_glBlendFuncSeparateiOES = (PFNGLBLENDFUNCSEPARATEIOESPROC) load(userptr, "glBlendFuncSeparateiOES");
    glad_glBlendFunciOES = (PFNGLBLENDFUNCIOESPROC) load(userptr, "glBlendFunciOES");
    glad_glColorMaskiOES = (PFNGLCOLORMASKIOESPROC) load(userptr, "glColorMaskiOES");
    glad_glDisableiOES = (PFNGLDISABLEIOESPROC) load(userptr, "glDisableiOES");
    glad_glEnableiOES = (PFNGLENABLEIOESPROC) load(userptr, "glEnableiOES");
    glad_glIsEnablediOES = (PFNGLISENABLEDIOESPROC) load(userptr, "glIsEnablediOES");
}
static void glad_gl_load_GL_OES_draw_elements_base_vertex( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_draw_elements_base_vertex) return;
    glad_glDrawElementsBaseVertexOES = (PFNGLDRAWELEMENTSBASEVERTEXOESPROC) load(userptr, "glDrawElementsBaseVertexOES");
    glad_glDrawElementsInstancedBaseVertexOES = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXOESPROC) load(userptr, "glDrawElementsInstancedBaseVertexOES");
    glad_glDrawRangeElementsBaseVertexOES = (PFNGLDRAWRANGEELEMENTSBASEVERTEXOESPROC) load(userptr, "glDrawRangeElementsBaseVertexOES");
    glad_glMultiDrawElementsBaseVertexEXT = (PFNGLMULTIDRAWELEMENTSBASEVERTEXEXTPROC) load(userptr, "glMultiDrawElementsBaseVertexEXT");
}
static void glad_gl_load_GL_OES_geometry_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_geometry_shader) return;
    glad_glFramebufferTextureOES = (PFNGLFRAMEBUFFERTEXTUREOESPROC) load(userptr, "glFramebufferTextureOES");
}
static void glad_gl_load_GL_OES_get_program_binary( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_get_program_binary) return;
    glad_glGetProgramBinaryOES = (PFNGLGETPROGRAMBINARYOESPROC) load(userptr, "glGetProgramBinaryOES");
    glad_glProgramBinaryOES = (PFNGLPROGRAMBINARYOESPROC) load(userptr, "glProgramBinaryOES");
}
static void glad_gl_load_GL_OES_mapbuffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_mapbuffer) return;
    glad_glGetBufferPointervOES = (PFNGLGETBUFFERPOINTERVOESPROC) load(userptr, "glGetBufferPointervOES");
    glad_glMapBufferOES = (PFNGLMAPBUFFEROESPROC) load(userptr, "glMapBufferOES");
    glad_glUnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC) load(userptr, "glUnmapBufferOES");
}
static void glad_gl_load_GL_OES_primitive_bounding_box( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_primitive_bounding_box) return;
    glad_glPrimitiveBoundingBoxOES = (PFNGLPRIMITIVEBOUNDINGBOXOESPROC) load(userptr, "glPrimitiveBoundingBoxOES");
}
static void glad_gl_load_GL_OES_sample_shading( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_sample_shading) return;
    glad_glMinSampleShadingOES = (PFNGLMINSAMPLESHADINGOESPROC) load(userptr, "glMinSampleShadingOES");
}
static void glad_gl_load_GL_OES_tessellation_shader( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_tessellation_shader) return;
    glad_glPatchParameteriOES = (PFNGLPATCHPARAMETERIOESPROC) load(userptr, "glPatchParameteriOES");
}
static void glad_gl_load_GL_OES_texture_3D( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_3D) return;
    glad_glCompressedTexImage3DOES = (PFNGLCOMPRESSEDTEXIMAGE3DOESPROC) load(userptr, "glCompressedTexImage3DOES");
    glad_glCompressedTexSubImage3DOES = (PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC) load(userptr, "glCompressedTexSubImage3DOES");
    glad_glCopyTexSubImage3DOES = (PFNGLCOPYTEXSUBIMAGE3DOESPROC) load(userptr, "glCopyTexSubImage3DOES");
    glad_glFramebufferTexture3DOES = (PFNGLFRAMEBUFFERTEXTURE3DOESPROC) load(userptr, "glFramebufferTexture3DOES");
    glad_glTexImage3DOES = (PFNGLTEXIMAGE3DOESPROC) load(userptr, "glTexImage3DOES");
    glad_glTexSubImage3DOES = (PFNGLTEXSUBIMAGE3DOESPROC) load(userptr, "glTexSubImage3DOES");
}
static void glad_gl_load_GL_OES_texture_border_clamp( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_border_clamp) return;
    glad_glGetSamplerParameterIivOES = (PFNGLGETSAMPLERPARAMETERIIVOESPROC) load(userptr, "glGetSamplerParameterIivOES");
    glad_glGetSamplerParameterIuivOES = (PFNGLGETSAMPLERPARAMETERIUIVOESPROC) load(userptr, "glGetSamplerParameterIuivOES");
    glad_glGetTexParameterIivOES = (PFNGLGETTEXPARAMETERIIVOESPROC) load(userptr, "glGetTexParameterIivOES");
    glad_glGetTexParameterIuivOES = (PFNGLGETTEXPARAMETERIUIVOESPROC) load(userptr, "glGetTexParameterIuivOES");
    glad_glSamplerParameterIivOES = (PFNGLSAMPLERPARAMETERIIVOESPROC) load(userptr, "glSamplerParameterIivOES");
    glad_glSamplerParameterIuivOES = (PFNGLSAMPLERPARAMETERIUIVOESPROC) load(userptr, "glSamplerParameterIuivOES");
    glad_glTexParameterIivOES = (PFNGLTEXPARAMETERIIVOESPROC) load(userptr, "glTexParameterIivOES");
    glad_glTexParameterIuivOES = (PFNGLTEXPARAMETERIUIVOESPROC) load(userptr, "glTexParameterIuivOES");
}
static void glad_gl_load_GL_OES_texture_buffer( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_buffer) return;
    glad_glTexBufferOES = (PFNGLTEXBUFFEROESPROC) load(userptr, "glTexBufferOES");
    glad_glTexBufferRangeOES = (PFNGLTEXBUFFERRANGEOESPROC) load(userptr, "glTexBufferRangeOES");
}
static void glad_gl_load_GL_OES_texture_storage_multisample_2d_array( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_storage_multisample_2d_array) return;
    glad_glTexStorage3DMultisampleOES = (PFNGLTEXSTORAGE3DMULTISAMPLEOESPROC) load(userptr, "glTexStorage3DMultisampleOES");
}
static void glad_gl_load_GL_OES_texture_view( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_texture_view) return;
    glad_glTextureViewOES = (PFNGLTEXTUREVIEWOESPROC) load(userptr, "glTextureViewOES");
}
static void glad_gl_load_GL_OES_vertex_array_object( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_vertex_array_object) return;
    glad_glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC) load(userptr, "glBindVertexArrayOES");
    glad_glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC) load(userptr, "glDeleteVertexArraysOES");
    glad_glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC) load(userptr, "glGenVertexArraysOES");
    glad_glIsVertexArrayOES = (PFNGLISVERTEXARRAYOESPROC) load(userptr, "glIsVertexArrayOES");
}
static void glad_gl_load_GL_OES_viewport_array( GLADuserptrloadfunc load, void* userptr) {
    if(!GLAD_GL_OES_viewport_array) return;
    glad_glDepthRangeArrayfvOES = (PFNGLDEPTHRANGEARRAYFVOESPROC) load(userptr, "glDepthRangeArrayfvOES");
    glad_glDepthRangeIndexedfOES = (PFNGLDEPTHRANGEINDEXEDFOESPROC) load(userptr, "glDepthRangeIndexedfOES");
    glad_glDisableiOES = (PFNGLDISABLEIOESPROC) load(userptr, "glDisableiOES");
    glad_glEnableiOES = (PFNGLENABLEIOESPROC) load(userptr, "glEnableiOES");
    glad_glGetFloati_vOES = (PFNGLGETFLOATI_VOESPROC) load(userptr, "glGetFloati_vOES");
    glad_glIsEnablediOES = (PFNGLISENABLEDIOESPROC) load(userptr, "glIsEnablediOES");
    glad_glScissorArrayvOES = (PFNGLSCISSORARRAYVOESPROC) load(userptr, "glScissorArrayvOES");
    glad_glScissorIndexedOES = (PFNGLSCISSORINDEXEDOESPROC) load(userptr, "glScissorIndexedOES");
    glad_glScissorIndexedvOES = (PFNGLSCISSORINDEXEDVOESPROC) load(userptr, "glScissorIndexedvOES");
    glad_glViewportArrayvOES = (PFNGLVIEWPORTARRAYVOESPROC) load(userptr, "glViewportArrayvOES");
    glad_glViewportIndexedfOES = (PFNGLVIEWPORTINDEXEDFOESPROC) load(userptr, "glViewportIndexedfOES");
    glad_glViewportIndexedfvOES = (PFNGLVIEWPORTINDEXEDFVOESPROC) load(userptr, "glViewportIndexedfvOES");
}



static void glad_gl_free_extensions(char **exts_i) {
    if (exts_i != NULL) {
        unsigned int index;
        for(index = 0; exts_i[index]; index++) {
            free((void *) (exts_i[index]));
        }
        free((void *)exts_i);
        exts_i = NULL;
    }
}
static int glad_gl_get_extensions( const char **out_exts, char ***out_exts_i) {
#if defined(GL_ES_VERSION_3_0) || defined(GL_VERSION_3_0)
    if (glad_glGetStringi != NULL && glad_glGetIntegerv != NULL) {
        unsigned int index = 0;
        unsigned int num_exts_i = 0;
        char **exts_i = NULL;
        glad_glGetIntegerv(GL_NUM_EXTENSIONS, (int*) &num_exts_i);
        exts_i = (char **) malloc((num_exts_i + 1) * (sizeof *exts_i));
        if (exts_i == NULL) {
            return 0;
        }
        for(index = 0; index < num_exts_i; index++) {
            const char *gl_str_tmp = (const char*) glad_glGetStringi(GL_EXTENSIONS, index);
            size_t len = strlen(gl_str_tmp) + 1;

            char *local_str = (char*) malloc(len * sizeof(char));
            if(local_str == NULL) {
                exts_i[index] = NULL;
                glad_gl_free_extensions(exts_i);
                return 0;
            }

            memcpy(local_str, gl_str_tmp, len * sizeof(char));
            exts_i[index] = local_str;
        }
        exts_i[index] = NULL;

        *out_exts_i = exts_i;

        return 1;
    }
#else
    GLAD_UNUSED(out_exts_i);
#endif
    if (glad_glGetString == NULL) {
        return 0;
    }
    *out_exts = (const char *)glad_glGetString(GL_EXTENSIONS);
    return 1;
}
static int glad_gl_has_extension(const char *exts, char **exts_i, const char *ext) {
    if(exts_i) {
        unsigned int index;
        for(index = 0; exts_i[index]; index++) {
            const char *e = exts_i[index];
            if(strcmp(e, ext) == 0) {
                return 1;
            }
        }
    } else {
        const char *extensions;
        const char *loc;
        const char *terminator;
        extensions = exts;
        if(extensions == NULL || ext == NULL) {
            return 0;
        }
        while(1) {
            loc = strstr(extensions, ext);
            if(loc == NULL) {
                return 0;
            }
            terminator = loc + strlen(ext);
            if((loc == extensions || *(loc - 1) == ' ') &&
                (*terminator == ' ' || *terminator == '\0')) {
                return 1;
            }
            extensions = terminator;
        }
    }
    return 0;
}

static GLADapiproc glad_gl_get_proc_from_userptr(void *userptr, const char* name) {
    return (GLAD_GNUC_EXTENSION (GLADapiproc (*)(const char *name)) userptr)(name);
}

static int glad_gl_find_extensions_gles2(void) {
    const char *exts = NULL;
    char **exts_i = NULL;
    if (!glad_gl_get_extensions(&exts, &exts_i)) return 0;

    GLAD_GL_EXT_EGL_image_array = glad_gl_has_extension(exts, exts_i, "GL_EXT_EGL_image_array");
    GLAD_GL_EXT_EGL_image_storage = glad_gl_has_extension(exts, exts_i, "GL_EXT_EGL_image_storage");
    GLAD_GL_EXT_EGL_image_storage_compression = glad_gl_has_extension(exts, exts_i, "GL_EXT_EGL_image_storage_compression");
    GLAD_GL_EXT_YUV_target = glad_gl_has_extension(exts, exts_i, "GL_EXT_YUV_target");
    GLAD_GL_EXT_base_instance = glad_gl_has_extension(exts, exts_i, "GL_EXT_base_instance");
    GLAD_GL_EXT_blend_func_extended = glad_gl_has_extension(exts, exts_i, "GL_EXT_blend_func_extended");
    GLAD_GL_EXT_blend_minmax = glad_gl_has_extension(exts, exts_i, "GL_EXT_blend_minmax");
    GLAD_GL_EXT_buffer_storage = glad_gl_has_extension(exts, exts_i, "GL_EXT_buffer_storage");
    GLAD_GL_EXT_clear_texture = glad_gl_has_extension(exts, exts_i, "GL_EXT_clear_texture");
    GLAD_GL_EXT_clip_control = glad_gl_has_extension(exts, exts_i, "GL_EXT_clip_control");
    GLAD_GL_EXT_clip_cull_distance = glad_gl_has_extension(exts, exts_i, "GL_EXT_clip_cull_distance");
    GLAD_GL_EXT_color_buffer_float = glad_gl_has_extension(exts, exts_i, "GL_EXT_color_buffer_float");
    GLAD_GL_EXT_color_buffer_half_float = glad_gl_has_extension(exts, exts_i, "GL_EXT_color_buffer_half_float");
    GLAD_GL_EXT_conservative_depth = glad_gl_has_extension(exts, exts_i, "GL_EXT_conservative_depth");
    GLAD_GL_EXT_copy_image = glad_gl_has_extension(exts, exts_i, "GL_EXT_copy_image");
    GLAD_GL_EXT_debug_label = glad_gl_has_extension(exts, exts_i, "GL_EXT_debug_label");
    GLAD_GL_EXT_debug_marker = glad_gl_has_extension(exts, exts_i, "GL_EXT_debug_marker");
    GLAD_GL_EXT_depth_clamp = glad_gl_has_extension(exts, exts_i, "GL_EXT_depth_clamp");
    GLAD_GL_EXT_discard_framebuffer = glad_gl_has_extension(exts, exts_i, "GL_EXT_discard_framebuffer");
    GLAD_GL_EXT_disjoint_timer_query = glad_gl_has_extension(exts, exts_i, "GL_EXT_disjoint_timer_query");
    GLAD_GL_EXT_draw_buffers = glad_gl_has_extension(exts, exts_i, "GL_EXT_draw_buffers");
    GLAD_GL_EXT_draw_buffers_indexed = glad_gl_has_extension(exts, exts_i, "GL_EXT_draw_buffers_indexed");
    GLAD_GL_EXT_draw_elements_base_vertex = glad_gl_has_extension(exts, exts_i, "GL_EXT_draw_elements_base_vertex");
    GLAD_GL_EXT_draw_instanced = glad_gl_has_extension(exts, exts_i, "GL_EXT_draw_instanced");
    GLAD_GL_EXT_draw_transform_feedback = glad_gl_has_extension(exts, exts_i, "GL_EXT_draw_transform_feedback");
    GLAD_GL_EXT_external_buffer = glad_gl_has_extension(exts, exts_i, "GL_EXT_external_buffer");
    GLAD_GL_EXT_float_blend = glad_gl_has_extension(exts, exts_i, "GL_EXT_float_blend");
    GLAD_GL_EXT_fragment_shading_rate = glad_gl_has_extension(exts, exts_i, "GL_EXT_fragment_shading_rate");
    GLAD_GL_EXT_geometry_point_size = glad_gl_has_extension(exts, exts_i, "GL_EXT_geometry_point_size");
    GLAD_GL_EXT_geometry_shader = glad_gl_has_extension(exts, exts_i, "GL_EXT_geometry_shader");
    GLAD_GL_EXT_gpu_shader5 = glad_gl_has_extension(exts, exts_i, "GL_EXT_gpu_shader5");
    GLAD_GL_EXT_instanced_arrays = glad_gl_has_extension(exts, exts_i, "GL_EXT_instanced_arrays");
    GLAD_GL_EXT_map_buffer_range = glad_gl_has_extension(exts, exts_i, "GL_EXT_map_buffer_range");
    GLAD_GL_EXT_memory_object = glad_gl_has_extension(exts, exts_i, "GL_EXT_memory_object");
    GLAD_GL_EXT_memory_object_fd = glad_gl_has_extension(exts, exts_i, "GL_EXT_memory_object_fd");
    GLAD_GL_EXT_memory_object_win32 = glad_gl_has_extension(exts, exts_i, "GL_EXT_memory_object_win32");
    GLAD_GL_EXT_multi_draw_arrays = glad_gl_has_extension(exts, exts_i, "GL_EXT_multi_draw_arrays");
    GLAD_GL_EXT_multi_draw_indirect = glad_gl_has_extension(exts, exts_i, "GL_EXT_multi_draw_indirect");
    GLAD_GL_EXT_multisampled_compatibility = glad_gl_has_extension(exts, exts_i, "GL_EXT_multisampled_compatibility");
    GLAD_GL_EXT_multisampled_render_to_texture = glad_gl_has_extension(exts, exts_i, "GL_EXT_multisampled_render_to_texture");
    GLAD_GL_EXT_multisampled_render_to_texture2 = glad_gl_has_extension(exts, exts_i, "GL_EXT_multisampled_render_to_texture2");
    GLAD_GL_EXT_multiview_draw_buffers = glad_gl_has_extension(exts, exts_i, "GL_EXT_multiview_draw_buffers");
    GLAD_GL_EXT_multiview_tessellation_geometry_shader = glad_gl_has_extension(exts, exts_i, "GL_EXT_multiview_tessellation_geometry_shader");
    GLAD_GL_EXT_multiview_texture_multisample = glad_gl_has_extension(exts, exts_i, "GL_EXT_multiview_texture_multisample");
    GLAD_GL_EXT_multiview_timer_query = glad_gl_has_extension(exts, exts_i, "GL_EXT_multiview_timer_query");
    GLAD_GL_EXT_occlusion_query_boolean = glad_gl_has_extension(exts, exts_i, "GL_EXT_occlusion_query_boolean");
    GLAD_GL_EXT_polygon_offset_clamp = glad_gl_has_extension(exts, exts_i, "GL_EXT_polygon_offset_clamp");
    GLAD_GL_EXT_post_depth_coverage = glad_gl_has_extension(exts, exts_i, "GL_EXT_post_depth_coverage");
    GLAD_GL_EXT_primitive_bounding_box = glad_gl_has_extension(exts, exts_i, "GL_EXT_primitive_bounding_box");
    GLAD_GL_EXT_protected_textures = glad_gl_has_extension(exts, exts_i, "GL_EXT_protected_textures");
    GLAD_GL_EXT_pvrtc_sRGB = glad_gl_has_extension(exts, exts_i, "GL_EXT_pvrtc_sRGB");
    GLAD_GL_EXT_raster_multisample = glad_gl_has_extension(exts, exts_i, "GL_EXT_raster_multisample");
    GLAD_GL_EXT_read_format_bgra = glad_gl_has_extension(exts, exts_i, "GL_EXT_read_format_bgra");
    GLAD_GL_EXT_render_snorm = glad_gl_has_extension(exts, exts_i, "GL_EXT_render_snorm");
    GLAD_GL_EXT_robustness = glad_gl_has_extension(exts, exts_i, "GL_EXT_robustness");
    GLAD_GL_EXT_sRGB = glad_gl_has_extension(exts, exts_i, "GL_EXT_sRGB");
    GLAD_GL_EXT_sRGB_write_control = glad_gl_has_extension(exts, exts_i, "GL_EXT_sRGB_write_control");
    GLAD_GL_EXT_semaphore = glad_gl_has_extension(exts, exts_i, "GL_EXT_semaphore");
    GLAD_GL_EXT_semaphore_fd = glad_gl_has_extension(exts, exts_i, "GL_EXT_semaphore_fd");
    GLAD_GL_EXT_semaphore_win32 = glad_gl_has_extension(exts, exts_i, "GL_EXT_semaphore_win32");
    GLAD_GL_EXT_separate_depth_stencil = glad_gl_has_extension(exts, exts_i, "GL_EXT_separate_depth_stencil");
    GLAD_GL_EXT_separate_shader_objects = glad_gl_has_extension(exts, exts_i, "GL_EXT_separate_shader_objects");
    GLAD_GL_EXT_shader_framebuffer_fetch = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_framebuffer_fetch");
    GLAD_GL_EXT_shader_framebuffer_fetch_non_coherent = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_framebuffer_fetch_non_coherent");
    GLAD_GL_EXT_shader_group_vote = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_group_vote");
    GLAD_GL_EXT_shader_implicit_conversions = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_implicit_conversions");
    GLAD_GL_EXT_shader_integer_mix = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_integer_mix");
    GLAD_GL_EXT_shader_io_blocks = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_io_blocks");
    GLAD_GL_EXT_shader_non_constant_global_initializers = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_non_constant_global_initializers");
    GLAD_GL_EXT_shader_pixel_local_storage = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_pixel_local_storage");
    GLAD_GL_EXT_shader_pixel_local_storage2 = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_pixel_local_storage2");
    GLAD_GL_EXT_shader_samples_identical = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_samples_identical");
    GLAD_GL_EXT_shader_texture_lod = glad_gl_has_extension(exts, exts_i, "GL_EXT_shader_texture_lod");
    GLAD_GL_EXT_shadow_samplers = glad_gl_has_extension(exts, exts_i, "GL_EXT_shadow_samplers");
    GLAD_GL_EXT_sparse_texture = glad_gl_has_extension(exts, exts_i, "GL_EXT_sparse_texture");
    GLAD_GL_EXT_sparse_texture2 = glad_gl_has_extension(exts, exts_i, "GL_EXT_sparse_texture2");
    GLAD_GL_EXT_tessellation_point_size = glad_gl_has_extension(exts, exts_i, "GL_EXT_tessellation_point_size");
    GLAD_GL_EXT_tessellation_shader = glad_gl_has_extension(exts, exts_i, "GL_EXT_tessellation_shader");
    GLAD_GL_EXT_texture_border_clamp = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_border_clamp");
    GLAD_GL_EXT_texture_buffer = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_buffer");
    GLAD_GL_EXT_texture_compression_astc_decode_mode = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_compression_astc_decode_mode");
    GLAD_GL_EXT_texture_compression_bptc = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_compression_bptc");
    GLAD_GL_EXT_texture_compression_dxt1 = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_compression_dxt1");
    GLAD_GL_EXT_texture_compression_rgtc = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_compression_rgtc");
    GLAD_GL_EXT_texture_compression_s3tc = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_compression_s3tc");
    GLAD_GL_EXT_texture_compression_s3tc_srgb = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_compression_s3tc_srgb");
    GLAD_GL_EXT_texture_cube_map_array = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_cube_map_array");
    GLAD_GL_EXT_texture_filter_anisotropic = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_filter_anisotropic");
    GLAD_GL_EXT_texture_filter_minmax = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_filter_minmax");
    GLAD_GL_EXT_texture_format_BGRA8888 = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_format_BGRA8888");
    GLAD_GL_EXT_texture_format_sRGB_override = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_format_sRGB_override");
    GLAD_GL_EXT_texture_mirror_clamp_to_edge = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_mirror_clamp_to_edge");
    GLAD_GL_EXT_texture_norm16 = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_norm16");
    GLAD_GL_EXT_texture_query_lod = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_query_lod");
    GLAD_GL_EXT_texture_rg = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_rg");
    GLAD_GL_EXT_texture_sRGB_R8 = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_sRGB_R8");
    GLAD_GL_EXT_texture_sRGB_RG8 = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_sRGB_RG8");
    GLAD_GL_EXT_texture_sRGB_decode = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_sRGB_decode");
    GLAD_GL_EXT_texture_shadow_lod = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_shadow_lod");
    GLAD_GL_EXT_texture_storage = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_storage");
    GLAD_GL_EXT_texture_storage_compression = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_storage_compression");
    GLAD_GL_EXT_texture_type_2_10_10_10_REV = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_type_2_10_10_10_REV");
    GLAD_GL_EXT_texture_view = glad_gl_has_extension(exts, exts_i, "GL_EXT_texture_view");
    GLAD_GL_EXT_unpack_subimage = glad_gl_has_extension(exts, exts_i, "GL_EXT_unpack_subimage");
    GLAD_GL_EXT_win32_keyed_mutex = glad_gl_has_extension(exts, exts_i, "GL_EXT_win32_keyed_mutex");
    GLAD_GL_EXT_window_rectangles = glad_gl_has_extension(exts, exts_i, "GL_EXT_window_rectangles");
    GLAD_GL_KHR_blend_equation_advanced = glad_gl_has_extension(exts, exts_i, "GL_KHR_blend_equation_advanced");
    GLAD_GL_KHR_blend_equation_advanced_coherent = glad_gl_has_extension(exts, exts_i, "GL_KHR_blend_equation_advanced_coherent");
    GLAD_GL_KHR_context_flush_control = glad_gl_has_extension(exts, exts_i, "GL_KHR_context_flush_control");
    GLAD_GL_KHR_debug = glad_gl_has_extension(exts, exts_i, "GL_KHR_debug");
    GLAD_GL_KHR_no_error = glad_gl_has_extension(exts, exts_i, "GL_KHR_no_error");
    GLAD_GL_KHR_parallel_shader_compile = glad_gl_has_extension(exts, exts_i, "GL_KHR_parallel_shader_compile");
    GLAD_GL_KHR_robust_buffer_access_behavior = glad_gl_has_extension(exts, exts_i, "GL_KHR_robust_buffer_access_behavior");
    GLAD_GL_KHR_robustness = glad_gl_has_extension(exts, exts_i, "GL_KHR_robustness");
    GLAD_GL_KHR_shader_subgroup = glad_gl_has_extension(exts, exts_i, "GL_KHR_shader_subgroup");
    GLAD_GL_KHR_texture_compression_astc_hdr = glad_gl_has_extension(exts, exts_i, "GL_KHR_texture_compression_astc_hdr");
    GLAD_GL_KHR_texture_compression_astc_ldr = glad_gl_has_extension(exts, exts_i, "GL_KHR_texture_compression_astc_ldr");
    GLAD_GL_KHR_texture_compression_astc_sliced_3d = glad_gl_has_extension(exts, exts_i, "GL_KHR_texture_compression_astc_sliced_3d");
    GLAD_GL_OES_EGL_image = glad_gl_has_extension(exts, exts_i, "GL_OES_EGL_image");
    GLAD_GL_OES_EGL_image_external = glad_gl_has_extension(exts, exts_i, "GL_OES_EGL_image_external");
    GLAD_GL_OES_EGL_image_external_essl3 = glad_gl_has_extension(exts, exts_i, "GL_OES_EGL_image_external_essl3");
    GLAD_GL_OES_compressed_ETC1_RGB8_sub_texture = glad_gl_has_extension(exts, exts_i, "GL_OES_compressed_ETC1_RGB8_sub_texture");
    GLAD_GL_OES_compressed_ETC1_RGB8_texture = glad_gl_has_extension(exts, exts_i, "GL_OES_compressed_ETC1_RGB8_texture");
    GLAD_GL_OES_compressed_paletted_texture = glad_gl_has_extension(exts, exts_i, "GL_OES_compressed_paletted_texture");
    GLAD_GL_OES_copy_image = glad_gl_has_extension(exts, exts_i, "GL_OES_copy_image");
    GLAD_GL_OES_depth24 = glad_gl_has_extension(exts, exts_i, "GL_OES_depth24");
    GLAD_GL_OES_depth32 = glad_gl_has_extension(exts, exts_i, "GL_OES_depth32");
    GLAD_GL_OES_depth_texture = glad_gl_has_extension(exts, exts_i, "GL_OES_depth_texture");
    GLAD_GL_OES_draw_buffers_indexed = glad_gl_has_extension(exts, exts_i, "GL_OES_draw_buffers_indexed");
    GLAD_GL_OES_draw_elements_base_vertex = glad_gl_has_extension(exts, exts_i, "GL_OES_draw_elements_base_vertex");
    GLAD_GL_OES_element_index_uint = glad_gl_has_extension(exts, exts_i, "GL_OES_element_index_uint");
    GLAD_GL_OES_fbo_render_mipmap = glad_gl_has_extension(exts, exts_i, "GL_OES_fbo_render_mipmap");
    GLAD_GL_OES_fragment_precision_high = glad_gl_has_extension(exts, exts_i, "GL_OES_fragment_precision_high");
    GLAD_GL_OES_geometry_point_size = glad_gl_has_extension(exts, exts_i, "GL_OES_geometry_point_size");
    GLAD_GL_OES_geometry_shader = glad_gl_has_extension(exts, exts_i, "GL_OES_geometry_shader");
    GLAD_GL_OES_get_program_binary = glad_gl_has_extension(exts, exts_i, "GL_OES_get_program_binary");
    GLAD_GL_OES_gpu_shader5 = glad_gl_has_extension(exts, exts_i, "GL_OES_gpu_shader5");
    GLAD_GL_OES_mapbuffer = glad_gl_has_extension(exts, exts_i, "GL_OES_mapbuffer");
    GLAD_GL_OES_packed_depth_stencil = glad_gl_has_extension(exts, exts_i, "GL_OES_packed_depth_stencil");
    GLAD_GL_OES_primitive_bounding_box = glad_gl_has_extension(exts, exts_i, "GL_OES_primitive_bounding_box");
    GLAD_GL_OES_required_internalformat = glad_gl_has_extension(exts, exts_i, "GL_OES_required_internalformat");
    GLAD_GL_OES_rgb8_rgba8 = glad_gl_has_extension(exts, exts_i, "GL_OES_rgb8_rgba8");
    GLAD_GL_OES_sample_shading = glad_gl_has_extension(exts, exts_i, "GL_OES_sample_shading");
    GLAD_GL_OES_sample_variables = glad_gl_has_extension(exts, exts_i, "GL_OES_sample_variables");
    GLAD_GL_OES_shader_image_atomic = glad_gl_has_extension(exts, exts_i, "GL_OES_shader_image_atomic");
    GLAD_GL_OES_shader_io_blocks = glad_gl_has_extension(exts, exts_i, "GL_OES_shader_io_blocks");
    GLAD_GL_OES_shader_multisample_interpolation = glad_gl_has_extension(exts, exts_i, "GL_OES_shader_multisample_interpolation");
    GLAD_GL_OES_standard_derivatives = glad_gl_has_extension(exts, exts_i, "GL_OES_standard_derivatives");
    GLAD_GL_OES_stencil1 = glad_gl_has_extension(exts, exts_i, "GL_OES_stencil1");
    GLAD_GL_OES_stencil4 = glad_gl_has_extension(exts, exts_i, "GL_OES_stencil4");
    GLAD_GL_OES_surfaceless_context = glad_gl_has_extension(exts, exts_i, "GL_OES_surfaceless_context");
    GLAD_GL_OES_tessellation_point_size = glad_gl_has_extension(exts, exts_i, "GL_OES_tessellation_point_size");
    GLAD_GL_OES_tessellation_shader = glad_gl_has_extension(exts, exts_i, "GL_OES_tessellation_shader");
    GLAD_GL_OES_texture_3D = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_3D");
    GLAD_GL_OES_texture_border_clamp = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_border_clamp");
    GLAD_GL_OES_texture_buffer = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_buffer");
    GLAD_GL_OES_texture_compression_astc = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_compression_astc");
    GLAD_GL_OES_texture_cube_map_array = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_cube_map_array");
    GLAD_GL_OES_texture_float = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_float");
    GLAD_GL_OES_texture_float_linear = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_float_linear");
    GLAD_GL_OES_texture_half_float = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_half_float");
    GLAD_GL_OES_texture_half_float_linear = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_half_float_linear");
    GLAD_GL_OES_texture_npot = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_npot");
    GLAD_GL_OES_texture_stencil8 = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_stencil8");
    GLAD_GL_OES_texture_storage_multisample_2d_array = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_storage_multisample_2d_array");
    GLAD_GL_OES_texture_view = glad_gl_has_extension(exts, exts_i, "GL_OES_texture_view");
    GLAD_GL_OES_vertex_array_object = glad_gl_has_extension(exts, exts_i, "GL_OES_vertex_array_object");
    GLAD_GL_OES_vertex_half_float = glad_gl_has_extension(exts, exts_i, "GL_OES_vertex_half_float");
    GLAD_GL_OES_vertex_type_10_10_10_2 = glad_gl_has_extension(exts, exts_i, "GL_OES_vertex_type_10_10_10_2");
    GLAD_GL_OES_viewport_array = glad_gl_has_extension(exts, exts_i, "GL_OES_viewport_array");

    glad_gl_free_extensions(exts_i);

    return 1;
}

static int glad_gl_find_core_gles2(void) {
    int i;
    const char* version;
    const char* prefixes[] = {
        "OpenGL ES-CM ",
        "OpenGL ES-CL ",
        "OpenGL ES ",
        "OpenGL SC ",
        NULL
    };
    int major = 0;
    int minor = 0;
    version = (const char*) glad_glGetString(GL_VERSION);
    if (!version) return 0;
    for (i = 0;  prefixes[i];  i++) {
        const size_t length = strlen(prefixes[i]);
        if (strncmp(version, prefixes[i], length) == 0) {
            version += length;
            break;
        }
    }

    GLAD_IMPL_UTIL_SSCANF(version, "%d.%d", &major, &minor);

    GLAD_GL_ES_VERSION_2_0 = (major == 2 && minor >= 0) || major > 2;

    return GLAD_MAKE_VERSION(major, minor);
}

int gladLoadGLES2UserPtr( GLADuserptrloadfunc load, void *userptr) {
    int version;

    glad_glGetString = (PFNGLGETSTRINGPROC) load(userptr, "glGetString");
    if(glad_glGetString == NULL) return 0;
    version = glad_gl_find_core_gles2();

    glad_gl_load_GL_ES_VERSION_2_0(load, userptr);

    if (!glad_gl_find_extensions_gles2()) return 0;
    glad_gl_load_GL_EXT_EGL_image_storage(load, userptr);
    glad_gl_load_GL_EXT_base_instance(load, userptr);
    glad_gl_load_GL_EXT_blend_func_extended(load, userptr);
    glad_gl_load_GL_EXT_buffer_storage(load, userptr);
    glad_gl_load_GL_EXT_clear_texture(load, userptr);
    glad_gl_load_GL_EXT_clip_control(load, userptr);
    glad_gl_load_GL_EXT_copy_image(load, userptr);
    glad_gl_load_GL_EXT_debug_label(load, userptr);
    glad_gl_load_GL_EXT_debug_marker(load, userptr);
    glad_gl_load_GL_EXT_discard_framebuffer(load, userptr);
    glad_gl_load_GL_EXT_disjoint_timer_query(load, userptr);
    glad_gl_load_GL_EXT_draw_buffers(load, userptr);
    glad_gl_load_GL_EXT_draw_buffers_indexed(load, userptr);
    glad_gl_load_GL_EXT_draw_elements_base_vertex(load, userptr);
    glad_gl_load_GL_EXT_draw_instanced(load, userptr);
    glad_gl_load_GL_EXT_draw_transform_feedback(load, userptr);
    glad_gl_load_GL_EXT_external_buffer(load, userptr);
    glad_gl_load_GL_EXT_fragment_shading_rate(load, userptr);
    glad_gl_load_GL_EXT_geometry_shader(load, userptr);
    glad_gl_load_GL_EXT_instanced_arrays(load, userptr);
    glad_gl_load_GL_EXT_map_buffer_range(load, userptr);
    glad_gl_load_GL_EXT_memory_object(load, userptr);
    glad_gl_load_GL_EXT_memory_object_fd(load, userptr);
    glad_gl_load_GL_EXT_memory_object_win32(load, userptr);
    glad_gl_load_GL_EXT_multi_draw_arrays(load, userptr);
    glad_gl_load_GL_EXT_multi_draw_indirect(load, userptr);
    glad_gl_load_GL_EXT_multisampled_render_to_texture(load, userptr);
    glad_gl_load_GL_EXT_multiview_draw_buffers(load, userptr);
    glad_gl_load_GL_EXT_occlusion_query_boolean(load, userptr);
    glad_gl_load_GL_EXT_polygon_offset_clamp(load, userptr);
    glad_gl_load_GL_EXT_primitive_bounding_box(load, userptr);
    glad_gl_load_GL_EXT_raster_multisample(load, userptr);
    glad_gl_load_GL_EXT_robustness(load, userptr);
    glad_gl_load_GL_EXT_semaphore(load, userptr);
    glad_gl_load_GL_EXT_semaphore_fd(load, userptr);
    glad_gl_load_GL_EXT_semaphore_win32(load, userptr);
    glad_gl_load_GL_EXT_separate_shader_objects(load, userptr);
    glad_gl_load_GL_EXT_shader_framebuffer_fetch_non_coherent(load, userptr);
    glad_gl_load_GL_EXT_shader_pixel_local_storage2(load, userptr);
    glad_gl_load_GL_EXT_sparse_texture(load, userptr);
    glad_gl_load_GL_EXT_tessellation_shader(load, userptr);
    glad_gl_load_GL_EXT_texture_border_clamp(load, userptr);
    glad_gl_load_GL_EXT_texture_buffer(load, userptr);
    glad_gl_load_GL_EXT_texture_storage(load, userptr);
    glad_gl_load_GL_EXT_texture_storage_compression(load, userptr);
    glad_gl_load_GL_EXT_texture_view(load, userptr);
    glad_gl_load_GL_EXT_win32_keyed_mutex(load, userptr);
    glad_gl_load_GL_EXT_window_rectangles(load, userptr);
    glad_gl_load_GL_KHR_blend_equation_advanced(load, userptr);
    glad_gl_load_GL_KHR_debug(load, userptr);
    glad_gl_load_GL_KHR_parallel_shader_compile(load, userptr);
    glad_gl_load_GL_KHR_robustness(load, userptr);
    glad_gl_load_GL_OES_EGL_image(load, userptr);
    glad_gl_load_GL_OES_copy_image(load, userptr);
    glad_gl_load_GL_OES_draw_buffers_indexed(load, userptr);
    glad_gl_load_GL_OES_draw_elements_base_vertex(load, userptr);
    glad_gl_load_GL_OES_geometry_shader(load, userptr);
    glad_gl_load_GL_OES_get_program_binary(load, userptr);
    glad_gl_load_GL_OES_mapbuffer(load, userptr);
    glad_gl_load_GL_OES_primitive_bounding_box(load, userptr);
    glad_gl_load_GL_OES_sample_shading(load, userptr);
    glad_gl_load_GL_OES_tessellation_shader(load, userptr);
    glad_gl_load_GL_OES_texture_3D(load, userptr);
    glad_gl_load_GL_OES_texture_border_clamp(load, userptr);
    glad_gl_load_GL_OES_texture_buffer(load, userptr);
    glad_gl_load_GL_OES_texture_storage_multisample_2d_array(load, userptr);
    glad_gl_load_GL_OES_texture_view(load, userptr);
    glad_gl_load_GL_OES_vertex_array_object(load, userptr);
    glad_gl_load_GL_OES_viewport_array(load, userptr);



    return version;
}


int gladLoadGLES2( GLADloadfunc load) {
    return gladLoadGLES2UserPtr( glad_gl_get_proc_from_userptr, GLAD_GNUC_EXTENSION (void*) load);
}



 


#ifdef __cplusplus
}
#endif
