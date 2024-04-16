
#version 430 core

#define saturate(x)        clamp(x, 0.0, 1.0) 


layout(rgba32f, binding = 0) uniform writeonly image2D uPosition;
layout(rgba32f, binding = 1) uniform writeonly image2D uAlbedo;
layout(rgba32f, binding = 2) uniform writeonly image2D uNormal;
layout(rgba32f, binding = 3) uniform writeonly image2D uChebyshev;


uniform samplerCube _worldPosCubemap;
uniform samplerCube _normalCubemap;
uniform samplerCube _albedoCubemap;
uniform samplerCube _chebyshevCubemap;

uniform float _randSeed;
uniform vec4 _probePos;
uniform vec3 _probeIndex3;
uniform vec4 _coefficientVoxelSize;

int GetProbeIndex1DFromIndex3D(ivec3 probeIndex3, vec4 _coefficientVoxelSize)
{
    int probeIndex = int(probeIndex3.x * _coefficientVoxelSize.y * _coefficientVoxelSize.z
        + probeIndex3.y * _coefficientVoxelSize.z
        + probeIndex3.z);
    return probeIndex;
}


float rand(vec2 uv)
{
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

// ref: Unreal Engine 4, MonteCarlo.ush
vec3 UniformSphereSample(float u, float v)
{
    const float C_PI = 3.14159265359f;
    float phi = degrees(2.0 * C_PI * u);
    float cosine_theta = 1.0 - 2.0 * v;
    float sine_theta = sqrt(1.0 - cosine_theta * cosine_theta);
    
    float x = sine_theta * cos(phi);
    float y = sine_theta * sin(phi);
    float z = cosine_theta;

    return vec3(x, y, z);
}


//每个probe采样256根光线
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main()
{
    uvec3 id = gl_GlobalInvocationID;
    vec2 xy = vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y) / vec2(16, 16);
   
    xy += vec2(1, 1) * _randSeed;
    float u = rand(xy * 1.0);
    float v = rand(xy * 2.0);

    //float u = xy.x;
    //float v = xy.y;
    vec3 dir = UniformSphereSample(u, v);
    dir = normalize(dir);

    //拿到index
    int index = GetProbeIndex1DFromIndex3D(ivec3(_probeIndex3), _coefficientVoxelSize);

    // opaque geometry
    vec3 position = textureLod(_worldPosCubemap, dir, 0).rgb;
    vec3 albedo = textureLod(_albedoCubemap, dir, 0).rgb;
    vec4 normal_and_mask = textureLod(_normalCubemap, dir, 0);


    // sample sky
    //result.position += (_probePos.xyz + dir) * result.skyMask;

    uint surfelIndex = id.x * 16 + id.y;

    //写到纹理里面去
    imageStore(uPosition,ivec2(surfelIndex, index), vec4(position, 1.0f));
    imageStore(uAlbedo,  ivec2(surfelIndex, index), vec4(albedo, 1.0));
    imageStore(uNormal,  ivec2(surfelIndex, index), normal_and_mask);
    //imageStore(uChebyshev, vec2(surfelIndex, index), position);

}