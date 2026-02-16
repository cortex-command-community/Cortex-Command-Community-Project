#version 330 core
in vec2 textureUV;
out vec2 FragNearest;

uniform sampler2D uMask;
uniform vec2 uViewOrigin;
uniform vec2 uViewSize;
uniform vec2 uSceneSize;

void main()
{
	FragNearest = vec2(1.0, 0.45);
	return;
    /*vec2 sceneUV = (uViewOrigin + textureUV * uViewSize) / uSceneSize;
    float mask = texture(uMask, sceneUV).r;
    if (mask > 0.5) {
        vec2 worldPos = uViewOrigin + textureUV * uViewSize;
        FragNearest = worldPos;       // store world position
    } else {
        FragNearest = vec2(-1.0, -1.0); // sentinel
    }*/
}