#include "AtmosphereParameter.h"


void SetupEarthAtmosphere(AtmosphereParameters& info)
{
	// Values shown here are the result of integration over wavelength power spectrum integrated with paricular function.
	// Refer to https://github.com/ebruneton/precomputed_atmospheric_scattering for details.

	// All units in kilometers
	const float EarthBottomRadius = 6360.0f;
	const float EarthTopRadius = 6460.0f;   // 100km atmosphere radius, less edge visible and it contain 99.99% of the atmosphere medium https://en.wikipedia.org/wiki/K%C3%A1rm%C3%A1n_line
	const float EarthRayleighScaleHeight = 8.0f;
	const float EarthMieScaleHeight = 1.2f;

	// Sun - This should not be part of the sky model...
	//info.solar_irradiance = { 1.474000f, 1.850400f, 1.911980f };
	info.solar_irradiance = { 1.0f, 1.0f, 1.0f };	// Using a normalise sun illuminance. This is to make sure the LUTs acts as a transfert factor to apply the runtime computed sun irradiance over.
	info.sun_angular_radius = 0.004675f;

	// Earth
	info.bottom_radius = EarthBottomRadius;
	info.top_radius = EarthTopRadius;
	info.ground_albedo = { 0.0f, 0.0f, 0.0f };

	// Raleigh scattering
	info.rayleigh_density.layers[0] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	info.rayleigh_density.layers[1] = { 0.0f, 1.0f, -1.0f / EarthRayleighScaleHeight, 0.0f, 0.0f };
	info.rayleigh_scattering = { 0.005802f, 0.013558f, 0.033100f };		// 1/km

	// Mie scattering
	info.mie_density.layers[0] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	info.mie_density.layers[1] = { 0.0f, 1.0f, -1.0f / EarthMieScaleHeight, 0.0f, 0.0f };
	info.mie_scattering = { 0.003996f, 0.003996f, 0.003996f };			// 1/km
	info.mie_extinction = { 0.004440f, 0.004440f, 0.004440f };			// 1/km
	info.mie_phase_function_g = 0.8f;

	// Ozone absorption
	info.absorption_density.layers[0] = { 25.0f, 0.0f, 0.0f, 1.0f / 15.0f, -2.0f / 3.0f };
	info.absorption_density.layers[1] = { 0.0f, 0.0f, 0.0f, -1.0f / 15.0f, 8.0f / 3.0f };
	info.absorption_extinction = { 0.000650f, 0.001881f, 0.000085f };	// 1/km


	const double max_sun_zenith_angle = 3.141592653 * 120.0 / 180.0; // (use_half_precision_ ? 102.0 : 120.0) / 180.0 * kPi;
	info.mu_s_min = (float)cos(max_sun_zenith_angle);
}
