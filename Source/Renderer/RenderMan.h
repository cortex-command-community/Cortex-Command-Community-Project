#pragma once
#include "Singleton.h"
#include "RenderBatch.h"
#include "GLState.h"
#include "DrawCall.h"
#include "Texture.h"
#include <memory>

#define g_RenderMan RenderMan::Instance()

namespace RTE {
	class DrawCall;
	class RenderMan: public Singleton<RenderMan> {
	public:
		/// Constructor
		RenderMan() = default;
		/// Destructor
		~RenderMan() { Destroy(); }

		/// Initializes this RenderMan.
		void Initialize();

		/// Destroys this RenderMan.
		void Destroy();

		/// Returns the active RenderBatch.
		/// @return The active RenderBatch.
		const RenderBatch* GetActiveBatch() { return m_RenderBatch.get(); }

		/// Returns the current RenderDepth to be set on new Vertices.
		/// @return Depth value to use for the current draw call.
		float GetCurrentDepth() const { return m_RenderBatch->m_CurrentDepth; }

		/// Returns the current depth offset to be added to the current depth.
		/// @return Depth offset to be added to the vertex or transform.
		float GetCurrentZOffset() const { return m_RenderBatch->m_CurrentZ; }

		/// Set the current depth offset, this can be used to change draw order without sorting.
		/// @param depth The new offset.
		void SetCurrentZOffset(float depth) { m_RenderBatch->m_CurrentDepth = depth; }

		/// Set the default shader for upcoming draw calls.
		/// @param shader (non owning) pointer to the new shader.
		void SetCurrentShader(const Shader* shader) { m_RenderBatch->m_CurrentShader = shader; }

		const Shader* GetCurrentShader() { return m_RenderBatch->m_CurrentShader ? m_RenderBatch->m_CurrentShader : GetDefaultShader(); }

		void SetActiveBlendMode(BlendMode mode) { m_RenderBatch->m_CurrentBlendMode = std::move(mode); }

		/// Add a uniform value to be set on upcoming draw calls.
		/// @param uniform The unform value to enable.
		void PushUniform(std::shared_ptr<UniformValueType> uniform) { m_RenderBatch->m_CurrentUniforms.push_back(uniform); }

		/// Clears active uniforms.
		void ClearUniforms() { m_RenderBatch->m_CurrentUniforms.clear(); }

		/// Schedules a new draw call on the current batch, initialized with the current shader, uniforms and camera.
		std::shared_ptr<DrawCall> BeginDraw();

		/// Returns the shape texture id (1x1 white pixel texture).
		/// @return GL texture object.
		GLuint GetShapeTexture() { return m_ShapesTexture->GetTextureId(); }

		/// Returns the texture id for the default palette. TODO: allow palette swapping.
		GLuint GetPaletteTexture() { return m_PaletteTexture->GetTextureId(); }

		/// Returns the default shader.
		const Shader* GetDefaultShader() { return m_DefaultShader.get(); }

		/// Returns the camera the was set at BeginFrame.
		Camera* GetActiveCamera() { return m_CurrentCamera; }

		/// Begin a set of draws with camera. Clears batched draw calls.
		void BeginFrame(Camera* camera);

		/// Draws current draw calls to the active target and clears the batch for new draws.
		void DrawActiveBatch();
	private:
		std::unique_ptr<RenderBatch> m_RenderBatch{nullptr};
		std::unique_ptr<GLState> m_GLState{nullptr};
		std::shared_ptr<BitmapTexture> m_ShapesTexture{nullptr};
		std::shared_ptr<BitmapTexture> m_PaletteTexture{nullptr};
		std::shared_ptr<Shader> m_DefaultShader{nullptr};
		Camera* m_CurrentCamera{nullptr};
	};
}
