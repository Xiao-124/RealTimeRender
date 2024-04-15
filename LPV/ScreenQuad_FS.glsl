#version 430 core

in  vec2 v2f_TexCoords;
out vec4 Color_;

uniform sampler2D u_IndirectTexture;
uniform sampler2D u_DirectTexture;
uniform sampler2D u_AlbedoTexture;
uniform float u_Exposure = 2.0f;

uniform int useIndirect;
uniform float GIIntensity = 1.0;
void main()
{
	vec3 Albedo = texture(u_AlbedoTexture, v2f_TexCoords).rgb;
	if(equal(Albedo,vec3(0,0,0)) == bvec3(1,1,1))
		Albedo = vec3(0.52, 0.77, 1);

	vec3 DirectColor = (texture(u_DirectTexture, v2f_TexCoords).rgb * 0.8f  )* Albedo;
	vec3 IndirectColor = vec3(0,0,0);
	if(useIndirect == 1)
	{
		IndirectColor = GIIntensity *texture(u_IndirectTexture, v2f_TexCoords).rgb* Albedo;;
	}
	vec3 TexelColor = DirectColor + IndirectColor;


	vec3 mapped = vec3(1.0) - exp(-TexelColor * u_Exposure);
	mapped = pow(mapped, vec3(1.0f / 2.2f));
	Color_ = vec4(mapped, 1.0f);
}