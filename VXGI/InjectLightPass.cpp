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


//Function:
void CInjectLightPass::initV()
{
	m_pShader = std::make_shared<CShader>("InjectLight_CS.glsl");
	MipmapShader = std::make_shared<CShader>("MipMap_CS.glsl");

	int VoxelSize = ElayGraphics::ResourceManager::getSharedDataByName<int>("VoxelSize");
	Radiance3D = std::make_shared<ElayGraphics::STexture>();
	Radiance3D->TextureType = ElayGraphics::STexture::ETextureType::Texture3D;
	Radiance3D->InternalFormat = GL_RGBA8;
	Radiance3D->ExternalFormat = GL_RGBA;
	Radiance3D->DataType = GL_UNSIGNED_BYTE;

	Radiance3D->Width = VoxelSize;
	Radiance3D->Height = VoxelSize;
	Radiance3D->Depth = VoxelSize;

	Radiance3D->Type4MagFilter = GL_LINEAR;
	Radiance3D->Type4MinFilter = GL_LINEAR_MIPMAP_LINEAR;
	//Radiance3D->Type4WrapR = GL_CLAMP_TO_BORDER;
	//Radiance3D->Type4WrapS = GL_CLAMP_TO_BORDER;
	//Radiance3D->Type4WrapT = GL_CLAMP_TO_BORDER;
	Radiance3D->isMipmap = true;
	genTexture(Radiance3D);
	ElayGraphics::ResourceManager::registerSharedData("VoxelRadiance3D", Radiance3D);
}

//************************************************************************************
//Function:
void CInjectLightPass::updateV()
{

	float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glClearTexImage(Radiance3D->TextureID, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);
	
	
	auto LightDepthTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("LightDepthTexture");
	//glm::mat4 DirectionalLightSpaceMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightViewMatrix");
	
	glm::mat4 LightProjectionMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightProjectionMatrix");
	glm::mat4 LightViewMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightViewMatrix");
	glm::mat4 DirectionalLightSpaceMatrix = LightProjectionMatrix * LightViewMatrix;


	float far_plane = ElayGraphics::Camera::getMainCameraFar();
	float MaxCoord = ElayGraphics::ResourceManager::getSharedDataByName<float>("MaxCoord");
	int VoxelSize = ElayGraphics::ResourceManager::getSharedDataByName<int>("VoxelSize");
	int workgroupsize = VoxelSize / 8;
	auto voxelTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("VoxelTexture");
	auto voxelNormalTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("VoxelTexture");
	auto LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	
	m_pShader->activeShader();
	m_pShader->setMat4UniformValue("lightSpaceMatrix", glm::value_ptr(DirectionalLightSpaceMatrix));
	m_pShader->setFloatUniformValue("DirectionalLightStrength", ElayGraphics::ResourceManager::getSharedDataByName<float>("LightIntensity"));
	
	//m_pShader->setFloatUniformValue("far_plane", far_plane);
	m_pShader->setFloatUniformValue("MaxCoord", MaxCoord);
	m_pShader->setIntUniformValue("VoxelSize", VoxelSize);

	LightDir = glm::normalize(-LightDir);
	m_pShader->setFloatUniformValue("lightDir", LightDir.x, LightDir.y, LightDir.z);
	
	glBindImageTexture(0, Radiance3D->TextureID, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, voxelTexture->TextureID);
	m_pShader->setIntUniformValue("Albedo3D", 0);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_3D, voxelNormalTexture->TextureID);
	m_pShader->setIntUniformValue("Normal3D", 1);
	
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, LightDepthTexture->TextureID);
	m_pShader->setIntUniformValue("DirectionalDepthMap", 2);
	
	glDispatchCompute(workgroupsize, workgroupsize, workgroupsize);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


	int level = 6;
	MipmapShader->activeShader();
	for (int mipLevel = 1; mipLevel < level; mipLevel++) 
	{
		glBindImageTexture(0, Radiance3D->TextureID, mipLevel, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
		glBindImageTexture(1, Radiance3D->TextureID, mipLevel - 1, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
		int div = pow(2, mipLevel);
		glDispatchCompute(workgroupsize / div, workgroupsize / div, workgroupsize / div);
		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


}