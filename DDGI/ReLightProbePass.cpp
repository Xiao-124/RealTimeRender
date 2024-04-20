
#include "ReLightProbePass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include "Sponza.h"
#include "LightProbePass.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

CReLightProbePass::CReLightProbePass(const std::string& vPassName, int vExcutionOrder) : IRenderPass(vPassName, vExcutionOrder)
{


}

CReLightProbePass::~CReLightProbePass()
{


}


void CReLightProbePass::initV()
{
	m_pShader = std::make_shared<CShader>("ReLight_VS.glsl", "ReLight_FS.glsl");
	auto TextureConfigLight = std::make_shared<ElayGraphics::STexture>();
	TextureConfigLight->InternalFormat = GL_RGBA32F;
	TextureConfigLight->ExternalFormat = GL_RGBA;
	TextureConfigLight->DataType = GL_FLOAT;
	TextureConfigLight->isMipmap = false;
	genTexture(TextureConfigLight);
	m_FBO = genFBO({ TextureConfigLight });



	auto LightDepthTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("LightDepthTexture");
	auto TextureSamplePosition = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("TextureSamplePosition");
	auto TextureSampleAlbedo = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("TextureSampleAlbedo");
	auto TextureSampleNormal = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("TextureSampleNormal");
	auto TextureSampleChebyshev = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("TextureSampleChebyshev");
	
	m_pShader->activeShader();
	m_pShader->setTextureUniformValue("u_LightDepthTexture", LightDepthTexture);
	m_pShader->setTextureUniformValue("u_PositionTexture", TextureSamplePosition);
	m_pShader->setTextureUniformValue("u_NormalTexture", TextureSampleNormal);
	m_pShader->setTextureUniformValue("u_AlbedoTexture", TextureSampleAlbedo);


}

void CReLightProbePass::updateV()
{

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());

	m_pShader->activeShader();
	glm::mat4 LightProjectionMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightProjectionMatrix");
	glm::mat4 LightViewMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightViewMatrix");
	glm::vec3 LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	LightDir = glm::normalize(-LightDir);

	m_pShader->setMat4UniformValue("u_LightVPMatrix", glm::value_ptr(LightProjectionMatrix * LightViewMatrix));
	m_pShader->setFloatUniformValue("u_LightDir", LightDir.x, LightDir.y, LightDir.z);
	drawQuad();

	
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

