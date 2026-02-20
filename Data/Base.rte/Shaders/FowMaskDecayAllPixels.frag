#version 330 core

in vec2 textureUV;
out vec4 FragColor;

uniform float secsSinceLastCall;
uniform float decayPerSecond;

uniform sampler2D instantFowMask;
uniform sampler2D previousFow;

void main() {
	bool isInFieldOfVision = texture(instantFowMask, textureUV).r == 0;
	if (isInFieldOfVision) {
		FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}
	float previousFowValue = texture(previousFow, textureUV).r;
	float finalFowValue = min(1.0, previousFowValue + decayPerSecond * secsSinceLastCall);
	FragColor = vec4(finalFowValue, 0.0, 0.0, 1.0);
	return;
}
