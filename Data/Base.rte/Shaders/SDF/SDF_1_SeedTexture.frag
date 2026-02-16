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
	/*
    
    if (mask > 0.5) {
		vec2 uv = gl_FragCoord.xy / uViewSize;
        FragNearest = vec4(uv, 0.0, 1.0);
    } else {
        FragNearest = vec4(-1.0, -1.0, 0.0, 1.0);
    }*/
	/*if (textureUV.x > 0.5) {
		FragNearest = vec4(1.0, 0.0, 0.0, 1.0);
	} else {
		FragNearest = vec4(0.0, 1.0, 0.0, 1.0);
	}*/
	float mask = texture(uMask, sceneUV).r;
	FragNearest = vec4(mask, mask, 0.0, 1.0);
}