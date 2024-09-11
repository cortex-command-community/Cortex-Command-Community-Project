#pragma once

#include "Serializable.h"
#include "glm/fwd.hpp"

namespace RTE {
	class Shader {
	public:
		/// Constructs an empty shader program, which can be initialized using `Shader::Compile`
		Shader();

		/// Constructs a Shader from vertex shader file and fragment shader file.
		/// @param vertexFilename
		/// Filepath to the vertex shader file.
		/// @param fragPath
		/// Filepath to the fragment shader file.
		Shader(const std::string& vertexFilename, const std::string& fragPath);

		/// Destructor.
		virtual ~Shader();

		/// Create this shader from a vertex shader file and a fragment shader file.
		/// @param vertexFilename
		/// Filepath to the vertex shader.
		/// @param fragPath
		/// Filepath to the fragment shader
		bool Compile(const std::string& vertexFilename, const std::string& fragPath);

		void Use();
#pragma region Uniform handling
		/// Returns the location of a uniform given by name.
		/// @param name
		/// String containing the name of a uniform in this shader program.
		/// @return
		/// A GLint containing the location of the requested uniform.
		int32_t GetUniformLocation(const std::string& name);

		/// Set a boolean uniform value in the active program by name.
		/// @param name
		/// The name of the uniform to set.
		/// @param value
		/// The boolean value to set the uniform to.
		void SetBool(const std::string& name, bool value);

		/// Set an integer uniform value in the active program by name.
		/// @param name
		/// The name of the uniform to set.
		/// @param value
		/// The integer value to set the uniform to.
		void SetInt(const std::string& name, int value);

		/// Set a float uniform value in the active program by name.
		/// @param name
		/// The name of the uniform to set.
		/// @param value
		/// The float value to set the uniform to.
		void SetFloat(const std::string& name, float value);

		/// Set a float mat4 uniform value in the active program by name.
		/// @param name
		/// The name of the uniform to set.
		/// @param value
		/// The float mat4 value to set the uniform to.
		void SetMatrix4f(const std::string& name, const glm::mat4& value);

		/// Set a float vec2 uniform value in the active program by name.
		/// @param name
		/// The name of the uniform to set.
		/// @param value
		/// The float vec2 value to set the uniform to.
		void SetVector2f(const std::string& name, const glm::vec2& value);

		/// Set a float vec3 uniform value in the active program by name.
		/// @param name
		/// The name of the uniform to set.
		/// @param value
		/// The float vec3 value to set the uniform to.
		void SetVector3f(const std::string& name, const glm::vec3& value);

		/// Set a float vec4 uniform value in the active program by name.
		/// @param name
		/// The name of the uniform to set.
		/// @param value
		/// The float vec4 value to set the uniform to.
		void SetVector4f(const std::string& name, const glm::vec4& value);

		/// Set a boolean uniform value in the active program by location.
		/// @param uniformLoc
		/// The location of the uniform to set.
		/// @param value
		/// The boolean value to set the uniform to.
		static void SetBool(int32_t uniformLoc, bool value);

		/// Set an integer uniform value in the active program by location.
		/// @param uniformLoc
		/// The location of the uniform to set.
		/// @param value
		/// The integer value to set the uniform to.
		static void SetInt(int32_t uniformLoc, int value);

		/// Set a float uniform value in the active program by location.
		/// @param uniformLoc
		/// The location of the uniform to set.
		/// @param value
		/// The float value to set the uniform to.
		static void SetFloat(int32_t uniformLoc, float value);

		/// Set a float mat4 uniform value in the active program by location.
		/// @param uniformLoc
		/// The location of the uniform to set.
		/// @param value
		/// The float mat4 value to set the uniform to.
		static void SetMatrix4f(int32_t uniformLoc, const glm::mat4& value);

		/// Set a float vec2 uniform value in the active program by location.
		/// @param uniformLoc
		/// The location of the uniform to set.
		/// @param value
		/// The float vec2 value to set the uniform to.
		static void SetVector2f(int32_t uniformLoc, const glm::vec2& value);

		/// Set a float vec3 uniform value in the active program by location.
		/// @param uniformLoc
		/// The location of the uniform to set.
		/// @param value
		/// The float vec3 value to set the uniform to.
		static void SetVector3f(int32_t uniformLoc, const glm::vec3& value);

		/// Set a float vec4 uniform value in the active program by location.
		/// @param uniformLoc
		/// The location of the uniform to set.
		/// @param value
		/// The float vec4 value to set the uniform to.
		static void SetVector4f(int32_t uniformLoc, const glm::vec4& value);
#pragma endregion

#pragma region Engine Defined Uniforms

		/// Get the location of the texture sampler uniform.
		/// @return
		/// The location of the texture uniform. This may be -1 if the shader doesn't use textures, in which case the value will be ignored.
		int GetTextureUniform() { return m_TextureUniform; }

		/// Get the location of the color modifier uniform.
		/// @return
		/// The location of the color modifier uniform. This may be -1 if the shader doesn't use the color mod, in which case the value will  be ignored.
		int GetColorUniform() { return m_ColorUniform; }

		/// Get the location of the transformation matrix uniform.
		/// @return
		/// The location of the transformation matrix uniform. This may be -1 if the shader doesn't use transforms, in which case the value will be ignored.
		int GetTransformUniform() { return m_TransformUniform; }

		/// Get the location of the transformation matrix uniform.
		/// @return
		/// The location of the UV transformation matrix uniform. This may be -1 if the shader doesn't use UV transforms, in which case the value will be ignored.
		int GetUVTransformUniform() { return m_UVTransformUniform; }

		/// Get the location of the projection matrix uniform.
		/// @return
		/// The location of the color modifier uniform. This may be -1 if the shader doesn't apply projection, in which case the value will be ignored.
		int GetProjectionUniform() { return m_ProjectionUniform; }

#pragma endregion
	private:
		uint32_t m_ProgramID;

		/// Compiles a shader component from a data string.
		/// @param shaderID
		/// ID of the shader component to compile.
		/// @param data
		/// The shader code string.
		/// @param error
		/// String to contain error data returned by opengl during compilation.
		/// @return
		/// Whether compilation was successful.
		bool CompileShader(uint32_t shaderID, const std::string& data, std::string& error);

		/// Links a shader program from a vertex and fragment shader.
		/// @param vtxShader
		/// The compiled vertex shader.
		/// @param fragShader
		/// The compiled fragment shader.
		/// @return
		/// Whether linking was successful.
		bool Link(uint32_t vtxShader, uint32_t fragShader);

		/// Sets default values for the shader uniforms (may not persist across frames!)
		void ApplyDefaultUniforms();

		int m_TextureUniform; //!< Location of the texture uniform (sampler2d rteTexture).
		int m_ColorUniform; //!< Location of the colormod uniform (vec4 rteColor).
		int m_TransformUniform; //!< Location of the transform uniform (mat4 rteTransform).
		int m_UVTransformUniform; //!< Location of the UV transform uniform (mat4 rteUVTransform).
		int m_ProjectionUniform; //!< Location of the projection uniform (mat4 rteProjection).
	};
} // namespace RTE
