#pragma once
#include "glad/gl.h"

namespace RTE {
	/// Blend Modes
	/// see https://en.wikipedia.org/wiki/Blend_modes
	enum class Blend {
		ALPHA,
		ALPHA_PREMULTIPLY,
		BURN, //!< Requires GL_KHR_blend_equation_advanced
		DODGE, //!< Requires GL_KHR_blend_equation_advanced
		HSL_COLOR, //!< Requires GL_KHR_blend_equation_advanced
		HSL_SATURATION, //!< Requires GL_KHR_blend_equation_advanced
		HSL_LUMINOSITY, //!< Requires GL_KHR_blend_equation_advanced
		SCREEN,
		ADD,
		MULTIPLY,
		CUSTOM
	};
	class BlendMode {
	public:
		/// Constructs default Alpha blending mode.
		BlendMode() = default;

		/// Constructs blend mode from Blend enum. Sets appropriate Func and Equation for Blend mode
		BlendMode(Blend blendMode);

		/// Constructs custom blend mode from srcFunc, destFunc, equation for use in glBlendFunc and glBlendEquation.
		/// @param srcFunc Source function as in glBlendFunc.
		/// @param destFunc Destination function as in glBlendFunc.
		/// @param equation Blending equation as in glBlendEquation.
		BlendMode(GLint srcFunc, GLint destFunc, GLint equation);

		/// Constructs custom blend mode from separate rgb and alpha function and equations for use in glBlendFuncSeparate and glBlendEquationSeparate.
		/// @param srcFuncRGB Source RGB function.
		/// @param srcFuncAlpha Source alpha function.
		/// @param destFuncRGB Destination RGB function.
		BlendMode(GLint srcFuncRGB, GLint srcFuncAlpha, GLint destFuncRGB, GLint destFuncAlpha, GLint equationRGB, GLint equationAlpha);

		void Enable();
	private:
		Blend m_BlendMode{Blend::ALPHA};
		GLint m_SrcFunc{GL_SRC_ALPHA}; //!< glBlendFunc source function.
		GLint m_DestFunc{GL_ONE_MINUS_SRC_ALPHA}; //!< glBlendFunc dest function.
		GLint m_Equation{GL_FUNC_ADD}; //!< glBlendEquation.
		GLint m_SrcFuncRGB{0}; //!< glBlendFuncSeparate source rgb function.
		GLint m_DestFuncRGB{0}; //!< glBlendFuncSeparate dest rgb function.
		GLint m_SrcFuncAlpha{0}; //!< glBlendFuncSeparate source alpha function.
		GLint m_DestFuncAlpha{0}; //!< glBlendFuncSeparate dest alpha function.
		GLint m_EquationRGB{0}; //!< glBlendEquationSeparate RGB equation.
		GLint EquationAlpha{0}; //!< glBlendEquationSeparate alpha equation.
	};
}
