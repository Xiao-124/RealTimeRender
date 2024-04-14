#version 430 core

layout (local_size_x = 8, local_size_y = 8, local_size_z = 8) in;

layout(binding = 0, rgba8) writeonly uniform image3D Radiance3D;

uniform sampler3D Albedo3D;
uniform sampler3D Normal3D;
uniform sampler2D DirectionalDepthMap;

uniform float MaxCoord;
uniform int VoxelSize;
uniform mat4 lightSpaceMatrix;
uniform float DirectionalLightStrength;
uniform vec3 lightDir;

const float cellSize = 2.0 * MaxCoord / float(VoxelSize);

vec3 volPosToWorldPos(ivec3 volPos)
{
	return vec3(volPos) * cellSize - vec3(MaxCoord);
}



float calculateDirectionalShadow(vec3 FragPos)
{
	vec4 fragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
	float closestDepth = texture(DirectionalDepthMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

	float bais = 0.02;
	//float bais = 0.000005;
    if (closestDepth < currentDepth - bais)
	{
		return 0.0;
	}
	return 1.0;
}


vec3 DecodeNormal(vec3 normal)
{
    //return normal * 2.0f - vec3(1.0f);
	return normal;
}



//拿shadowMap做,有没有可能拿体素做效果会更好，试试体素的。
void main()
{
	if(gl_GlobalInvocationID.x >= VoxelSize ||
		gl_GlobalInvocationID.y >= VoxelSize ||
		gl_GlobalInvocationID.z >= VoxelSize) 
		return;

	ivec3 VoxelPos = ivec3(gl_GlobalInvocationID);
	vec4 color = texelFetch(Albedo3D, VoxelPos, 0);
	if (color.a == 0.0f)
	{
		return;
	}
	vec3 normal = DecodeNormal(texelFetch(Normal3D, VoxelPos, 0).xyz);
	vec3 worldPos = volPosToWorldPos(VoxelPos);


	float dir_visibility = calculateDirectionalShadow(worldPos);
    vec3 diffuseColor = max(dot(lightDir, normal), 0.0) * color.xyz;

	//vec3 diffuseColor = color.xyz;
	vec4 lighting = vec4((dir_visibility * DirectionalLightStrength)* diffuseColor, 1.0);
	//lighting = vec4(color.xyz, 1.0f);

	imageStore(Radiance3D, VoxelPos, lighting);

}