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
			int vao{true}; // GL_ARB_vertex_array_object (GLES2.0 support)
			int instancing{false}; // GL_ARB_draw_instanced
			int texNPOT{false}; // GL_ARB_texture_non_power_of_two
			int texDepth{false}; // GL_ARB_depth_texture
			int texDepthWebGL{false}; // GL_WEBGL_depth_texture
			int programmablePipeline{false}; // GL_ARB_separate_shader_objects
			int computeShader{false}; // GL_ARB_compute_shader
			int ssbo{false};
			int advanced_blend_equations{false};
			int advanced_blend_equations_coherent{false};
			int debug_output{false};
		} m_SupportedExtensions;
		BlendMode m_CurrentBlendMode;
		Shader* m_CurrentShader;
		std::vector<UniformValueType> m_CurrentUniforms;

		GLState();

		/// Reset to a consistent state at frame start
		void BeginFrame();

		void SetBlendMode(BlendMode mode);
		void SetShader(Shader* shader);
	};
}
