#version 430 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 8) in;
layout(binding = 0, rgba8) uniform writeonly image3D Radiance3D;
layout(binding = 1, rgba8) uniform readonly image3D LastRadiance3D;

const ivec3 offsets[] = ivec3[8]
(
	ivec3(1, 1, 1),
	ivec3(1, 1, 0),
	ivec3(1, 0, 1),
	ivec3(1, 0, 0),
	ivec3(0, 1, 1),
	ivec3(0, 1, 0),
	ivec3(0, 0, 1),
	ivec3(0, 0, 0)
);

// subject to change according to coneTracing method
vec4 fetchSum(ivec3 pos) 
{
	vec3 sum = vec3(0.0);
	float asum = 0.0;
	float acount = 0.0;
	for (int i = 0; i < 8; i++)
	{
		vec4 color = imageLoad(LastRadiance3D, pos + offsets[i]);
		sum += color.xyz;
		asum += color.a;
		acount += step(0.01, color.a);
	}
	return vec4(sum / acount, asum / 8.0);
}

void main()
{
	ivec3 VoxelPos = ivec3(gl_GlobalInvocationID);
	vec4 sum = fetchSum(2 * VoxelPos);
	imageStore(Radiance3D, VoxelPos, sum);
}