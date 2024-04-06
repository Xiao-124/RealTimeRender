#pragma once

#include <GLM/glm.hpp>


using uint32 = unsigned int;
struct DensityProfileLayer
{
	float width;
	float exp_term;
	float exp_scale;
	float linear_term;
	float constant_term;
};


struct DensityProfile
{
	DensityProfileLayer layers[2];
};

struct AtmosphereParameters
{
	//大气顶部的太阳辐照度。
	glm::vec3 solar_irradiance;
	//太阳角
	float sun_angular_radius;

	float bottom_radius;
	float top_radius;

	//空气分子的密度分布
	DensityProfile rayleigh_density;


	glm::vec3 rayleigh_scattering;

	//气溶胶的密度分布
	DensityProfile mie_density;
	glm::vec3 mie_scattering;

	//气溶胶在其密度所在高度的消光系数
	glm::vec3 mie_extinction;

	//Cornette-Shanks 相位函数的不对称参数
	float mie_phase_function_g;

	//吸收光的空气分子的密度分布
	DensityProfile absorption_density;
	glm::vec3 absorption_extinction;

	// 地面的平均反照率。
	glm::vec3 ground_albedo;

	//大气散射的最大太阳天顶角的余弦
	float mu_s_min;
};


typedef AtmosphereParameters AtmosphereInfo;

struct LookUpTablesInfo
{
#if 1
	uint32 TRANSMITTANCE_TEXTURE_WIDTH = 256;
	uint32 TRANSMITTANCE_TEXTURE_HEIGHT = 64;

	uint32 SCATTERING_TEXTURE_R_SIZE = 32;
	uint32 SCATTERING_TEXTURE_MU_SIZE = 128;
	uint32 SCATTERING_TEXTURE_MU_S_SIZE = 32;
	uint32 SCATTERING_TEXTURE_NU_SIZE = 8;

	uint32 IRRADIANCE_TEXTURE_WIDTH = 64;
	uint32 IRRADIANCE_TEXTURE_HEIGHT = 16;
#else
	uint32 TRANSMITTANCE_TEXTURE_WIDTH = 64;
	uint32 TRANSMITTANCE_TEXTURE_HEIGHT = 16;

	uint32 SCATTERING_TEXTURE_R_SIZE = 16;
	uint32 SCATTERING_TEXTURE_MU_SIZE = 16;
	uint32 SCATTERING_TEXTURE_MU_S_SIZE = 16;
	uint32 SCATTERING_TEXTURE_NU_SIZE = 4;

	uint32 IRRADIANCE_TEXTURE_WIDTH = 32;
	uint32 IRRADIANCE_TEXTURE_HEIGHT = 8;
#endif

	// Derived from above
	uint32 SCATTERING_TEXTURE_WIDTH = 0xDEADBEEF;
	uint32 SCATTERING_TEXTURE_HEIGHT = 0xDEADBEEF;
	uint32 SCATTERING_TEXTURE_DEPTH = 0xDEADBEEF;

	void updateDerivedData()
	{
		SCATTERING_TEXTURE_WIDTH = SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE;
		SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;
		SCATTERING_TEXTURE_DEPTH = SCATTERING_TEXTURE_R_SIZE;
	}

	LookUpTablesInfo() { updateDerivedData(); }
};



struct SkyAtmosphereConstantBufferStructure
{
	//
	// From AtmosphereParameters
	//

	glm::vec3 solar_irradiance;
	float sun_angular_radius;

	glm::vec3 absorption_extinction;
	float mu_s_min;

	glm::vec3 rayleigh_scattering;
	float mie_phase_function_g;

	glm::vec3 mie_scattering;
	float bottom_radius;

	glm::vec3 mie_extinction;
	float top_radius;

	glm::vec3 mie_absorption;
	float pad00;

	glm::vec3 ground_albedo;
	float pad0;

	float rayleigh_density[12];
	float mie_density[12];
	float absorption_density[12];

	//
	// Add generated static header constant
	//

	int TRANSMITTANCE_TEXTURE_WIDTH;
	int TRANSMITTANCE_TEXTURE_HEIGHT;
	int IRRADIANCE_TEXTURE_WIDTH;
	int IRRADIANCE_TEXTURE_HEIGHT;

	int SCATTERING_TEXTURE_R_SIZE;
	int SCATTERING_TEXTURE_MU_SIZE;
	int SCATTERING_TEXTURE_MU_S_SIZE;
	int SCATTERING_TEXTURE_NU_SIZE;

	glm::vec3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
	float  pad3;
	glm::vec3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
	float  pad4;

	//
	// Other globals
	//
	glm::mat4 gSkyViewProjMat;
	glm::mat4 gSkyInvViewProjMat;
	glm::mat4 gSkyInvProjMat;
	glm::mat4 gSkyInvViewMat;
	glm::mat4 gShadowmapViewProjMat;

	glm::vec3 camera;
	float  pad5;
	glm::vec3 sun_direction;
	float  pad6;
	glm::vec3 view_ray;
	float  pad7;

	float MultipleScatteringFactor;
	float MultiScatteringLUTRes;
	float pad9;
	float pad10;
};


void SetupEarthAtmosphere(AtmosphereParameters& info);
