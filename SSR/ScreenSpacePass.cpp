#include "ScreenSpacePass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include "Sponza.h"
#include "AABB.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
CScreenSpacePass::CScreenSpacePass(const std::string& vPassName, int vExcutionOrder):IRenderPass(vPassName, vExcutionOrder)
{
}

CScreenSpacePass::~CScreenSpacePass()
{
}

void CScreenSpacePass::initV()
{
	m_pShader = std::make_shared<CShader>("ScreenSpace_VS.glsl", "ScreenSpace_FS.glsl");


	std::shared_ptr<ElayGraphics::STexture> TextureConfigDepth = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("TextureConfig4Depth");
	std::shared_ptr<ElayGraphics::STexture> AlbedoTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("AlbedoTexture");
	std::shared_ptr<ElayGraphics::STexture> PositionTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("PositionTexture");
	std::shared_ptr<ElayGraphics::STexture> NormalTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("NormalTexture");

	m_pShader->activeShader();
	m_pShader->setTextureUniformValue("PositionTexture", PositionTexture);
	m_pShader->setTextureUniformValue("NormalTexture", NormalTexture);
	m_pShader->setTextureUniformValue("ColorTexture", AlbedoTexture);
	m_pShader->setTextureUniformValue("DownscaledDepth", TextureConfigDepth);



}

void CScreenSpacePass::updateV()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	int maxLevel = ElayGraphics::ResourceManager::getSharedDataByName<int>("maxLevel");
	m_pShader->activeShader();
	m_pShader->setIntUniformValue("maxLevel", maxLevel);
	drawQuad();

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}
