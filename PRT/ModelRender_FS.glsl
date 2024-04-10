#version 430 core


#define PI 3.1415926
#define saturate(x)        clamp(x, 0.0, 1.0) 

in  vec3 v2f_Position;
in  vec2 v2f_TexCoords;
in  vec3 v2f_Normal;
out vec4 Color_;



layout(binding=2, std430) readonly buffer ALLSHBuffer 
{
	int _coefficientSH9[];
};

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


ivec3 GetProbeIndex3DFromWorldPos(vec3 worldPos, vec4 _coefficientVoxelSize, float _coefficientVoxelGridSize, vec4 _coefficientVoxelCorner)
{
    vec3 probeIndexF = floor((worldPos.xyz - _coefficientVoxelCorner.xyz) / _coefficientVoxelGridSize);
    ivec3 probeIndex3 = ivec3(probeIndexF.x, probeIndexF.y, probeIndexF.z);
    return probeIndex3;
}

int GetProbeIndex1DFromIndex3D(ivec3 probeIndex3, vec4 _coefficientVoxelSize)
{
    int probeIndex = int(probeIndex3.x * _coefficientVoxelSize.y * _coefficientVoxelSize.z
                    + probeIndex3.y * _coefficientVoxelSize.z 
                    + probeIndex3.z);
    return probeIndex;
}

bool IsIndex3DInsideVoxel(ivec3 probeIndex3, vec4 _coefficientVoxelSize)
{
    bool isInsideVoxelX = 0 <= probeIndex3.x && probeIndex3.x < _coefficientVoxelSize.x;
    bool isInsideVoxelY = 0 <= probeIndex3.y && probeIndex3.y < _coefficientVoxelSize.y;
    bool isInsideVoxelZ = 0 <= probeIndex3.z && probeIndex3.z < _coefficientVoxelSize.z;
    bool isInsideVoxel = isInsideVoxelX && isInsideVoxelY && isInsideVoxelZ;
    return isInsideVoxel;
}

void DecodeSHCoefficientFromVoxel(inout vec3 c[9], int probeIndex)
{
    const int coefficientByteSize = 27; // 3x9 for SH9 RGB
    int offset = probeIndex * coefficientByteSize;   
    for(int i=0; i<9; i++)
    {
        c[i].x = DecodeFloatFromInt(_coefficientSH9[offset + i*3+0]);
        c[i].y = DecodeFloatFromInt(_coefficientSH9[offset + i*3+1]);
        c[i].z = DecodeFloatFromInt(_coefficientSH9[offset + i*3+2]);
    }
}

vec3 GetProbePositionFromIndex3D(ivec3 probeIndex3, float _coefficientVoxelGridSize, vec4 _coefficientVoxelCorner)
{
    vec3 res = vec3(probeIndex3.x, probeIndex3.y, probeIndex3.z) * _coefficientVoxelGridSize + _coefficientVoxelCorner.xyz;
    return res;
}

vec3 TrilinearInterpolationvec3(in vec3 value[8], vec3 rate)
{
    vec3 a = mix(value[0], value[4], rate.x);    // 000, 100
    vec3 b = mix(value[2], value[6], rate.x);    // 010, 110
    vec3 c = mix(value[1], value[5], rate.x);    // 001, 101
    vec3 d = mix(value[3], value[7], rate.x);    // 011, 111
    vec3 e = mix(a, b, rate.y);
    vec3 f = mix(c, d, rate.y);
    vec3 g = mix(e, f, rate.z); 
    return g;
}

vec3 SampleSHVoxel(
    in vec4 worldPos, 
    in vec3 albedo, 
    in vec3 normal,
    in float _coefficientVoxelGridSize,
    in vec4 _coefficientVoxelCorner,
    in vec4 _coefficientVoxelSize
    )
{
    // probe grid index for current fragment
    ivec3 probeIndex3 = GetProbeIndex3DFromWorldPos(worldPos.xyz, _coefficientVoxelSize, _coefficientVoxelGridSize, _coefficientVoxelCorner);
    ivec3 offset[8] = {
        ivec3(0, 0, 0), ivec3(0, 0, 1), ivec3(0, 1, 0), ivec3(0, 1, 1), 
        ivec3(1, 0, 0), ivec3(1, 0, 1), ivec3(1, 1, 0), ivec3(1, 1, 1), 
    };

    vec3 c[9];
    vec3 Lo[8] = { vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), vec3(0, 0, 0), };
    vec3 BRDF = albedo / PI;
    float weight = 0.0005;

    // near 8 probes
    for(int i=0; i<8; i++)
    {
        ivec3 idx3 = probeIndex3 + offset[i];
        bool isInsideVoxel = IsIndex3DInsideVoxel(idx3, _coefficientVoxelSize);
        if(!isInsideVoxel) 
        {
            Lo[i] = vec3(0, 0, 0);
            continue;
        }

        // normal weight blend
        vec3 probePos = GetProbePositionFromIndex3D(idx3, _coefficientVoxelGridSize, _coefficientVoxelCorner);
        vec3 dir = normalize(probePos - worldPos.xyz);
        float normalWeight = saturate(dot(dir, normal));
        weight += normalWeight;

        // decode SH9
        int probeIndex = GetProbeIndex1DFromIndex3D(idx3, _coefficientVoxelSize);
        DecodeSHCoefficientFromVoxel(c,  probeIndex);
        Lo[i] = IrradianceSH9(c, normal) * BRDF * normalWeight;      
    }

    // trilinear interpolation
    vec3 minCorner = GetProbePositionFromIndex3D(probeIndex3, _coefficientVoxelGridSize, _coefficientVoxelCorner);
    vec3 maxCorner = minCorner + vec3(1, 1, 1) * _coefficientVoxelGridSize;
    vec3 rate = (worldPos.xyz - minCorner) / _coefficientVoxelGridSize;
    vec3 color = TrilinearInterpolationvec3(Lo, rate) / weight;
    
    return color;
}



uniform float _coefficientVoxelGridSize;
uniform vec4 _coefficientVoxelCorner;
uniform vec4 _coefficientVoxelSize;
uniform sampler2D u_DiffuseTexture;


uniform vec3 lightDirection = vec3(0.7,0.5,0);
uniform vec3 lightColor = vec3(1,1,1);
uniform float _GIIntensity = 1.0;
void main()
{
	vec3 Albedo = vec3(1);
	vec3 Position = v2f_Position;
	vec3 Normal = normalize(v2f_Normal);

    vec4 WorldPos = vec4(Position, 1.0f);
    vec3 ldir = normalize(lightDirection);

	vec3 DiffuseColor = texture(u_DiffuseTexture, v2f_TexCoords).rgb;	
    // radiance from light
    float NdotL = saturate(dot(Normal, ldir));
    vec3 diffuse = NdotL * lightColor * DiffuseColor;


	vec3 gi = SampleSHVoxel(
                    WorldPos, 
                    Albedo, 
                    Normal,
                    _coefficientVoxelGridSize,
                    _coefficientVoxelCorner,
                    _coefficientVoxelSize
                );
    diffuse += gi*_GIIntensity;

	Color_ = vec4(diffuse, 1.0f);
    //Color_  = vec4(1.0f,1.0f,1.0f, 1.0f);
}