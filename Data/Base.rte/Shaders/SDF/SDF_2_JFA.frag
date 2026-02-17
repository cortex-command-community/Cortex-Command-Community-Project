#version 330 core

in vec2 textureUV;
out vec4 FragNearest;

uniform sampler2D uPrev;
uniform vec2 uViewSize
uniform float uStep;

void main()
{
    // Pixel coords of this fragment inside view
    vec2 fragPx = gl_FragCoord.xy;

    // Sample current nearest value stored at this texel
    vec4 cur = texture(uPrev, textureUV);
    vec2 best = cur.xy;

    // Initialize best distance squared (in pixel units)
    float bestD2;
    if (best.x < 0.0) {
        // No current seed known here -> set large distance
        bestD2 = 1e20;
    } else {
        vec2 bestPx = best * uViewSize;
        vec2 d = bestPx - fragPx;
        bestD2 = dot(d, d);
    }

    // 8-neighborhood directions
    const ivec2 dirs[8] = ivec2[8](
        ivec2(-1, -1), ivec2(-1, 0), ivec2(-1, 1),
        ivec2( 0, -1),               ivec2( 0, 1),
        ivec2( 1, -1), ivec2( 1, 0), ivec2( 1, 1)
    );

    // For each neighbor offset (dir * uStep), sample the previous-pass texture at that location
    for (int i = 0; i < 8; ++i) {
        ivec2 di = dirs[i];

        // Compute neighbor sample UV: textureUV + dir * (uStep / uViewSize)
        vec2 offsetUV = vec2(float(di.x) * (uStep / uViewSize.x),
                             float(di.y) * (uStep / uViewSize.y));

        vec2 sampleUV = textureUV + offsetUV;

        // Skip out-of-range samples (optional but avoids sampling outside)
        if (sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0)
            continue;

        vec4 s = texture(uPrev, sampleUV);

        // s.xy is the candidate seed position (normalized) or (-1,-1) sentinel
        if (s.x < 0.0) continue; // No seed info in that neighbor

        vec2 candPx = s.xy * uViewSize;
        vec2 d = candPx - fragPx;
        float d2 = dot(d, d);

        if (d2 < bestD2) {
            bestD2 = d2;
            best = s.xy;
        }
    }

    // Write best (or (-1,-1) if none found)
    FragNearest = vec4(best, 0.0, 1.0);
}
