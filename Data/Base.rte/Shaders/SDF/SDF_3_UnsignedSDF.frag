#version 330 core
in vec2 textureUV;
out vec4 FragColor;

uniform sampler2D uNearest;
uniform vec2 uViewSize;
uniform float uMaxDist;

void main() {
    vec4 n = texture(uNearest, textureUV);
    if (n.x < 0.0) {
        // No seed in view (treat as far away)
        FragColor = vec4(1.0,1.0,1.0,1.0); // White = far
        return;
    }
    vec2 nearestPx = n.xy * uViewSize;
    vec2 fragPx = gl_FragCoord.xy;
    float dist = length(nearestPx - fragPx); // Distance in pixels

    // Normalize for display (0-1)
    float outVal = clamp(dist / uMaxDist, 0.0, 1.0);	
	FragColor = vec4(outVal, 0.0, 0.0, 1.0);
}