#include "AtmoSpherePass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
//#include "ResourceManager.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include "windows.h"
#define PI 3.1415926

using GlslVec3 = glm::vec3;

auto EqualFloat3 = [](const GlslVec3 a, const GlslVec3 b) {return a.x == b.x && a.y == b.y && a.z == b.z; };
GlslVec3 CreateGlslVec3(float x, float y, float z) {GlslVec3 vec = { x, y, z }; return vec; };
float length3(GlslVec3 v) {return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); };
GlslVec3 normalize3(GlslVec3 v, float l) {GlslVec3 r; r.x = v.x / l, r.y = v.y / l, r.z = v.z / l; return r; };
GlslVec3 scale3(GlslVec3 v, float l) {GlslVec3 r; r.x = v.x * l, r.y = v.y * l, r.z = v.z * l; return r; };
GlslVec3 sub3(GlslVec3 a, GlslVec3 b) {GlslVec3 r; r.x = a.x - b.x; r.y = a.y - b.y; r.z = a.z - b.z; return r; };
GlslVec3 add3(GlslVec3 a, GlslVec3 b) {GlslVec3 r; r.x = a.x + b.x; r.y = a.y + b.y; r.z = a.z + b.z; return r; };
GlslVec3 MaxZero3(GlslVec3 a) {GlslVec3 r; r.x = a.x > 0.0f ? a.x : 0.0f; r.y = a.y > 0.0f ? a.y : 0.0f; r.z = a.z > 0.0f ? a.z : 0.0f; return r; };

#undef max
#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

// This is the resolution of the terrain. Render tile by tile using instancing... Bad but it works and this is not important for what I need to do.
const static int TerrainResolution = 512;

auto createTexture2dFromExr = [&](const char* filename)
{
	const char* exrErrorStr = nullptr;

	float* rgba = nullptr;
	int width = -1;
	int height = -1;
	const char* err = nullptr;
	int exrError = LoadEXR(&rgba, &width, &height, filename, &err);
	auto tex = std::make_shared<ElayGraphics::STexture>();
	tex->InternalFormat = GL_RGBA32F;
	tex->ExternalFormat = GL_RGBA;
	tex->Width = width;
	tex->Height = height;
	tex->DataType = GL_FLOAT;
	tex->pDataSet.resize(1);
	tex->pDataSet[0] = rgba;
	tex->Type4WrapR = GL_CLAMP_TO_EDGE;
	tex->Type4WrapS = GL_CLAMP_TO_EDGE;
	tex->Type4WrapT = GL_CLAMP_TO_EDGE;

	genTexture(tex);

	free(rgba);
	return tex;
};


