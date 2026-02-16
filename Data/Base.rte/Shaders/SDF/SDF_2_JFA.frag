#version 330 core
in vec2 textureUV;
out vec4 FragColor;

uniform sampler2D uPrev;
uniform vec2 uViewSize;
uniform int uStep;

void main() {
	FragColor = vec4(texture(uPrev, textureUV).rg, 0.0, 1.0);
	return;
	//FragColor.a += 1.0;
	
    vec2 uv = gl_FragCoord.xy / uViewSize;
    vec2 bestSeed = texture(uPrev, textureUV).xy;
    float bestDist = 1e20;
    if (bestSeed.x >= 0.0) {
        // distance in pixels between current pixel and stored seed
        vec2 seedPos = bestSeed * uViewSize;
        vec2 curPos = gl_FragCoord.xy;
        bestDist = length(seedPos - curPos);
    }

    // sample 8 neighbors at offset jump
    vec2 offsets[8] = vec2[](
        vec2(1.0, 0.0), vec2(-1.0, 0.0),
        vec2(0.0, 1.0), vec2(0.0, -1.0),
        vec2(1.0, 1.0), vec2(-1.0, 1.0),
        vec2(1.0, -1.0), vec2(-1.0, -1.0)
    );

    for (int i = 0; i < 8; ++i) {
        vec2 off = offsets[i] * uStep / uViewSize; // normalized offset
        vec2 s = texture(uPrev, textureUV + off).xy;
        if (s.x >= 0.0) {
            vec2 seedPos = s * uViewSize;
            float d = length(seedPos - gl_FragCoord.xy);
            if (d < bestDist) {
                bestDist = d;
                bestSeed = s;
            }
        }
    }
    FragColor = vec4(bestSeed, 0.0, 1.0);
	/**/
}