
#version 430 core
#define saturate(x)        clamp(x, 0.0, 1.0) 
#define PI 3.1415926

float SH(in int l, in int m, in vec3 s) 
{ 
    #define k01 0.2820947918    // sqrt(  1/PI)/2
    #define k02 0.4886025119    // sqrt(  3/PI)/2
    #define k03 1.0925484306    // sqrt( 15/PI)/2
    #define k04 0.3153915652    // sqrt(  5/PI)/4
    #define k05 0.5462742153    // sqrt( 15/PI)/4

	//vec3 n = s.zxy;
    float x = s.x;
    float y = s.z;
    float z = s.y;
	
    //----------------------------------------------------------
    if( l==0 )          return  k01;
    //----------------------------------------------------------
	if( l==1 && m==-1 ) return  k02*y;
    if( l==1 && m== 0 ) return  k02*z;
    if( l==1 && m== 1 ) return  k02*x;
    //----------------------------------------------------------
	if( l==2 && m==-2 ) return  k03*x*y;
    if( l==2 && m==-1 ) return  k03*y*z;
    if( l==2 && m== 0 ) return  k04*(2.0*z*z-x*x-y*y);
    if( l==2 && m== 1 ) return  k03*x*z;
    if( l==2 && m== 2 ) return  k05*(x*x-y*y);

	return 0.0;
}
 
// decode irradiance
vec3 IrradianceSH9(in vec3 c[9], in vec3 dir)
{
    #define A0 3.1415
    #define A1 2.0943
    #define A2 0.7853

    vec3 irradiance = vec3(0, 0, 0);
    irradiance += SH(0,  0, dir) * c[0] * A0;
    irradiance += SH(1, -1, dir) * c[1] * A1;
    irradiance += SH(1,  0, dir) * c[2] * A1;
    irradiance += SH(1,  1, dir) * c[3] * A1;
    irradiance += SH(2, -2, dir) * c[4] * A2;
    irradiance += SH(2, -1, dir) * c[5] * A2;
    irradiance += SH(2,  0, dir) * c[6] * A2;
    irradiance += SH(2,  1, dir) * c[7] * A2;
    irradiance += SH(2,  2, dir) * c[8] * A2;
    irradiance = max(vec3(0, 0, 0), irradiance);

    return irradiance;
}

#define FIXED_SCALE 100000.0
int EncodeFloatToInt(float x)
{
    return int(x * FIXED_SCALE);
}
float DecodeFloatFromInt(int x)
{
    return float(x) / FIXED_SCALE;
}

struct Surfel
{
    vec3 position;
    vec3 normal;
    vec3 albedo;
    float skyMask;
};

uniform float _skyLightIntensity = 1.0;
uniform vec4 _probePos;
layout(binding=0, std430) readonly buffer SurfelBuffer 
{
	Surfel _surfels[];
};


// 使用定点数存储小数, 因为 compute shader 的 InterlockedAdd 不支持 float
// array size: 3x9=27
layout(binding=1, std430) writeonly buffer SHBuffer 
{
	int _coefficientSH9[];
};

layout(binding=2, std430) writeonly buffer SHBuffer_Float 
{
	float _coefficientSH9Float[];
};


float _GIIntensity;
// volume param
uniform float _coefficientVoxelGridSize;
uniform vec4 _coefficientVoxelCorner;
uniform vec4 _coefficientVoxelSize;

uniform sampler2D u_LightDepthTexture; 
uniform mat4 u_LightVPMatrix;

uniform vec3 lightDirection = vec3(-1.0, -0.7071, 0);
uniform vec3 lightColor = vec3(2,2,2);



float ShadowCalculation(vec4 PosLightSpace, vec3 normal, vec3 lightDir)
{
    // 执行透视除法
    vec3 FragPosInLightSpace = PosLightSpace.xyz / PosLightSpace.w;
    FragPosInLightSpace.xyz = (FragPosInLightSpace.xyz + 1.0) / 2.0;
    if(FragPosInLightSpace.z < 0.0f || FragPosInLightSpace.x > 1.0f || FragPosInLightSpace.y > 1.0f || FragPosInLightSpace.x < 0.0f || FragPosInLightSpace.y < 0.0f )
    {
        return 0;
    }
    else
    {
	    vec2 FragNDCPos4Light = FragPosInLightSpace.xy;
	    float ClosetDepth4Light = texture(u_LightDepthTexture, FragNDCPos4Light).r; 
	    float Bias = max(0.00001 * (1.0 - dot(normal, lightDir)), 0.00001);
	    float Visibility4DirectLight = (FragPosInLightSpace.z - Bias < ClosetDepth4Light) ? 1.0f : 0.0f;
	    return Visibility4DirectLight;
    }
}

