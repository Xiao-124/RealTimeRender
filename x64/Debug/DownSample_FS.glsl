#version 430 


uniform int depthLod;
uniform int evenLod;
uniform sampler2D depthTex;
uniform vec2 srcResolution;

in vec2 v2f_TexCoords;
out float gl_FragDepth;
void main()
{
    //vec2 uv = v2f_TexCoords;
    //
    //float depth = 0.0f; 
    //vec2 srcTexelSize = 1.0 / srcResolution;
    //
    //vec2 offsets[] = vec2[](
    //    vec2(0,0),
    //    vec2(0,srcTexelSize.y),
    //    vec2(srcTexelSize.x,srcTexelSize.y),
    //    vec2(srcTexelSize.x,0)  
    //);
    //for (int i = 0; i < 4; i++)
    //{
    //    depth = max(depth,  textureLod(depthTex, uv + offsets[i], depthLod-1).r);
    //}
    //gl_FragDepth = depth;


    ivec2 lodSize = textureSize(depthTex,depthLod-1);
    float depth = 0;
    ivec2 offsets[] = ivec2[](
      ivec2(0,0),
      ivec2(0,1),
      ivec2(1,1),
      ivec2(1,0)
    );
    ivec2 coord = ivec2(gl_FragCoord.xy);
    coord *= 2;
    
    for (int i = 0; i < 4; i++){
      depth = max(
        depth, 
        texelFetch(depthTex,
          clamp(coord + offsets[i], ivec2(0), lodSize - ivec2(1)),
          depthLod-1).r );
    }
    gl_FragDepth = depth;
    
}