SkyAtmosphereConstantBufferStructure AtmoSpherePass::updateSkyAtmosphereConstant()
{

	// Constant buffer update
	{
		SkyAtmosphereConstantBufferStructure cb;
		memset(&cb, 0xBA, sizeof(SkyAtmosphereConstantBufferStructure));

		cb.solar_irradiance = AtmosphereInfos.solar_irradiance;
		cb.sun_angular_radius = AtmosphereInfos.sun_angular_radius;
		cb.absorption_extinction = AtmosphereInfos.absorption_extinction;
		cb.mu_s_min = AtmosphereInfos.mu_s_min;

		memcpy(cb.rayleigh_density, &AtmosphereInfos.rayleigh_density, sizeof(AtmosphereInfos.rayleigh_density));
		memcpy(cb.mie_density, &AtmosphereInfos.mie_density, sizeof(AtmosphereInfos.mie_density));
		memcpy(cb.absorption_density, &AtmosphereInfos.absorption_density, sizeof(AtmosphereInfos.absorption_density));

		cb.mie_phase_function_g = AtmosphereInfos.mie_phase_function_g;
		cb.rayleigh_scattering = AtmosphereInfos.rayleigh_scattering;
		const float RayleighScatScale = 1.0f;
		cb.rayleigh_scattering.x *= RayleighScatScale;
		cb.rayleigh_scattering.y *= RayleighScatScale;
		cb.rayleigh_scattering.z *= RayleighScatScale;
		cb.mie_scattering = AtmosphereInfos.mie_scattering;
		cb.mie_absorption = MaxZero3(sub3(AtmosphereInfos.mie_extinction, AtmosphereInfos.mie_scattering));
		cb.mie_extinction = AtmosphereInfos.mie_extinction;
		cb.ground_albedo = AtmosphereInfos.ground_albedo;
		cb.bottom_radius = AtmosphereInfos.bottom_radius;
		cb.top_radius = AtmosphereInfos.top_radius;
		cb.MultipleScatteringFactor = currentMultipleScatteringFactor;
		cb.MultiScatteringLUTRes = MultiScatteringLUTRes;

		//
		cb.TRANSMITTANCE_TEXTURE_WIDTH = LutsInfo.TRANSMITTANCE_TEXTURE_WIDTH;
		cb.TRANSMITTANCE_TEXTURE_HEIGHT = LutsInfo.TRANSMITTANCE_TEXTURE_HEIGHT;
		cb.IRRADIANCE_TEXTURE_WIDTH = LutsInfo.IRRADIANCE_TEXTURE_WIDTH;
		cb.IRRADIANCE_TEXTURE_HEIGHT = LutsInfo.IRRADIANCE_TEXTURE_HEIGHT;
		cb.SCATTERING_TEXTURE_R_SIZE = LutsInfo.SCATTERING_TEXTURE_R_SIZE;
		cb.SCATTERING_TEXTURE_MU_SIZE = LutsInfo.SCATTERING_TEXTURE_MU_SIZE;
		cb.SCATTERING_TEXTURE_MU_S_SIZE = LutsInfo.SCATTERING_TEXTURE_MU_S_SIZE;
		cb.SCATTERING_TEXTURE_NU_SIZE = LutsInfo.SCATTERING_TEXTURE_NU_SIZE;
		cb.SKY_SPECTRAL_RADIANCE_TO_LUMINANCE = glm::vec3(114974.916437f, 71305.954816f, 65310.548555f); // Not used if using LUTs as transfert
		cb.SUN_SPECTRAL_RADIANCE_TO_LUMINANCE = glm::vec3(98242.786222f, 69954.398112f, 66475.012354f);  // idem

		//
		cb.gSkyViewProjMat = mViewProjMat;
		cb.gSkyInvViewProjMat = glm::inverse(mViewProjMat);
		cb.gSkyInvProjMat = glm::inverse(mProjMat);
		cb.gSkyInvViewMat = glm::inverse(mViewMat);
		cb.camera = mCamPosFinal;
		cb.view_ray = mViewDir;
		cb.sun_direction = glm::vec3(0.0f,0.9f,0.4f);

		//SkyAtmosphereBuffer->update(cb);
		return cb;
	}

}


AtmoSpherePass::AtmoSpherePass(const std::string& vPassName, int vExcutionOrder) :IRenderPass(vPassName, vExcutionOrder)
{



}

AtmoSpherePass::~AtmoSpherePass()
{


}

