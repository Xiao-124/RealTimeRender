#include "GBufferPass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include "Sponza.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>



// GL_KHR_debug

static void GLAPIENTRY debug_message(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	switch (severity) 
	{
	case GL_DEBUG_SEVERITY_HIGH:
		std::cerr << "Error: " << message << std::endl;
		return;
	case GL_DEBUG_SEVERITY_MEDIUM:
		std::cerr << "Warning: " << message << std::endl;
		return;
	case GL_DEBUG_SEVERITY_LOW:
		std::cout << "Info: " << message << std::endl;
		return;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		std::cout << "Notification: " << message << std::endl;;
		return;
	}
	//ASSERT(false, "Unknown severity level!");
}

void initgldebug()
{
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(debug_message, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
}


GBufferPass::GBufferPass(const std::string& vPassName, int vExcutionOrder) : IRenderPass(vPassName, vExcutionOrder)
{
}

GBufferPass::~GBufferPass()
{
}

void GBufferPass::initV()
{
	initgldebug();

	m_pShader = std::make_shared<CShader>("GBuffer_VS.glsl", "GBuffer_FS.glsl");
	m_pSponza = std::dynamic_pointer_cast<CSponza>(ElayGraphics::ResourceManager::getGameObjectByName("Sponza"));
	auto TextureConfig4Position = std::make_shared<ElayGraphics::STexture>();
	auto TextureConfig4NormalAndDoubleRoughness = std::make_shared<ElayGraphics::STexture>();
	auto TextureConfig4AlbedoAndMetallic = std::make_shared<ElayGraphics::STexture>();
	TextureConfig4AlbedoAndMetallic->InternalFormat = TextureConfig4NormalAndDoubleRoughness->InternalFormat = TextureConfig4Position->InternalFormat = GL_RGBA32F;
	TextureConfig4AlbedoAndMetallic->ExternalFormat = TextureConfig4NormalAndDoubleRoughness->ExternalFormat = TextureConfig4Position->ExternalFormat = GL_RGBA;
	TextureConfig4AlbedoAndMetallic->DataType = TextureConfig4NormalAndDoubleRoughness->DataType = TextureConfig4Position->DataType = GL_FLOAT;
	TextureConfig4AlbedoAndMetallic->isMipmap = TextureConfig4NormalAndDoubleRoughness->isMipmap = TextureConfig4Position->isMipmap = false;
	genTexture(TextureConfig4AlbedoAndMetallic);
	genTexture(TextureConfig4NormalAndDoubleRoughness);
	genTexture(TextureConfig4Position);
	m_FBO = genFBO({ TextureConfig4AlbedoAndMetallic, TextureConfig4NormalAndDoubleRoughness, TextureConfig4Position});

	ElayGraphics::ResourceManager::registerSharedData("AlbedoTexture", TextureConfig4AlbedoAndMetallic);
	ElayGraphics::ResourceManager::registerSharedData("NormalTexture", TextureConfig4NormalAndDoubleRoughness);
	ElayGraphics::ResourceManager::registerSharedData("PositionTexture", TextureConfig4Position);

	m_pShader->activeShader();
	m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(m_pSponza->getModelMatrix()));
	m_pShader->setFloatUniformValue("u_Near", ElayGraphics::Camera::getMainCameraNear());
	m_pShader->setFloatUniformValue("u_Far", ElayGraphics::Camera::getMainCameraFar());
	m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(m_pSponza->getModelMatrix()));
	m_pShader->setMat4UniformValue("u_TransposeInverseViewModelMatrix", glm::value_ptr(glm::transpose(glm::inverse(ElayGraphics::Camera::getMainCameraViewMatrix() *  m_pSponza->getModelMatrix()))));
	m_pSponza->initModel(*m_pShader);
}

void GBufferPass::updateV()
{

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	m_pShader->activeShader();
	m_pShader->setFloatUniformValue("u_Near", ElayGraphics::Camera::getMainCameraNear());
	m_pShader->setFloatUniformValue("u_Far", ElayGraphics::Camera::getMainCameraFar());
	m_pShader->setMat4UniformValue("u_TransposeInverseViewModelMatrix", glm::value_ptr(glm::transpose(glm::inverse(ElayGraphics::Camera::getMainCameraViewMatrix() *  m_pSponza->getModelMatrix()))));
	m_pSponza->updateModel(*m_pShader);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}