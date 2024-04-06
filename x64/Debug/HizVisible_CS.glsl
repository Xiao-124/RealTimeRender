#version 430 core


layout (local_size_x = 1, local_size_y = 1, local_size_z = 64) in;


struct BboxData 
{
  vec4    bboxMin;
  vec4    bboxMax;
};


layout(binding=0, std430) writeonly buffer visibleBuffer 
{
	int visibles[];
};

layout(binding=1, std430) readonly buffer bboxBuffer 
{
	BboxData bboxes[];
};




layout(std140, binding = 0) uniform u_Matrices4ProjectionWorld
{
	mat4 u_ProjectionMatrix;
	mat4 u_ViewMatrix;
};
uniform sampler2D depthTex;

vec4 getBoxCorner(vec4 bboxMin, vec4 bboxMax, int n)
{
//#if 1
//    bvec3 useMax = bvec3((n & 1) != 0, (n & 2) != 0, (n & 4) != 0);
//    return vec4(mix(bboxMin.xyz, bboxMax.xyz, useMax),1);
//#else
    switch(n)
    {
    case 0:
        return vec4(bboxMin.x,bboxMin.y,bboxMin.z,1);
    case 1:
        return vec4(bboxMax.x,bboxMin.y,bboxMin.z,1);
    case 2:
        return vec4(bboxMin.x,bboxMax.y,bboxMin.z,1);
    case 3:
        return vec4(bboxMax.x,bboxMax.y,bboxMin.z,1);
    case 4:
        return vec4(bboxMin.x,bboxMin.y,bboxMax.z,1);
    case 5:
        return vec4(bboxMax.x,bboxMin.y,bboxMax.z,1);
    case 6:
        return vec4(bboxMin.x,bboxMax.y,bboxMax.z,1);
    case 7:
        return vec4(bboxMax.x,bboxMax.y,bboxMax.z,1);
    }
//#endif
}

uint getCullBits(vec4 hPos)
{
    uint cullBits = 0;
    cullBits |= hPos.x < -hPos.w ?  1 : 0;
    cullBits |= hPos.x >  hPos.w ?  2 : 0;
    cullBits |= hPos.y < -hPos.w ?  4 : 0;
    cullBits |= hPos.y >  hPos.w ?  8 : 0;
    cullBits |= hPos.z < -hPos.w ? 16 : 0;
    cullBits |= hPos.z >  hPos.w ? 32 : 0;
    cullBits |= hPos.w <= 0      ? 64 : 0; 
    return cullBits;
}

vec3 projected(vec4 pos) 
{
    return pos.xyz/pos.w;
}

void main()
{
	uvec3 ThreadId = gl_GlobalInvocationID.xyz;
	uint objectId = ThreadId.z;


	bool isVisible = true;
    mat4 worldViewProjTM = u_ProjectionMatrix*u_ViewMatrix;
  

    vec4 bboxMin = bboxes[objectId].bboxMin;
    vec4 bboxMax = bboxes[objectId].bboxMax;

    // clipspace bbox
    vec4 hPos0    = worldViewProjTM * getBoxCorner(bboxMin, bboxMax, 0);
    vec3 clipmin  = projected(hPos0);
    vec3 clipmax  = clipmin;
    uint clipbits = getCullBits(hPos0);

    for (int n = 1; n < 8; n++)
    {
        vec4 hPos   = worldViewProjTM * getBoxCorner(bboxMin, bboxMax, n);
        vec3 ab     = projected(hPos);
        clipmin     = min(clipmin,ab);
        clipmax     = max(clipmax,ab);
        clipbits    &= getCullBits(hPos);
    }

    //isVisible = (clipbits == 0);
    if (isVisible)
    {
        clipmin = clipmin * 0.5 + 0.5;
        clipmax = clipmax * 0.5 + 0.5;
        vec2 size = (clipmax.xy - clipmin.xy);
        ivec2 texsize = textureSize(depthTex,0);
        float maxsize = max(size.x, size.y) * float(max(texsize.x,texsize.y));
        float miplevel = ceil(log2(maxsize));
    
        float depth = 0;
        float a = textureLod(depthTex,clipmin.xy,miplevel).r;
        float b = textureLod(depthTex,vec2(clipmax.x,clipmin.y),miplevel).r;
        float c = textureLod(depthTex,clipmax.xy,miplevel).r;
        float d = textureLod(depthTex,vec2(clipmin.x,clipmax.y),miplevel).r;


        depth = max(depth,   max(max(max(a,b),c),d));
        //depth = min(depth,   min(min(min(a,b),c),d));

        isVisible =  clipmin.z <= depth;
        //isVisible =  clipmin.z < depth;
    }
    visibles[objectId] = isVisible ? 1 : 0;

}