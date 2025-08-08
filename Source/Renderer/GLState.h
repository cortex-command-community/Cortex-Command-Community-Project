#pragma once
#include "Shader.h"
#include "glad/gl.h"
#include "BlendMode.h"
#include "glm/fwd.hpp"
#include <vector>

namespace RTE {
	class Shader;
	struct GLState {
		struct SupportedExtensions {
			bool vao{false}; // GL_ARB_vertex_array_object (GLES2.0 support)
			bool instancing{false}; // GL_ARB_draw_instanced
			bool texNPOT{false}; // GL_ARB_texture_non_power_of_two
			bool texDepth{false}; // GL_ARB_depth_texture
			bool texDepthWebGL{false}; // GL_WEBGL_depth_texture
			bool programmablePipeline{false}; // GL_ARB_separate_shader_objects
			bool computeShader{false}; // GL_ARB_compute_shader
			bool ssbo{false};
			bool advanced_blend_equations{false};
			bool advanced_blend_equations_coherent{false};
			bool debug_output{false};
		} m_SupportedExtensions;
		BlendMode m_CurrentBlendMode;
		Shader* m_CurrentShader;
		std::vector<UniformValueType> m_CurrentUniforms;

		void BeginFrame();
	};
}
