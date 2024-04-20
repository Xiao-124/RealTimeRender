#version 430 core

in  vec2 v2f_TexCoords;
out vec4 Color_;

uniform sampler2D u_PositionTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_LightDepthTexture;


uniform mat4  u_LightVPMatrix;
uniform vec3  u_LightDir;
uniform float u_Intensity = 1.0f;
void main()
{
	vec3 Position = textureLod(u_PositionTexture, v2f_TexCoords, 0).rgb;
	vec3 Normal = textureLod(u_NormalTexture, v2f_TexCoords, 0).rgb;
	vec3 Albedo = textureLod(u_AlbedoTexture, v2f_TexCoords, 0).rgb;

	if ((abs(Normal.x) < 0.0001f) && (abs(Normal.y) < 0.0001f) && (abs(Normal.z) < 0.0001f))
	{
		Color_ = vec4(0, 0, 0, 1);
		return;
	}

	vec4 FragPosInLightSpace = u_LightVPMatrix * vec4(Position, 1);

	float DirectIllumination;
	FragPosInLightSpace /= FragPosInLightSpace.w;
	FragPosInLightSpace.xyz = (FragPosInLightSpace.xyz + 1) / 2;
	float Visibility4DirectLight = 0.0f;
	if (FragPosInLightSpace.z < 0.0f || FragPosInLightSpace.x > 1.0f || FragPosInLightSpace.y > 1.0f || FragPosInLightSpace.x < 0.0f || FragPosInLightSpace.y < 0.0f)
		DirectIllumination = 0;
	else
	{
		vec2 FragNDCPos4Light = FragPosInLightSpace.xy;
		float ClosetDepth4Light = texture(u_LightDepthTexture, FragNDCPos4Light).r;
		float Bias = max(0.00001 * (1.0 - dot(Normal, u_LightDir)), 0.00001);
		Visibility4DirectLight = (FragPosInLightSpace.z - Bias < ClosetDepth4Light) ? 1.0f : 0.0f;
		DirectIllumination = u_Intensity * max(dot(u_LightDir, Normal), 0) * Visibility4DirectLight;
	}
	Color_ = vec4(Albedo * DirectIllumination, 1.0f);
}