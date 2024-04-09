#version 430 core

in  vec3 v2f_Position;
in  vec2 v2f_TexCoords;
in  vec3 v2f_Normal;


layout (location = 0) out vec4 AlbedoAndMetallic_;
layout (location = 1) out vec4 Normal_;
layout (location = 2) out vec3 Chebyshevs_;


uniform sampler2D u_DiffuseTexture;
uniform float u_Near = 0.1;
uniform float u_Far = 100.0f;


float LinearizeDepth(float vDepth)
{
    float z = vDepth * 2.0 - 1.0; 
    return (2.0 * u_Near * u_Far) / (u_Far + u_Near - z * (u_Far - u_Near));    
}

void main()
{
	float Gamma = 2.2;
	vec3 DiffuseColor = pow(texture(u_DiffuseTexture, v2f_TexCoords).rgb, vec3(Gamma));	
	float Alpha = textureLod(u_DiffuseTexture, v2f_TexCoords,0).a;
	if(Alpha != 1.0f)
		discard;
	AlbedoAndMetallic_ = vec4(DiffuseColor,1.0);
	Normal_ = vec4(v2f_Normal.xyz, 1);
	float depth = (gl_FragCoord.z);
	Chebyshevs_ = vec3(depth, depth * depth,LinearizeDepth(depth));

}