void AtmoSpherePass::initV()
{
	TransmittanceShader = std::make_shared<CShader>("RenderTransmittanceLut_VS.glsl", "RenderTransmittanceLut_FS.glsl");
	multiScatterShader = std::make_shared<CShader>("NewMultiScatt_CS.glsl");
	skyLutShader = std::make_shared<CShader>("SkyViewLut_VS.glsl", "SkyViewLut_FS.glsl");
	cameraVolumeShader = std::make_shared<CShader>("RenderCameraVolume_VS.glsl",  "RenderCameraVolume_FS.glsl");
	rayMarchingShader = std::make_shared<CShader>("RenderRayMarching_VS.glsl", "RenderRayMarching_FS.glsl");
	postShader = std::make_shared<CShader>("PostProcess_VS.glsl", "PostProcess_FS.glsl");
	terrianShader = std::make_shared<CShader>("RenderTerrain_VS.glsl", "RenderTerrain_FS.glsl");
	
	
	auto TerrianColor = std::make_shared<ElayGraphics::STexture>();
	genTexture(TerrianColor);
	auto TerrianDepth = std::make_shared<ElayGraphics::STexture>();
	TerrianDepth->InternalFormat = GL_DEPTH_COMPONENT32F;
	TerrianDepth->ExternalFormat = GL_DEPTH_COMPONENT;
	TerrianDepth->DataType = GL_FLOAT;
	TerrianDepth->Type4MinFilter = GL_LINEAR;
	TerrianDepth->Type4MagFilter = GL_LINEAR;
	//TextureConfig4Depth->Width = TextureConfig4Depth->Height = m_ShadowmapResolution;
	TerrianDepth->Type4WrapS = TerrianDepth->Type4WrapT = GL_CLAMP_TO_BORDER;
	TerrianDepth->BorderColor = { 0,0,0,0 };
	TerrianDepth->TextureAttachmentType = ElayGraphics::STexture::ETextureAttachmentType::DepthTexture;
	genTexture(TerrianDepth);
	TerrianFBO = genFBO({ TerrianColor, TerrianDepth });


	auto mBlueNoise2dTex = createTexture2dFromExr("../Texture/bluenoise.exr");		// I do not remember where this noise texture comes from.
	auto mTerrainHeightmapTex = createTexture2dFromExr("../Texture/heightmap1.exr");
	


	auto TextureConfig4Albedo = std::make_shared<ElayGraphics::STexture>();
	TextureConfig4Albedo->InternalFormat = GL_RGBA32F;
	TextureConfig4Albedo->ExternalFormat = GL_RGBA;
	TextureConfig4Albedo->DataType = GL_FLOAT;
	TextureConfig4Albedo->Width = LutsInfo.TRANSMITTANCE_TEXTURE_WIDTH;
	TextureConfig4Albedo->Height = LutsInfo.TRANSMITTANCE_TEXTURE_HEIGHT;
	TextureConfig4Albedo->Type4WrapS = GL_CLAMP_TO_EDGE;
	TextureConfig4Albedo->Type4WrapT = GL_CLAMP_TO_EDGE;
	TextureConfig4Albedo->Type4WrapR = GL_CLAMP_TO_EDGE;
	genTexture(TextureConfig4Albedo);
	TransmittanceShader->activeShader();
	TransmittanceFBO = genFBO({ TextureConfig4Albedo });

	ElayGraphics::ResourceManager::registerSharedData("TransmittanceLutTexture", TextureConfig4Albedo);
	multiScatterShader->activeShader();
	multiScatterShader->setTextureUniformValue("TransmittanceLutTexture", TextureConfig4Albedo);


	terrianShader->activeShader();
	terrianShader->setTextureUniformValue("TerrainHeightmapTex", mTerrainHeightmapTex);
	terrianShader->setTextureUniformValue("TransmittanceLutTexture", TextureConfig4Albedo);


	auto multiScatterTexture = std::make_shared<ElayGraphics::STexture>();
	multiScatterTexture->InternalFormat = GL_RGBA32F;
	multiScatterTexture->ExternalFormat = GL_RGBA;
	multiScatterTexture->DataType = GL_FLOAT;
	multiScatterTexture->Width = LutsInfo.SCATTERING_TEXTURE_R_SIZE;
	multiScatterTexture->Height = LutsInfo.SCATTERING_TEXTURE_R_SIZE;
	multiScatterTexture->Type4WrapS = GL_CLAMP_TO_EDGE;
	multiScatterTexture->Type4WrapT = GL_CLAMP_TO_EDGE;
	multiScatterTexture->Type4WrapR = GL_CLAMP_TO_EDGE;
	multiScatterTexture->ImageBindUnit = 0;
	genTexture(multiScatterTexture);

	skyLutShader->activeShader();
	skyLutShader->setTextureUniformValue("TransmittanceLutTexture", TextureConfig4Albedo);
	skyLutShader->setTextureUniformValue("MultiScatTexture", multiScatterTexture);


	cameraVolumeShader->activeShader();
	cameraVolumeShader->setTextureUniformValue("TransmittanceLutTexture", TextureConfig4Albedo);
	cameraVolumeShader->setTextureUniformValue("MultiScatTexture", multiScatterTexture);


	auto skyLutTexture = std::make_shared<ElayGraphics::STexture>();
	skyLutTexture->InternalFormat = GL_RGB16F;
	skyLutTexture->ExternalFormat = GL_RGB;
	skyLutTexture->DataType = GL_FLOAT;
	skyLutTexture->Width = 192;
	skyLutTexture->Height = 108;
	genTexture(skyLutTexture);
	SkyLutFBO = genFBO({ skyLutTexture });



	auto cameraVolumeTexture = std::make_shared<ElayGraphics::STexture>();
	cameraVolumeTexture->TextureType = ElayGraphics::STexture::ETextureType::Texture3D;

	//cameraVolumeTexture->TextureType = ElayGraphics::STexture::ETextureType::Texture2DArray;
	cameraVolumeTexture->InternalFormat = GL_RGBA16F;
	cameraVolumeTexture->ExternalFormat = GL_RGBA;
	cameraVolumeTexture->DataType = GL_FLOAT;
	cameraVolumeTexture->Width = LutsInfo.SCATTERING_TEXTURE_R_SIZE;
	cameraVolumeTexture->Height = LutsInfo.SCATTERING_TEXTURE_R_SIZE;
	cameraVolumeTexture->Depth = LutsInfo.SCATTERING_TEXTURE_R_SIZE;
	genTexture(cameraVolumeTexture);
	ElayGraphics::ResourceManager::registerSharedData("cameraVolumeTexture", cameraVolumeTexture);
	//m_FBO3 = genFBO({ cameraVolumeTexture });


	rayMarchingShader->activeShader();
	rayMarchingShader->setTextureUniformValue("ViewDepthTexture", TerrianDepth);
	rayMarchingShader->setTextureUniformValue("SkyViewLutTexture", skyLutTexture);
	rayMarchingShader->setTextureUniformValue("AtmosphereCameraScatteringVolume", cameraVolumeTexture);
	

	auto rayMarchingTexture = std::make_shared<ElayGraphics::STexture>();
	rayMarchingTexture->InternalFormat = GL_RGBA16F;
	rayMarchingTexture->ExternalFormat = GL_RGBA;
	rayMarchingTexture->DataType = GL_FLOAT;
	genTexture(rayMarchingTexture);
	RayMarchingFBO = genFBO({ rayMarchingTexture });

	postShader->activeShader();
	postShader->setTextureUniformValue("resultTexture", rayMarchingTexture);
	//mViewProjMat = ElayGraphics::Camera::getMainCameraProjectionMatrix() * ElayGraphics::Camera::getMainCameraViewMatrix();
	
	mCamPosFinal = { 0.0, 0.0f, 0.0f };
	glm::vec3 mViewDir = { 0,1,0 };

	mCamPosFinal.x += mViewDir.x * uiCamForward;
	mCamPosFinal.y += mViewDir.y * uiCamForward;
	mCamPosFinal.z += mViewDir.z * uiCamForward;
	mCamPosFinal.z += uiCamHeight;

	glm::vec3 focusPosition2 = glm::vec3(mCamPosFinal.x + mViewDir.x, mCamPosFinal.y + mViewDir.y, mCamPosFinal.z + mViewDir.z);
	glm::vec3 eyePosition = mCamPosFinal;
	glm::vec3 focusPosition = focusPosition2;
	glm::vec3 upDirection = glm::vec3( 0.0f, 0.0f, 1.0f);	// Unreal z-up



	mProjMat =  glm::perspectiveFovLH(66.6f * 3.14159f / 180.0f, float(ElayGraphics::WINDOW_KEYWORD::getWindowWidth()), 
		float(ElayGraphics::WINDOW_KEYWORD::getWindowHeight()), 0.1f, 20000.0f);
	mViewMat = glm::lookAtLH(eyePosition, focusPosition, upDirection);

	mViewProjMat = mProjMat * mViewMat;
	//mViewProjMat = mViewMat * mProjMat;

	mConstantBufferCPU.gViewProjMat = mViewProjMat;
	mConstantBufferCPU.gColor = { 0.0, 1.0, 1.0, 1.0 };
	mConstantBufferCPU.gResolution[0] = uint32(ElayGraphics::WINDOW_KEYWORD::getWindowWidth());
	mConstantBufferCPU.gResolution[1] = uint32(ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	mConstantBufferCPU.gSunIlluminance = { 1.0f * mSunIlluminanceScale, 1.0f * mSunIlluminanceScale, 1.0f * mSunIlluminanceScale };
	mConstantBufferCPU.gScatteringMaxPathDepth = NumScatteringOrder;
	mConstantBufferCPU.gTerrainResolution = TerrainResolution;
	mConstantBufferCPU.RayMarchMinMaxSPP[0] = 4;
	mConstantBufferCPU.RayMarchMinMaxSPP[1] = 14;
	mConstantBufferCPU.gMouseLastDownPos[0] = 0;
	mConstantBufferCPU.gMouseLastDownPos[1] = 0;
	static ULONGLONG LastTime = GetTickCount64();
	static float ElapsedTimeSec = 0;
	const ULONGLONG CurTime = GetTickCount64();
	mConstantBufferCPU.gFrameTimeSec = float(CurTime - LastTime) / 1000.0f;
	mConstantBufferCPU.gTimeSec = ElapsedTimeSec;
	mConstantBufferCPU.gFrameId = mFrameId;
	uiViewRayMarchMaxSPP = uiViewRayMarchMinSPP >= uiViewRayMarchMaxSPP ? uiViewRayMarchMinSPP + 1 : uiViewRayMarchMaxSPP;
	mConstantBufferCPU.RayMarchMinMaxSPP[0] = float(uiViewRayMarchMinSPP);
	mConstantBufferCPU.RayMarchMinMaxSPP[1] = float(uiViewRayMarchMaxSPP);

	int s = sizeof(CommonConstantBufferStructure);
	mConstantBufferCPU.gScreenshotCaptureActive = 0; // Make sure the terrain or sundisk are not taken into account to focus on the most important part: atmosphere.
	ElapsedTimeSec += mConstantBufferCPU.gFrameTimeSec;
	
	//genBuffer(GL_SHADER_STORAGE_BUFFER, sizeof(CommonConstantBufferStructure),
	//	&mConstantBufferCPU, GL_STATIC_DRAW, 0);
	CommonConstantBuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, sizeof(CommonConstantBufferStructure),
		&mConstantBufferCPU, GL_STATIC_DRAW, 0);

	SetupEarthAtmosphere(AtmosphereInfos);
	SkyAtmosphereConstantBufferStructure cb = updateSkyAtmosphereConstant();
	AtmosphereBuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, sizeof(SkyAtmosphereConstantBufferStructure),
		&cb, GL_STATIC_DRAW, 1);
}

