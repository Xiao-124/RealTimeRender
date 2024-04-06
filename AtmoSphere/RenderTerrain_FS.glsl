#version 430 core



layout (std430, binding = 0) buffer mConstantBufferCPU
{
	mat4 gViewProjMat;
	vec4 gColor;
	vec3 gSunIlluminance;
	int gScatteringMaxPathDepth;
	uvec2 gResolution;
	float gFrameTimeSec;
	float gTimeSec;
	uvec2 gMouseLastDownPos;
	uint gFrameId;
	uint gTerrainResolution;
	float gScreenshotCaptureActive;
	vec2 RayMarchMinMaxSPP;
	vec2 pad;
};


layout (std430, binding = 1) buffer SkyAtmosphereConstantBufferStructure
{
	//
	// From AtmosphereParameters
	//

	vec3	solar_irradiance;
	float	sun_angular_radius;

	vec3	absorption_extinction;
	float	mu_s_min;

	vec3	rayleigh_scattering;
	float	mie_phase_function_g;

	vec3	mie_scattering;
	float	bottom_radius;

	vec3	mie_extinction;
	float	top_radius;

	vec3	mie_absorption;
	float	pad00;

	vec3	ground_albedo;
	float   pad0;

	vec4 rayleigh_density[3];
	vec4 mie_density[3];
	vec4 absorption_density[3];

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

	vec3 SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
	float  pad3;
	vec3 SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
	float  pad4;

	//
	// Other globals
	//
	mat4 gSkyViewProjMat;
	mat4 gSkyInvViewProjMat;
	mat4 gSkyInvProjMat;
	mat4 gSkyInvViewMat;
	mat4 gShadowmapViewProjMat;

	vec3 camera;
	float  pad5;
	vec3 sun_direction;
	float  pad6;
	vec3 view_ray;
	float  pad7;

	float MultipleScatteringFactor;
	float MultiScatteringLUTRes;
	float pad9;
	float pad10;
};

struct AtmosphereParameters
{
	// Radius of the planet (center to ground)
	float BottomRadius;
	// Maximum considered atmosphere height (center to atmosphere top)
	float TopRadius;

	// Rayleigh scattering exponential distribution scale in the atmosphere
	float RayleighDensityExpScale;
	// Rayleigh scattering coefficients
	vec3 RayleighScattering;

	// Mie scattering exponential distribution scale in the atmosphere
	float MieDensityExpScale;
	// Mie scattering coefficients
	vec3 MieScattering;
	// Mie extinction coefficients
	vec3 MieExtinction;
	// Mie absorption coefficients
	vec3 MieAbsorption;
	// Mie phase function excentricity
	float MiePhaseG;

	// Another medium type in the atmosphere
	float AbsorptionDensity0LayerWidth;
	float AbsorptionDensity0ConstantTerm;
	float AbsorptionDensity0LinearTerm;
	float AbsorptionDensity1ConstantTerm;
	float AbsorptionDensity1LinearTerm;
	// This other medium only absorb light, e.g. useful to represent ozone in the earth atmosphere
	vec3 AbsorptionExtinction;

	// The albedo of the ground.
	vec3 GroundAlbedo;
};

AtmosphereParameters GetAtmosphereParameters()
{
	AtmosphereParameters Parameters;
	Parameters.AbsorptionExtinction = absorption_extinction;

	// Traslation from Bruneton2017 parameterisation.
	Parameters.RayleighDensityExpScale = rayleigh_density[1].w;
	Parameters.MieDensityExpScale = mie_density[1].w;
	Parameters.AbsorptionDensity0LayerWidth = absorption_density[0].x;
	Parameters.AbsorptionDensity0ConstantTerm = absorption_density[1].x;
	Parameters.AbsorptionDensity0LinearTerm = absorption_density[0].w;
	Parameters.AbsorptionDensity1ConstantTerm = absorption_density[2].y;
	Parameters.AbsorptionDensity1LinearTerm = absorption_density[2].x;

	Parameters.MiePhaseG = mie_phase_function_g;
	Parameters.RayleighScattering = rayleigh_scattering;
	Parameters.MieScattering = mie_scattering;
	Parameters.MieAbsorption = mie_absorption;
	Parameters.MieExtinction = mie_extinction;
	Parameters.GroundAlbedo = ground_albedo;
	Parameters.BottomRadius = bottom_radius;
	Parameters.TopRadius = top_radius;
	return Parameters;
}


void LutTransmittanceParamsToUv(AtmosphereParameters Atmosphere, in float viewHeight, in float viewZenithCosAngle, out vec2 uv)
{
	float H = sqrt(max(0.0f, Atmosphere.TopRadius * Atmosphere.TopRadius - Atmosphere.BottomRadius * Atmosphere.BottomRadius));
	float rho = sqrt(max(0.0f, viewHeight * viewHeight - Atmosphere.BottomRadius * Atmosphere.BottomRadius));

	float discriminant = viewHeight * viewHeight * (viewZenithCosAngle * viewZenithCosAngle - 1.0) + Atmosphere.TopRadius * Atmosphere.TopRadius;
	float d = max(0.0, (-viewHeight * viewZenithCosAngle + sqrt(discriminant))); // Distance to atmosphere boundary

	float d_min = Atmosphere.TopRadius - viewHeight;
	float d_max = rho + H;
	float x_mu = (d - d_min) / (d_max - d_min);
	float x_r = rho / H;

	uv = vec2(x_mu, x_r);
	//uv = vec2(fromUnitToSubUvs(uv.x, TRANSMITTANCE_TEXTURE_WIDTH), fromUnitToSubUvs(uv.y, TRANSMITTANCE_TEXTURE_HEIGHT)); // No real impact so off
}


in vec4 vs_position;
in vec4 vs_Uvs;
in vec4 vs_WorldPos;
in vec3 vs_color;
in vec3 vs_normal;

uniform sampler2D TransmittanceLutTexture;
out vec4 FragColor;

void main()
{
	float NoL = max(0.0, dot(sun_direction, normalize(vs_normal)));
	float sunShadow = 1.0f;

	AtmosphereParameters Atmosphere = GetAtmosphereParameters();
	vec3 P0 = vs_WorldPos.xyz / vs_WorldPos.w + vec3(0, 0, Atmosphere.BottomRadius);
	float viewHeight = length(P0);
	const vec3 UpVector = P0 / viewHeight;
	float viewZenithCosAngle = dot(sun_direction, UpVector);
	vec2 uv;
	LutTransmittanceParamsToUv(Atmosphere, viewHeight, viewZenithCosAngle, uv);
	const vec3 trans = textureLod(TransmittanceLutTexture, uv, 0).rgb;
	FragColor =  vec4(vs_color * sunShadow * NoL * trans, 1);
}