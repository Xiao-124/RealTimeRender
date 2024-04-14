#include "VoxelVisualizePass.h"
#include "Interface.h"
#include "Shader.h"
#include "Utils.h"
#include <GLFW/glfw3.h>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
CVoxelVisualizePass::CVoxelVisualizePass(const std::string& vPassName, int vExecutionOrder) : IRenderPass(vPassName, vExecutionOrder)
{
}

CVoxelVisualizePass::~CVoxelVisualizePass()
{
}


void CVoxelVisualizePass::initV()
{
	visFaceShader = std::make_shared<CShader>("VoxelVisFace_VS.glsl", "VoxelVisFace_FS.glsl");
	m_pShader = std::make_shared<CShader>("VoxelVisTrace_VS.glsl", "VoxelVisTrace_FS.glsl");

	VoxelVisFrontFace = std::make_shared<ElayGraphics::STexture>();
	VoxelVisFrontFace->InternalFormat = GL_RGB16F;
	VoxelVisFrontFace->ExternalFormat = GL_RGB;
	VoxelVisFrontFace->DataType = GL_FLOAT;

	VoxelVisBackFace = std::make_shared<ElayGraphics::STexture>();
	VoxelVisBackFace->InternalFormat = GL_RGB16F;
	VoxelVisBackFace->ExternalFormat = GL_RGB;
	VoxelVisBackFace->DataType = GL_FLOAT;

	//VoxelVisOut = std::make_shared<ElayGraphics::STexture>();
	//VoxelVisOut->InternalFormat = GL_RGBA8;
	//VoxelVisOut->ExternalFormat = GL_RGBA;
	//VoxelVisOut->DataType = GL_UNSIGNED_BYTE;

	genTexture(VoxelVisFrontFace);
	genTexture(VoxelVisBackFace);
	//genTexture(VoxelVisOut);
	mFBO = genFBO({ VoxelVisFrontFace , VoxelVisBackFace });


	ElayGraphics::ResourceManager::registerSharedData("VoxelVisOut", VoxelVisOut);
}

void CVoxelVisualizePass::updateV()
{


	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
	
	visFaceShader->activeShader();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glCullFace(GL_BACK);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	drawCube();

	glDrawBuffer(GL_COLOR_ATTACHMENT1);
	glCullFace(GL_FRONT);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	drawCube();
	glCullFace(GL_BACK);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	//m_pShader->activeShader();
	//glDrawBuffer(GL_COLOR_ATTACHMENT2);
	//glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	//
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, VoxelVisBackFace->TextureID);
	//m_pShader->setIntUniformValue("textureBack", 0);
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, VoxelVisFrontFace->TextureID);
	//m_pShader->setIntUniformValue("textureFront", 1);
	//glActiveTexture(GL_TEXTURE2);
	//auto voxelTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>> ("VoxelTexture");
	//glBindTexture(GL_TEXTURE_3D, voxelTexture->TextureID);
	//m_pShader->setIntUniformValue("textureColor", 2);
	//
	glm::vec3 temp_campos = ElayGraphics::Camera::getMainCameraPos();
	//m_pShader->setFloatUniformValue("cameraPosition", temp_campos.x, temp_campos.y, temp_campos.z);
	//drawQuad();

	//glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	m_pShader->activeShader();
	m_pShader->setFloatUniformValue("cameraPosition", temp_campos.x, temp_campos.y, temp_campos.z);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, VoxelVisBackFace->TextureID);
	m_pShader->setIntUniformValue("textureBack", 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, VoxelVisFrontFace->TextureID);
	m_pShader->setIntUniformValue("textureFront", 1);
	glActiveTexture(GL_TEXTURE2);
	auto voxelTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("VoxelTexture");
	auto VoxelRadiance3D = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("VoxelRadiance3D");
	
	//glBindTexture(GL_TEXTURE_3D, voxelTexture->TextureID);
	glBindTexture(GL_TEXTURE_3D, VoxelRadiance3D->TextureID);

	m_pShader->setIntUniformValue("textureColor", 2);
	drawQuad();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
