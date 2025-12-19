#include "GLState.h"
#include "glad/gl.h"
#include "GLStateMan.h"

using namespace RTE;

GLState::GLState() {
	m_SupportedExtensions = {
	    true,
	    GLAD_GL_ARB_instanced_arrays,
	    GLAD_GL_ARB_texture_non_power_of_two,
	    GLAD_GL_ARB_depth_texture,
	    false,
	    false,
	    GLAD_GL_ARB_compute_shader,
	    GLAD_GL_ARB_shader_storage_buffer_object,
	    GLAD_GL_KHR_blend_equation_advanced,
	    GLAD_GL_KHR_blend_equation_advanced_coherent,
	    GLAD_GL_ARB_debug_output};
	m_CurrentBlendMode = BlendMode(Blend::ALPHA);
	m_CurrentShader = nullptr;
}

void GLState::BeginFrame() {
	m_CurrentShader = nullptr;
	m_CurrentBlendMode = Blend::ALPHA;
}

void GLState::SetBlendMode(BlendMode mode) {
	m_CurrentBlendMode = mode;
}

void GLState::SetShader(Shader* shader) {
	m_CurrentShader = shader;
}
