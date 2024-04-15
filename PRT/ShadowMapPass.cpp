#include "ShadowMapPass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include "Sponza.h"
#include "AABB.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

CShadowMapPass::CShadowMapPass(const std::string& vPassName, int vExcutionOrder) : IRenderPass(vPassName, vExcutionOrder)
{
}

CShadowMapPass::~CShadowMapPass()
{
}

void CShadowMapPass::initV()
{
	m_pShader = std::make_shared<CShader>("RSMBuffer_VS.glsl", "RSMBuffer_FS.glsl");
	m_pSponza = std::dynamic_pointer_cast<CSponza>(ElayGraphics::ResourceManager::getGameObjectByName("Sponza"));


	auto TextureConfig4Depth = std::make_shared<ElayGraphics::STexture>(); ;
	TextureConfig4Depth->InternalFormat = GL_DEPTH_COMPONENT32F;
	TextureConfig4Depth->ExternalFormat = GL_DEPTH_COMPONENT;
	TextureConfig4Depth->DataType = GL_FLOAT;
	TextureConfig4Depth->Width = TextureConfig4Depth->Height = m_RSMResolution;
	TextureConfig4Depth->BorderColor = { 0,0,0,0 };
	TextureConfig4Depth->TextureAttachmentType = ElayGraphics::STexture::ETextureAttachmentType::DepthTexture;

	genTexture(TextureConfig4Depth);
	m_FBO = genFBO({ TextureConfig4Depth });

	;
	ElayGraphics::ResourceManager::registerSharedData("LightDepthTexture", TextureConfig4Depth);

	m_pShader->activeShader();
	m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(m_pSponza->getModelMatrix()));
	m_pSponza->initModel(*m_pShader);

}

void CShadowMapPass::updateV()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glViewport(0, 0, m_RSMResolution, m_RSMResolution);

	m_pShader->activeShader();
	m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(m_pSponza->getModelMatrix()));
	glm::mat4 LightProjectionMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightProjectionMatrix");
	glm::mat4 LightViewMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightViewMatrix");
	
	m_pShader->setMat4UniformValue("u_LightVPMatrix", glm::value_ptr(LightProjectionMatrix * LightViewMatrix));
	m_pSponza->updateModel(*m_pShader);

	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glFlush();
}