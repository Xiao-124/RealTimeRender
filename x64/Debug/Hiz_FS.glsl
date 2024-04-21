#version 430


uniform int depthLod;
uniform int evenLod;
uniform sampler2D depthTex;

in vec2 v2f_TexCoords;
out float gl_FragDepth;
void main()
{

    ivec2 lodSize = textureSize(depthTex, depthLod);
    float depth = 0;
    if (evenLod == 1)
    {
        ivec2 offsets[] = ivec2[](
            ivec2(0, 0),
            ivec2(0, 1),
            ivec2(1, 1),
            ivec2(1, 0)
            );
        ivec2 coord = ivec2(gl_FragCoord.xy);
        coord *= 2;

        for (int i = 0; i < 4; i++) {
            depth = max(
                depth,
                texelFetch(depthTex,
                    clamp(coord + offsets[i], ivec2(0), lodSize - ivec2(1)),
                    depthLod).r);
        }
    }
    else
    {
        vec2 offsets[] = vec2[](
            vec2(-1, -1),
            vec2(0, -1),
            vec2(1, -1),
            vec2(-1, 0),
            vec2(0, 0),
            vec2(1, 0),
            vec2(-1, 1),
            vec2(0, 1),
            vec2(1, 1)
            );
        vec2 coord = v2f_TexCoords;
        vec2 texel = 1.0 / (vec2(lodSize));

        for (int i = 0; i < 9; i++) {
            vec2 pos = coord + offsets[i] * texel;
            depth = max(
                depth,
#if 1
                texelFetch(depthTex,
                    clamp(ivec2(pos * lodSize), ivec2(0), lodSize - ivec2(1)),
                    depthLod).r
#else
                textureLod(depthTex,
                    pos,
                    depthLod).r
#endif
            );
        }
    }
    gl_FragDepth = depth;

}