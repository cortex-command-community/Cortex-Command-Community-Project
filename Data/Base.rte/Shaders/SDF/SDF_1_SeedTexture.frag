#version 330 core
in vec2 textureUV;
out vec4 FragNearest;

uniform sampler2D uMask;
uniform vec2 uViewOrigin;
uniform vec2 uViewSize;
uniform vec2 uSceneSize;

const vec2 padding = vec2(1.0, 1.0);

void main()
{
	vec2 sceneUV = (uViewOrigin + textureUV * uViewSize) / uSceneSize;
    float mask = texture(uMask, sceneUV).r;
    if (mask > 0.5) {
        vec2 worldPos = uViewOrigin + textureUV * uViewSize;
        FragNearest = vec4(worldPos, padding);
    } else {
        FragNearest = vec4(-1.0, -1.0, padding);
    }
}