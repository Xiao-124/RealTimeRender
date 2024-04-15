#version 430 core

layout(location = 0) in vec3 _Position;
layout(location = 1) in vec3 _Normal;
layout(location = 2) in vec2 _TexCoord;

layout(std140, binding = 0) uniform u_Matrices4ProjectionWorld
{
	mat4 u_ProjectionMatrix;
	mat4 u_ViewMatrix;
};

struct Surfel
{
    vec3 position;
    vec3 normal;
    vec3 albedo;
    float skyMask;
};

layout(binding=0, std430) readonly buffer SurfelBuffer 
{
	Surfel _surfels[];
};

uniform mat4 u_ModelMatrix;
uniform vec3 probeIndex3;
uniform vec4 _coefficientVoxelSize;

int GetProbeIndex1DFromIndex3D(ivec3 probeIndex3, vec4 _coefficientVoxelSize)
{
    int probeIndex = int(probeIndex3.x * _coefficientVoxelSize.y * _coefficientVoxelSize.z
                    + probeIndex3.y * _coefficientVoxelSize.z 
                    + probeIndex3.z);
    return probeIndex;
}


out flat int instanceID;
out vec3 v2f_FragPosInViewSpace;
out vec2 v2f_TexCoords;
out vec3 v2f_Normal;
out flat int useDiscard;
void main()
{
	vec4 FragPos =  u_ModelMatrix * vec4(_Position, 1.0f);
	int Index = GetProbeIndex1DFromIndex3D(ivec3(probeIndex3), _coefficientVoxelSize);
	instanceID = gl_InstanceID;

	Index = 0;
	float skyMask = _surfels[Index * 256 + instanceID].skyMask;

	if(abs(skyMask - 1.0f) <= 0.001)
	{
		useDiscard = 1;
	}
	else
	{
		useDiscard = 0;
	}
	FragPos.xyz += _surfels[Index * 256 + instanceID].position;


	gl_Position = u_ProjectionMatrix * u_ViewMatrix * FragPos;
	v2f_TexCoords = _TexCoord;
	v2f_Normal = _Normal;	
	v2f_FragPosInViewSpace = vec3(_Position);
}