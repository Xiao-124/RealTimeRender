#include "VoxelizationPass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include "Sponza.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/glm.hpp>
#include <GLM/gtx/component_wise.hpp> 

//²»¿¼ÂÇ¶¯Ì¬³¡¾°
CVoxelizationPass::CVoxelizationPass(const std::string& vPassName, int vExecutionOrder, ElayGraphics::ERenderPassType vtype) : IRenderPass(vPassName, vExecutionOrder, vtype)
{



}

CVoxelizationPass::~CVoxelizationPass()
{



}

void CVoxelizationPass::initV()
{
	m_pShader = std::make_shared<CShader>("VoxelizationPass_VS.glsl", "VoxelizationPass_FS.glsl", "VoxelizationPass_GS.glsl");
	m_pSponza = std::dynamic_pointer_cast<CSponza>(ElayGraphics::ResourceManager::getGameObjectByName("Sponza"));
	
	voxelTexture = std::make_shared<ElayGraphics::STexture>();
	voxelTexture->TextureType = ElayGraphics::STexture::ETextureType::Texture3D;
	voxelTexture->InternalFormat = GL_RGBA8;
	voxelTexture->ExternalFormat = GL_RGBA;
	voxelTexture->DataType = GL_UNSIGNED_BYTE;

	//voxelTexture->InternalFormat = GL_R32UI;
	//voxelTexture->ExternalFormat = GL_RED;
	//voxelTexture->DataType = GL_UNSIGNED_INT;
	
	voxelTexture->Width = VoxelResolution;
	voxelTexture->Height = VoxelResolution;
	voxelTexture->Depth = VoxelResolution;
	//voxelTexture->isMipmap = true;
	genTexture(voxelTexture);

	voxelNormalTexture = std::make_shared<ElayGraphics::STexture>();
	voxelNormalTexture->TextureType = ElayGraphics::STexture::ETextureType::Texture3D;
	voxelNormalTexture->InternalFormat = GL_RGBA8;
	voxelNormalTexture->ExternalFormat = GL_RGBA;
	voxelNormalTexture->DataType = GL_UNSIGNED_BYTE;

	voxelNormalTexture->Width = VoxelResolution;
	voxelNormalTexture->Height = VoxelResolution;
	voxelNormalTexture->Depth = VoxelResolution;
	//voxelTexture->isMipmap = true;
	genTexture(voxelNormalTexture);

	glm::vec3 minAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	glm::vec3 maxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");

	
	float MaxCoord = 0;
	glm::vec3 midpoint = (minAABB + maxAABB) * 0.5f;
	MaxCoord = glm::compMax(glm::abs(midpoint - maxAABB)) + 1.0;
	glm::mat4 SVOGIproj = glm::ortho(-MaxCoord, MaxCoord, -MaxCoord, MaxCoord, 0.1f, 2.0f * MaxCoord + 0.1f);
	projectMat = SVOGIproj * glm::lookAt(glm::vec3(0.0f, 0.0f, MaxCoord + 0.1f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	
	ElayGraphics::ResourceManager::registerSharedData("MaxCoord", MaxCoord);
	ElayGraphics::ResourceManager::registerSharedData("VoxelSize", VoxelResolution);
	ElayGraphics::ResourceManager::registerSharedData("VoxelProjectMat", projectMat);

	ElayGraphics::ResourceManager::registerSharedData("VoxelTexture", voxelTexture);
	ElayGraphics::ResourceManager::registerSharedData("VoxelNormalTexture", voxelNormalTexture);

	m_pShader->activeShader();
	m_pShader->setIntUniformValue("VoxelSize", int(VoxelResolution));
	m_pSponza->initModel(*m_pShader);
}

void CVoxelizationPass::updateV()
{
	
	if (isinit == 0) 
	{
		float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glClearTexImage(voxelTexture->TextureID, 0, GL_RGBA, GL_UNSIGNED_BYTE, zero);

		//unsigned int zero[] = { 0.0f};
		//glClearTexImage(voxelTexture->TextureID, 0, GL_R32UI, GL_UNSIGNED_INT, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
		glViewport(0, 0, VoxelResolution, VoxelResolution);
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
	
		glm::mat4 model(1);
		m_pShader->activeShader();
		m_pShader->setMat4UniformValue("ProjectMat", glm::value_ptr(projectMat));
		m_pShader->setIntUniformValue("VoxelSize", int(VoxelResolution));
		m_pShader->setMat4UniformValue("model", glm::value_ptr(model));
	
		glBindImageTexture(0, voxelTexture->TextureID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
		glBindImageTexture(1, voxelNormalTexture->TextureID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
		//glBindImageTexture(0, voxelTexture->TextureID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
		//glBindImageTexture(0, voxelTexture->TextureID, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
		m_pSponza->updateModel(*m_pShader);
		glFlush();
	
		//glBindTexture(GL_TEXTURE_3D, voxelTexture->TextureID);
		//glGenerateMipmap(GL_TEXTURE_3D);
		//glBindTexture(GL_TEXTURE_3D, 0);
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);	
		glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
		isinit = 1;
		
	}


}
