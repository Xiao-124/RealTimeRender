#include "TerrainRenderPass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#undef max
#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

// This is the resolution of the terrain. Render tile by tile using instancing... Bad but it works and this is not important for what I need to do.
const static int TerrainResolution = 512;

TerrainRenderPass::TerrainRenderPass(const std::string& vPassName, int vExcutionOrder) :IRenderPass(vPassName, vExcutionOrder)
{


}

TerrainRenderPass::~TerrainRenderPass()
{

}

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
	genTexture(tex);

	free(rgba);
	return tex;
};

void TerrainRenderPass::initV()
{
	m_pShader = std::make_shared<CShader>("TerrainShadowMap_VS.glsl", "TerrainShadowMap_FS.glsl");
	auto TextureConfig4Color = std::make_shared<ElayGraphics::STexture>();
	auto TextureConfig4Depth = std::make_shared<ElayGraphics::STexture>();
	genTexture(TextureConfig4Color);

	TextureConfig4Depth->InternalFormat = GL_DEPTH_COMPONENT32F;
	TextureConfig4Depth->ExternalFormat = GL_DEPTH_COMPONENT;
	TextureConfig4Depth->DataType = GL_FLOAT;
	TextureConfig4Depth->Type4MinFilter = GL_LINEAR;
	TextureConfig4Depth->Type4MagFilter = GL_LINEAR;
	//TextureConfig4Depth->Width = TextureConfig4Depth->Height = m_ShadowmapResolution;
	TextureConfig4Depth->Type4WrapS = TextureConfig4Depth->Type4WrapT = GL_CLAMP_TO_BORDER;
	TextureConfig4Depth->BorderColor = { 0,0,0,0 };
	TextureConfig4Depth->TextureAttachmentType = ElayGraphics::STexture::ETextureAttachmentType::DepthTexture;
	genTexture(TextureConfig4Depth);

	m_FBO = genFBO({ TextureConfig4Color, TextureConfig4Depth });


	auto mBlueNoise2dTex = createTexture2dFromExr("../Texture/bluenoise.exr");		// I do not remember where this noise texture comes from.
	auto mTerrainHeightmapTex = createTexture2dFromExr("../Texture/heightmap1.exr");
	m_pShader->activeShader();
	m_pShader->setTextureUniformValue("TerrainHeightmapTex", mTerrainHeightmapTex);



	auto 
}

void TerrainRenderPass::updateV()
{

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());

	m_pShader->activeShader();
	m_pShader->setFloatUniformValue("gTerrainResolution", TerrainResolution);
	glm::mat4 projection = ElayGraphics::Camera::getMainCameraProjectionMatrix();
	glm::mat4 view = ElayGraphics::Camera::getMainCameraViewMatrix();
	m_pShader->setMat4UniformValue("gViewProjMat", glm::value_ptr(projection*view));
	//glm::mat4 LightProjectionMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightProjectionMatrix");
	//glm::mat4 LightViewMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightViewMatrix");
	//m_pShader->setMat4UniformValue("u_LightVPMatrix", glm::value_ptr(LightProjectionMatrix * LightViewMatrix));
	
	glDrawArraysInstanced(GL_TRIANGLES, 0, 6, TerrainResolution * TerrainResolution);
	
	//glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);



}
