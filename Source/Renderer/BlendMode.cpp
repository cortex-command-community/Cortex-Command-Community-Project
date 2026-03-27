#include "BlendMode.h"
#include <array>
#include <algorithm>

using namespace RTE;

static constexpr std::array<GLint, 5> c_KhrBlendEquations{GL_COLORBURN_KHR, GL_COLORDODGE_KHR, GL_HSL_COLOR_KHR, GL_HSL_SATURATION_KHR, GL_HSL_LUMINOSITY_KHR};

BlendMode::BlendMode(Blend mode) {
	m_BlendMode = mode;
	switch (mode) {
		case Blend::ALPHA: {
			m_SrcFunc = GL_SRC_ALPHA;
			m_DestFunc = GL_ONE_MINUS_SRC_ALPHA;
			m_Equation = GL_FUNC_ADD;
			break;
		}
		case Blend::ALPHA_PREMULTIPLY: {
			m_SrcFunc = GL_ONE;
			m_DestFunc = GL_ONE_MINUS_SRC_ALPHA;
			m_Equation = GL_FUNC_ADD;
			break;
		}
		case Blend::BURN: {
			m_SrcFunc = 0;
			m_DestFunc = 0;
			m_Equation = GL_COLORBURN_KHR;
			break;
		}
		case Blend::DODGE: {
			m_SrcFunc = 0;
			m_DestFunc = 0;
			m_Equation = GL_COLORDODGE_KHR;
			break;
		}
		case Blend::HSL_COLOR: {
			m_SrcFunc = 0;
			m_DestFunc = 0;
			m_Equation = GL_HSL_COLOR_KHR;
			break;
		}
		case Blend::HSL_SATURATION: {
			m_SrcFunc = 0;
			m_DestFunc = 0;
			m_Equation = GL_HSL_SATURATION_KHR;
			break;
		}
		case Blend::HSL_LUMINOSITY: {
			m_SrcFunc = 0;
			m_DestFunc = 0;
			m_Equation = GL_HSL_LUMINOSITY_KHR;
			break;
		}
		case Blend::SCREEN: {
			m_SrcFunc = 0;
			m_DestFunc = 0;
			m_Equation = GL_FUNC_ADD;
			m_SrcFuncRGB = GL_ONE;
			m_DestFuncRGB = GL_ONE_MINUS_SRC_COLOR;
			m_SrcFuncAlpha = GL_ONE;
			m_DestFuncAlpha = GL_ONE_MINUS_SRC_ALPHA;
			break;
		}
		case Blend::ADD: {
			m_SrcFunc = GL_SRC_ALPHA;
			m_DestFunc = GL_ONE;
			m_Equation = GL_FUNC_ADD;
			break;
		}
		case Blend::MULTIPLY: {
			m_SrcFunc = GL_DST_COLOR;
			m_DestFunc = GL_ONE_MINUS_SRC_ALPHA;
			m_Equation = GL_FUNC_ADD;
			break;
		}
		default: {
			m_SrcFunc = GL_ONE;
			m_DestFunc = GL_ONE_MINUS_SRC_ALPHA;
			m_Equation = GL_FUNC_ADD;
			m_BlendMode = Blend::ALPHA;
			break;
		}
	}
}

BlendMode::BlendMode(GLint srcFunc, GLint destFunc, GLint equation) :
    m_BlendMode(Blend::CUSTOM), m_SrcFunc(srcFunc), m_DestFunc(destFunc), m_Equation(equation) {}

BlendMode::BlendMode(GLint srcFuncRGB, GLint srcFuncAlpha, GLint destFuncRGB, GLint destFuncAlpha, GLint equationRGB, GLint equationAlpha) :
    m_BlendMode(Blend::CUSTOM), m_SrcFunc(0), m_DestFunc(0), m_Equation(0), m_SrcFuncRGB(srcFuncRGB), m_DestFuncRGB(destFuncRGB), m_SrcFuncAlpha(srcFuncAlpha), m_DestFuncAlpha(destFuncAlpha), m_EquationRGB(equationRGB), m_EquationAlpha(equationAlpha) {}

void BlendMode::Enable() {
	if (m_BlendMode == Blend::NONE) {
		glDisable(GL_BLEND);
		return;
	}
	glEnable(GL_BLEND);
	if (m_Equation) {
		glBlendEquation(m_Equation);
		if (std::find(c_KhrBlendEquations.cbegin(), c_KhrBlendEquations.cend(), m_Equation) != c_KhrBlendEquations.cend()) {
			return;
		}
	} else {
		glBlendEquationSeparate(m_EquationRGB, m_EquationAlpha);
	}
	if (m_SrcFuncAlpha || m_SrcFuncRGB || m_DestFuncAlpha || m_DestFuncRGB) {
		glBlendFuncSeparate(m_SrcFuncRGB, m_DestFuncRGB, m_SrcFuncAlpha, m_DestFuncAlpha);
	} else {
		glBlendFunc(m_SrcFunc, m_DestFunc);
	}
}