void AtmoSpherePass::_updateCommonConstantBuffer()
{
	uiCamHeight = ElayGraphics::ResourceManager::getSharedDataByName<float>("uiCamHeight");
	mCamPosFinal = { 0.0, 0.0f, 0.0f };
	glm::vec3 mViewDir = { 0,1,0 };
	mCamPosFinal.x += mViewDir.x * uiCamForward;
	mCamPosFinal.y += mViewDir.y * uiCamForward;
	mCamPosFinal.z += mViewDir.z * uiCamForward;
	mCamPosFinal.z += uiCamHeight;

	glm::vec3 focusPosition2 = glm::vec3(mCamPosFinal.x + mViewDir.x, mCamPosFinal.y + mViewDir.y, mCamPosFinal.z + mViewDir.z);
	glm::vec3 eyePosition = mCamPosFinal;
	glm::vec3 focusPosition = focusPosition2;
	glm::vec3 upDirection = glm::vec3(0.0f, 0.0f, 1.0f);	// Unreal z-up

	mProjMat = glm::perspectiveFovLH(66.6f * 3.14159f / 180.0f, float(ElayGraphics::WINDOW_KEYWORD::getWindowWidth()),
		float(ElayGraphics::WINDOW_KEYWORD::getWindowHeight()), 0.1f, 20000.0f);
	mViewMat = glm::lookAtLH(eyePosition, focusPosition, upDirection);
	mViewProjMat = mProjMat * mViewMat;
	mConstantBufferCPU.gViewProjMat = mViewProjMat;
	updateSSBOBuffer(CommonConstantBuffer, sizeof(mConstantBufferCPU), &mConstantBufferCPU, GL_STATIC_DRAW);
}






