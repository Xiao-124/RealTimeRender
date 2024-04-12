#include "InjectLightPass.h"
#include "Interface.h"
#include "Shader.h"
#include "Utils.h"
#include <GLFW/glfw3.h>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
CInjectLightPass::CInjectLightPass(const std::string& vPassName, int vExecutionOrder) : IRenderPass(vPassName, vExecutionOrder)
{
}

CInjectLightPass::~CInjectLightPass()
{
}


void CInjectLightPass::drawVPLVAO()
{

	if (m_VPLVAO == -1)
	{
		std::vector<glm::vec2> position;
		for (int i = 0; i < RSMResolution; i++)
		{
			for (int j = 0; j < RSMResolution; j++)
			{
				glm::vec2 p;
				p.x = (float)i / RSMResolution;
				p.y = (float)j / RSMResolution;
				p = p* 2.0f - glm::vec2(1);
				position.push_back(p);
			}
		}		
		m_VPLVAO = createVAO(position.data(), RSMResolution * RSMResolution *sizeof(glm::vec2), {2});
	}

	//if (m_VPLVAO == -1)
	//{
	//	float* VPLVertices = new float[RSMResolution * RSMResolution];
	//	m_VPLVAO = createVAO(VPLVertices, RSMResolution * RSMResolution, { 2 });
	//}
	glBindVertexArray(m_VPLVAO);
	glDrawArrays(GL_POINTS, 0, RSMResolution * RSMResolution);
	glBindVertexArray(0);
}
//************************************************************************************
//Function:
void CInjectLightPass::initV()
{
	RSMResolution = ElayGraphics::ResourceManager::getSharedDataByName<int>("RSMResolution");
	glm::vec3 MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	glm::vec3 MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");

	m_Dimensions = MaxAABB - MinAABB;
	TextureConfig4LPVGridR = std::make_shared<ElayGraphics::STexture>();
	TextureConfig4LPVGridG = std::make_shared<ElayGraphics::STexture>();
	TextureConfig4LPVGridB = std::make_shared<ElayGraphics::STexture>();
	TextureConfig4LPVGridR->ImageBindUnit = 0;
	TextureConfig4LPVGridG->ImageBindUnit = 1;
	TextureConfig4LPVGridB->ImageBindUnit = 2;
	TextureConfig4LPVGridR->TextureType = TextureConfig4LPVGridG->TextureType = TextureConfig4LPVGridB->TextureType = ElayGraphics::STexture::ETextureType::Texture3D;
	TextureConfig4LPVGridR->InternalFormat = TextureConfig4LPVGridG->InternalFormat = TextureConfig4LPVGridB->InternalFormat = GL_RGBA16F;
	TextureConfig4LPVGridR->ExternalFormat = TextureConfig4LPVGridG->ExternalFormat = TextureConfig4LPVGridB->ExternalFormat = GL_RGBA;
	TextureConfig4LPVGridR->DataType = TextureConfig4LPVGridG->DataType = TextureConfig4LPVGridB->DataType  = GL_FLOAT;
	TextureConfig4LPVGridR->Width = TextureConfig4LPVGridG->Width = TextureConfig4LPVGridB->Width = m_Dimensions.x;
	TextureConfig4LPVGridR->Height = TextureConfig4LPVGridG->Height = TextureConfig4LPVGridB->Height = m_Dimensions.y;
	TextureConfig4LPVGridR->Depth = TextureConfig4LPVGridG->Depth = TextureConfig4LPVGridB->Depth = m_Dimensions.z;
	TextureConfig4LPVGridR->Type4WrapS = TextureConfig4LPVGridG->Type4WrapS = TextureConfig4LPVGridB->Type4WrapS = GL_CLAMP_TO_EDGE;
	TextureConfig4LPVGridR->Type4WrapT = TextureConfig4LPVGridG->Type4WrapT = TextureConfig4LPVGridB->Type4WrapT = GL_CLAMP_TO_EDGE;


	TextureConfig4LPVGV = std::make_shared<ElayGraphics::STexture>();
	TextureConfig4LPVGV->ImageBindUnit = 3;
	TextureConfig4LPVGV->TextureType = ElayGraphics::STexture::ETextureType::Texture3D;
	TextureConfig4LPVGV->InternalFormat = GL_RGBA16F;
	TextureConfig4LPVGV->ExternalFormat = GL_RGBA;
	TextureConfig4LPVGV->DataType = GL_FLOAT;
	TextureConfig4LPVGV->Width = m_Dimensions.x;
	TextureConfig4LPVGV->Height = m_Dimensions.y;
	TextureConfig4LPVGV->Depth = m_Dimensions.z;
	TextureConfig4LPVGV->Type4WrapS = GL_CLAMP_TO_EDGE;
	TextureConfig4LPVGV->Type4WrapT = GL_CLAMP_TO_EDGE;
	

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	genTexture(TextureConfig4LPVGridR);
	genTexture(TextureConfig4LPVGridG);
	genTexture(TextureConfig4LPVGridB);
	genTexture(TextureConfig4LPVGV);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	//m_pShader = std::make_shared<CShader>("InjectLight_CS.glsl");
	m_pShader = std::make_shared<CShader>("LightInject_VS.glsl", "LightInject_FS.glsl", "LightInject_GS.glsl");

	m_pShader->activeShader();
	m_pShader->setFloatUniformValue("u_CellSize", 1.0);
	m_pShader->setFloatUniformValue("u_MinAABB", MinAABB.x, MinAABB.y , MinAABB.z);
	m_pShader->setFloatUniformValue("u_Dimensions", m_Dimensions.x, m_Dimensions.y, m_Dimensions.z);

	m_pShader->setIntUniformValue("u_RSMResolution", RSMResolution);
	m_pShader->setTextureUniformValue("u_RSMRadiantFluxTexture", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("RSMRadiantFluxTexture"));
	m_pShader->setTextureUniformValue("u_RSMPositionTexture", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("RSMPositionTexture"));
	m_pShader->setTextureUniformValue("u_RSMNormalTexture", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("RSMNormalAndDoubleRoughnessTexture"));

	ElayGraphics::ResourceManager::registerSharedData("LPVGridRTexture", TextureConfig4LPVGridR);
	ElayGraphics::ResourceManager::registerSharedData("LPVGridGTexture", TextureConfig4LPVGridG);
	ElayGraphics::ResourceManager::registerSharedData("LPVGridBTexture", TextureConfig4LPVGridB);
	ElayGraphics::ResourceManager::registerSharedData("LPVGVTexture", TextureConfig4LPVGV);

	
}

