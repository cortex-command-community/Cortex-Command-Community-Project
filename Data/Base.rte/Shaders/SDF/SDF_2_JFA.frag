#version 330 core
in vec2 textureUV;
out vec2 FragColor;

uniform sampler2D uPrev;
uniform vec2 uViewOrigin;
uniform vec2 uViewSize;
uniform ivec2 uTexSize;
uniform int uStep;

void main() {
	FragColor = vec2(1.0, 0.5);
	return;
}

    ivec2 pix = ivec2(gl_FragCoord.xy);
    vec2 curWorld = uViewOrigin + textureUV * uViewSize;

    vec2 best = texelFetch(uPrev, pix, 0).xy;
    float bestDist = 1e30;
    if (best.x >= 0.0) bestDist = length(best - curWorld);

    ivec2 dirs[8] = ivec2[8](
        ivec2( 1,  0), ivec2(-1,  0),
        ivec2( 0,  1), ivec2( 0, -1),
        ivec2( 1,  1), ivec2(-1, -1),
        ivec2( 1, -1), ivec2(-1,  1)
    );

    for (int i = 0; i < 8; ++i) {
        ivec2 neighbor = clamp(pix + dirs[i] * uStep, ivec2(0), uTexSize - ivec2(1));
        vec2 neighborNearest = texelFetch(uPrev, neighbor, 0).xy;
        if (neighborNearest.x < 0.0) continue;
        float d = length(neighborNearest - curWorld);
        if (d < bestDist) {
            bestDist = d;
            best = neighborNearest;
        }
    }

    FragColor = best;
}