void AtmoSpherePass::renderTerrian()
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, TerrianFBO);
	glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	terrianShader->activeShader();
	glBindVertexArray(ElayGraphics::ResourceManager::getOrCreateScreenQuadVAO());
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, TerrainResolution * TerrainResolution);
	glBindVertexArray(0);
	glFlush();
}

void AtmoSpherePass::renderTransmittanceLut()
{
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, LutsInfo.TRANSMITTANCE_TEXTURE_WIDTH, LutsInfo.TRANSMITTANCE_TEXTURE_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, TransmittanceFBO);
	glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	TransmittanceShader->activeShader();
	drawQuad();
	glFlush();
}

void AtmoSpherePass::renderMultiScatter()
{
	multiScatterShader->activeShader();
	glDispatchCompute(LutsInfo.SCATTERING_TEXTURE_R_SIZE, LutsInfo.SCATTERING_TEXTURE_R_SIZE, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glFlush();
}

void AtmoSpherePass::renderSkyLut()
{
	glBindFramebuffer(GL_FRAMEBUFFER, SkyLutFBO);
	glViewport(0, 0, 192, 108);
	glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	skyLutShader->activeShader();
	drawQuad();
	glFlush();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AtmoSpherePass::renderCameraVolume()
{
	glViewport(0, 0, LutsInfo.SCATTERING_TEXTURE_R_SIZE, LutsInfo.SCATTERING_TEXTURE_R_SIZE);
	mConstantBufferCPU.gResolution[0] = LutsInfo.SCATTERING_TEXTURE_R_SIZE;
	mConstantBufferCPU.gResolution[1] = LutsInfo.SCATTERING_TEXTURE_R_SIZE;
	updateSSBOBuffer(CommonConstantBuffer, sizeof(mConstantBufferCPU), &mConstantBufferCPU, GL_STATIC_DRAW);
	cameraVolumeShader->activeShader();
	std::shared_ptr<ElayGraphics::STexture> cameraVolumeTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("cameraVolumeTexture");
	for (int i = 0; i < LutsInfo.SCATTERING_TEXTURE_R_SIZE; i++)
	{
		GLuint tempFBO;
		glGenFramebuffers(1, &(GLuint&)tempFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, tempFBO);
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, cameraVolumeTexture->TextureID, 0, i);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "Error::FBO:: Framebuffer Is Not Complete." << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
		}
		glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cameraVolumeShader->setIntUniformValue("ilayer", i);
		drawQuad();
		glFlush();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &tempFBO);
	}
	glFlush();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void AtmoSpherePass::renderRayMarching()
{

	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, RayMarchingFBO);
	glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mConstantBufferCPU.gResolution[0] = ElayGraphics::WINDOW_KEYWORD::getWindowWidth();
	mConstantBufferCPU.gResolution[1] = ElayGraphics::WINDOW_KEYWORD::getWindowHeight();
	updateSSBOBuffer(CommonConstantBuffer, sizeof(mConstantBufferCPU), &mConstantBufferCPU, GL_STATIC_DRAW);
	rayMarchingShader->activeShader();
	drawQuad();
	glFlush();
}



void AtmoSpherePass::updateV()
{
	_updateCommonConstantBuffer();
	SkyAtmosphereConstantBufferStructure cb = updateSkyAtmosphereConstant();
	updateSSBOBuffer(AtmosphereBuffer, sizeof(SkyAtmosphereConstantBufferStructure), &cb, GL_STATIC_DRAW);

	renderTerrian();
	renderTransmittanceLut();
	renderMultiScatter();

	renderSkyLut();
	renderCameraVolume();
	renderRayMarching();
	

	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	postShader->activeShader();
	drawQuad();
	glFlush();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
