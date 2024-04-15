#version 460 core


layout (location = 0) out vec4 LPVGridR;
layout (location = 1) out vec4 LPVGridG;
layout (location = 2) out vec4 LPVGridB;
layout(location = 3)  out vec4  SHCoeffGV;

#define PI 3.1415926f

#define SH_coslobe_c0 0.886226925f
#define SH_coslobe_c1 1.02332671f


in FragData 
{
	vec3 WorldPos;
	vec3 Normal;
	vec4 flux;

}fs_in;


uniform float u_CellSize;
uniform mat4 u_LightViewMat;
uniform float u_RSMArea = 4.0;
uniform int u_RSMResolution;
uniform vec3 u_LightDir;

vec4 evalSH(vec3 normal)
{
	return vec4(SH_coslobe_c0 , -SH_coslobe_c1 * normal.y , SH_coslobe_c1 * normal.z , -SH_coslobe_c1 * normal.x );
}

float calculateSurfelAreaLightOrtho(vec3 lightPos)
{
	return (u_RSMArea) / (u_RSMResolution * u_RSMResolution);
}

float calculateBlockingPotencial(float surfelArea, vec3 dir, vec3 normal)
{
	return clamp((surfelArea * clamp(dot(normal, dir), 0.0, 1.0)) / (u_CellSize * u_CellSize), 0.0, 1.0);
}



void main()
{

	// if(length(fs_in.Normal)<0.01) discard;
	//光线注入
	LPVGridR = (fs_in.flux.r/PI)*evalSH(fs_in.Normal);
	LPVGridG = (fs_in.flux.g/PI)*evalSH(fs_in.Normal);
	LPVGridB = (fs_in.flux.b/PI)*evalSH(fs_in.Normal);


	//几何注入
	vec4 viewPos = u_LightViewMat * vec4(fs_in.WorldPos, 1.0);
	float surfelArea = calculateSurfelAreaLightOrtho(viewPos.xyz);
	float BlockingPotencial = calculateBlockingPotencial(surfelArea, u_LightDir, fs_in.WorldPos);
	SHCoeffGV = evalSH(fs_in.WorldPos) * BlockingPotencial;

}