//************************************************************************************
//Function:
void CInjectLightPass::updateV()
{
	
	int RSMResolution = ElayGraphics::ResourceManager::getSharedDataByName<int>("RSMResolution");
	glm::vec3 lightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	ClearTexture(TextureConfig4LPVGridR, GL_TEXTURE_3D);
	ClearTexture(TextureConfig4LPVGridG, GL_TEXTURE_3D);
	ClearTexture(TextureConfig4LPVGridB, GL_TEXTURE_3D);
	ClearTexture(TextureConfig4LPVGV, GL_TEXTURE_3D);

	//glDisable(GL_DEPTH_TEST);
	//glDepthFunc(GL_ALWAYS);
	//glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_CULL_FACE);
	glViewport(0, 0, m_Dimensions.x, m_Dimensions.y);

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);	
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureConfig4LPVGridR->TextureID, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, TextureConfig4LPVGridG->TextureID, 0);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, TextureConfig4LPVGridB->TextureID, 0);
	GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Error::FBO:: Framebuffer Is Not Complete." << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
	}
	//genFBO();
	glDrawBuffers(3, attachments);
	

	//glClearColor(0, 0, 0, 0);
	//glClear(GL_COLOR_BUFFER_BIT);

	m_pShader->activeShader();
	m_pShader->setFloatUniformValue("u_LightDir", lightDir.x, lightDir.y, lightDir.z);
	m_pShader->setFloatUniformValue("u_RSMArea", ElayGraphics::ResourceManager::getSharedDataByName<float>("LightCameraAreaInWorldSpace"));
	m_pShader->setMat4UniformValue("u_LightViewMat", glm::value_ptr(ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightViewMatrix")));

	//glDispatchCompute(RSMResolution / 32, RSMResolution / 32, 1);
	//glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	drawVPLVAO();
	glFlush();

	glDisable(GL_BLEND);
	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}