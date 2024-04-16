#version 430 core

in vec3 v2f_FragPosInViewSpace;
in vec2 v2f_TexCoords;
in vec3 v2f_Normal;


uniform vec3 probeIndex3;
uniform int surfelIndex;
uniform vec4 _coefficientVoxelSize;

out vec4 Color_;
int GetProbeIndex1DFromIndex3D(ivec3 probeIndex3, vec4 _coefficientVoxelSize)
{
    int probeIndex = int(probeIndex3.x * _coefficientVoxelSize.y * _coefficientVoxelSize.z
                    + probeIndex3.y * _coefficientVoxelSize.z 
                    + probeIndex3.z);
    return probeIndex;
}

//layout(binding=0, std430) readonly buffer SurfelBuffer 
//{
//	Surfel _surfels[];
//};

layout(binding=2, std430) readonly buffer SurfelRadianceBuffer
{
	vec3 _surfelRadiance[];
};


in flat int instanceID;
in flat int useDiscard;
void main()
{

    if(useDiscard == 1)
    {
        discard;
    }
    int Index = GetProbeIndex1DFromIndex3D(ivec3(probeIndex3), _coefficientVoxelSize);
    int offset = instanceID + Index * 256;

	Color_ = vec4(_surfelRadiance[offset], 1.0f);

}