vec3 GetProbePositionFromIndex3D(ivec3 probeIndex3, float _coefficientVoxelGridSize, vec4 _coefficientVoxelCorner)
{
    vec3 res = vec3(probeIndex3.x, probeIndex3.y, probeIndex3.z) * _coefficientVoxelGridSize + _coefficientVoxelCorner.xyz;
    return res;
}

int GetProbeIndex1DFromIndex3D(ivec3 probeIndex3, vec4 _coefficientVoxelSize)
{
    int probeIndex = int(probeIndex3.x * _coefficientVoxelSize.y * _coefficientVoxelSize.z
                    + probeIndex3.y * _coefficientVoxelSize.z 
                    + probeIndex3.z);
    return probeIndex;
}

layout (local_size_x = 512, local_size_y = 1, local_size_z = 1) in;
shared vec3 coSH9[512*9];

void main ()
{

    uvec3 lovocationid = gl_LocalInvocationID;
    uvec3 numGroup = gl_NumWorkGroups;
    ivec3 groupId = ivec3(gl_WorkGroupID);
    uint tid = lovocationid.x;

    int probeIndex = GetProbeIndex1DFromIndex3D(groupId, _coefficientVoxelSize);
    vec3 _probePos = GetProbePositionFromIndex3D(ivec3(groupId), _coefficientVoxelGridSize, _coefficientVoxelCorner);

    uint surfelIndex = probeIndex * 512 + lovocationid.x;
    uint shoffset = probeIndex*27;
   
    Surfel surfel = _surfels[surfelIndex];
    vec3 ldir = normalize(-lightDirection);
    vec4 WorldPos = vec4(surfel.position, 1.0f);

    vec4 fragPosLightSpace = u_LightVPMatrix * WorldPos;
    float Visibility4DirectLight = ShadowCalculation(fragPosLightSpace, surfel.normal, ldir);
    Visibility4DirectLight = 1.0;

    // radiance from light
    float NdotL = saturate(dot(surfel.normal, ldir));
    vec3 radiance = surfel.albedo  * NdotL * Visibility4DirectLight;

    // direction from probe to surfel
    vec3 dir = normalize(surfel.position - _probePos.xyz);

    // radiance from sky
    vec3 skyColor = vec3(1,1,1);
    //radiance += skyColor * surfel.skyMask * _skyLightIntensity;  

    // SH projection
    const float N = 32 * 16;
    vec3 c[9];
    c[0] = SH(0,  0, dir) * radiance * 4.0 * PI / N;
    c[1] = SH(1, -1, dir) * radiance * 4.0 * PI / N;
    c[2] = SH(1,  0, dir) * radiance * 4.0 * PI / N;
    c[3] = SH(1,  1, dir) * radiance * 4.0 * PI / N;
    c[4] = SH(2, -2, dir) * radiance * 4.0 * PI / N;
    c[5] = SH(2, -1, dir) * radiance * 4.0 * PI / N;
    c[6] = SH(2,  0, dir) * radiance * 4.0 * PI / N;
    c[7] = SH(2,  1, dir) * radiance * 4.0 * PI / N;
    c[8] = SH(2,  2, dir) * radiance * 4.0 * PI / N;

    for(int i=0; i<9; i++)
    {
        coSH9[lovocationid.x*9 + i]  = c[i];
    }
    groupMemoryBarrier();
    memoryBarrierShared();
    barrier();
    uint s = 256;
    for (; s > 0; s = s >> 1 ) 
    {
        if (tid < s) 
        {
            //sdata[tid] += sdata[tid + s];
            for(int i=0; i<9; i++)
            {
                coSH9[tid*9 + i]  += coSH9[ (tid+s) * 9 + i];
            }

        }
        groupMemoryBarrier();
        memoryBarrierShared();
        barrier();
    }  

    // atom write result to buffer
    for(int i=0; i<9; i++)
    {
        atomicAdd(_coefficientSH9[shoffset + i*3+0], EncodeFloatToInt(c[i].x));
        atomicAdd(_coefficientSH9[shoffset + i*3+1], EncodeFloatToInt(c[i].y));
        atomicAdd(_coefficientSH9[shoffset + i*3+2], EncodeFloatToInt(c[i].z));
    }


    for(int i=0; i<9; i++)
    {
        _coefficientSH9Float[shoffset + i*3+0] = coSH9[i].x;
        _coefficientSH9Float[shoffset + i*3+1] = coSH9[i].y;
        _coefficientSH9Float[shoffset + i*3+2] = coSH9[i].z;
    }

}