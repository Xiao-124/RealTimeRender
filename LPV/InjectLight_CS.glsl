#version 430 core

#define LOCAL_GROUP_SIZE 32


#define SH_C0 0.282094792 
#define SH_C1 0.488602512 

#define SH_cosLobe_C0 0.886226925 
#define SH_cosLobe_C1 1.02332671 

// 使用定点数存储小数, 保留小数点后 5 位
// 因为 compute shader 的 InterlockedAdd 不支持 float
#define FIXED_SCALE 100000.0
int EncodeFloatToInt(float x)
{
    return int(x * FIXED_SCALE);
}
float DecodeFloatFromInt(int x)
{
    return float(x) / FIXED_SCALE;
}

//#extension GL_EXT_shader_atomic_float : require
//#extension GL_EXT_shader_atomic_fp16_vector : require

//#extension GL_NV_shader_atomic_float : require
//#extension GL_NV_shader_atomic_fp16_vector : require
//#extension GL_NV_gpu_shader5 : require

layout(rgba16f, binding = 0) uniform image3D LPVGridR_;
layout(rgba16f, binding = 1) uniform image3D LPVGridG_;
layout(rgba16f, binding = 2) uniform image3D LPVGridB_;
layout(rgba16f, binding = 3) uniform image3D GeometryVolume_;

uniform int u_RSMResolution;
uniform float u_RSMArea = 4.0;
uniform vec3 u_LightDir;
uniform float u_CellSize;
uniform vec3 u_MinAABB;

vec4 evalSH_direct(vec3 dir) 
{	
	return vec4(SH_C0, -SH_C1 * dir.y, SH_C1 * dir.z, -SH_C1 * dir.x);
}

ivec3 convertPointToGridIndex(vec3 vPos) 
{
	return ivec3((vPos - u_MinAABB) / u_CellSize);
}

float calculateSurfelAreaLightOrtho(vec3 lightPos) 
{
	return (u_RSMArea)/(u_RSMResolution * u_RSMResolution);
}

vec4 evalCosineLobeToDir(vec3 dir) 
{
	return vec4(SH_cosLobe_C0, -SH_cosLobe_C1 * dir.y, SH_cosLobe_C1 * dir.z, -SH_cosLobe_C1 * dir.x);
}

float calculateBlockingPotencial(float surfelArea, vec3 dir, vec3 normal) 
{
	return clamp((surfelArea * clamp(dot(normal,dir),0.0,1.0))/(u_CellSize * u_CellSize),0.0,1.0);
}

uniform sampler2D u_RSMRadiantFluxTexture;
uniform sampler2D u_RSMPositionTexture;
uniform sampler2D u_RSMNormalTexture;
uniform mat4 u_LightViewMat;


layout (local_size_x = LOCAL_GROUP_SIZE, local_size_y = LOCAL_GROUP_SIZE) in;


void main()
{
	
	//光线注入
	ivec2 RSMCoords = ivec2(gl_GlobalInvocationID.xy);
	vec3 v2f_posFromRSM = texelFetch(u_RSMPositionTexture, RSMCoords, 0).rgb;
	vec3 v2f_normalFromRSM = texelFetch(u_RSMNormalTexture, RSMCoords,0).rgb;
	vec4 v2f_fluxFromRSM = texelFetch(u_RSMRadiantFluxTexture, RSMCoords,0);

	ivec3 v2f_volumeCellIndex = convertPointToGridIndex(v2f_posFromRSM);
	vec3 CellCenter = (v2f_volumeCellIndex - 0.5) * u_CellSize + u_MinAABB;
	vec3 v2f_vplToCell = normalize(v2f_posFromRSM - CellCenter);

	vec4 SHCoeffsR = evalSH_direct(v2f_vplToCell) * v2f_fluxFromRSM.r;
	vec4 SHCoeffsG = evalSH_direct(v2f_vplToCell) * v2f_fluxFromRSM.g;
	vec4 SHCoeffsB = evalSH_direct(v2f_vplToCell) * v2f_fluxFromRSM.b;

	//几何注入
	vec4 viewPos = u_LightViewMat * vec4(v2f_posFromRSM, 1.0);
	float surfelArea = calculateSurfelAreaLightOrtho(viewPos.xyz);
	float BlockingPotencial = calculateBlockingPotencial(surfelArea, u_LightDir, v2f_normalFromRSM);
	vec4 SHCoeffGV = evalCosineLobeToDir(v2f_normalFromRSM) * BlockingPotencial;

	groupMemoryBarrier();
	memoryBarrierImage();
	vec4 R = imageLoad(LPVGridR_,v2f_volumeCellIndex);
	vec4 G = imageLoad(LPVGridG_,v2f_volumeCellIndex);
	vec4 B = imageLoad(LPVGridB_,v2f_volumeCellIndex);	
	vec4 V = imageLoad(GeometryVolume_, v2f_volumeCellIndex);
	
	R += SHCoeffsR;
	G += SHCoeffsG;
	B += SHCoeffsB;
	V += SHCoeffGV;
	
	imageStore(LPVGridR_, v2f_volumeCellIndex, R);
	imageStore(LPVGridG_, v2f_volumeCellIndex, G);
	imageStore(LPVGridB_, v2f_volumeCellIndex, B);
	imageStore(GeometryVolume_, v2f_volumeCellIndex, V);

	groupMemoryBarrier();
	memoryBarrierImage();

	//imageAtomicAdd(LPVGridR_,v2f_volumeCellIndex, f16vec4(SHCoeffsR));
	//imageAtomicAdd(LPVGridG_,v2f_volumeCellIndex, f16vec4(SHCoeffsG));
	//imageAtomicAdd(LPVGridB_,v2f_volumeCellIndex, f16vec4(SHCoeffsB));
	//imageAtomicAdd(GeometryVolume_, v2f_volumeCellIndex, f16vec4(SHCoeffGV));

	//imageAtomicAdd(LPVGridR_,v2f_volumeCellIndex, SHCoeffsR);
	//imageAtomicAdd(LPVGridG_,v2f_volumeCellIndex, SHCoeffsG);
	//imageAtomicAdd(LPVGridB_,v2f_volumeCellIndex, SHCoeffsB);
	//imageAtomicAdd(GeometryVolume_, v2f_volumeCellIndex, SHCoeffGV);


}