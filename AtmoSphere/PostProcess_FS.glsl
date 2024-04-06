
#version 430 core

in vec2 v2f_TexCoords;
uniform sampler2D resultTexture;

out vec4 FragColor;
void main()
{

	vec2 texCoord = v2f_TexCoords;
	vec4 rgbA = textureLod(resultTexture, texCoord, 0);
	rgbA /= rgbA.aaaa;	// Normalise according to sample count when path tracing
	// Similar setup to the Bruneton demo
	vec3 white_point = vec3(1.08241, 0.96756, 0.95003);
	float exposure = 10.0;
	FragColor =  vec4( pow(vec3(1.0) - exp(-rgbA.rgb / white_point * exposure), vec3(1.0 / 2.2) ), 1.0 );
}