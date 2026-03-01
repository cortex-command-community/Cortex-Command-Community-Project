#version 330 core
in vec2 textureUV;
out vec4 FragNearest;

uniform sampler2D uMask;
uniform vec2 uViewOrigin;
uniform vec2 uViewSize;
uniform vec2 uSceneSize;

void main()
{
	vec2 sceneUV = (uViewOrigin + textureUV * uViewSize) / uSceneSize;
	float mask = texture(uMask, sceneUV).r;
    
    if (mask != 1.0) {
		vec2 uv = gl_FragCoord.xy / uViewSize;
        FragNearest = vec4(uv, 0.0, 1.0);
    } else {
        FragNearest = vec4(-1.0, -1.0, 0.0, 1.0);
    }
}