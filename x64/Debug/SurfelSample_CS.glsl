
#version 430 core

#define saturate(x)        clamp(x, 0.0, 1.0) 
struct Surfel
{
    vec3 position;
    vec3 normal;
    vec3 albedo;
    float skyMask;
};


layout(binding=0, std430) writeonly buffer SurfelBuffer 
{
	Surfel _surfels[];
};

uniform samplerCube _worldPosCubemap;
uniform samplerCube _normalCubemap;
uniform samplerCube _albedoCubemap;

uniform float _randSeed;
uniform vec4 _probePos;
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


layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
void main ()
{
    uvec3 id = gl_GlobalInvocationID;
    vec2 xy = vec2(gl_GlobalInvocationID.x, gl_GlobalInvocationID.y) / vec2(16, 16);
    xy += vec2(1, 1) * _randSeed;

    float u = rand(xy * 1.0);
    float v = rand(xy * 2.0);
    vec3 dir = UniformSphereSample(u, v);
    dir = normalize(dir);

    Surfel result;


    // opaque geometry
    result.position = textureLod(_worldPosCubemap, dir, 0).rgb;
    result.albedo = textureLod(_albedoCubemap, dir, 0).rgb;
    vec4 normal_and_mask = textureLod(_normalCubemap, dir, 0);
    result.normal = normal_and_mask.xyz;
    result.skyMask = saturate(1.0 - normal_and_mask.w);

    // sample sky
    //result.position += (_probePos.xyz + dir) * result.skyMask;

    uint surfelIndex = id.x * 16 + id.y;

    _surfels[surfelIndex] = result;


}