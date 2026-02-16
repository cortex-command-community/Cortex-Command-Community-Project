#version 330 core
in vec2 textureUV;
out vec4 FragColor;

uniform sampler2D uMask;
uniform vec2 uViewOrigin;
uniform vec2 uViewSize;
uniform vec2 uSceneSize;

void main()
{
	vec2 sceneUV = (uViewOrigin + textureUV * uViewSize) / uSceneSize;
    float mask = texture(uMask, sceneUV).r;
	FragColor = vec4(mask,mask,mask, 1.0);
	return;/*
    if (mask > 0.5) {
        vec2 worldPos = uViewOrigin + textureUV * uViewSize;
        //FragNearest = worldPos;
		FragNearest = vec2(0.5, 0.5);
    } else {
        FragNearest = vec2(-1.0, -1.0);
    }*/
}