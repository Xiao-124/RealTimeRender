#include "PropagationLightPass.h"
#include "Interface.h"
#include "Shader.h"
#include "Utils.h"
#include <GLFW/glfw3.h>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>


//¹âÏß´«²¥
CPropagationLightPass::CPropagationLightPass(const std::string& vPassName, int vExecutionOrder) : IRenderPass(vPassName, vExecutionOrder)
{
}

CPropagationLightPass::~CPropagationLightPass()
{
}



//************************************************************************************
//Function:
void CPropagationLightPass::initV()
{
	int RSMResolution = ElayGraphics::ResourceManager::getSharedDataByName<int>("RSMResolution");
	glm::vec3 MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	glm::vec3 MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");

	m_Dimensions = MaxAABB - MinAABB;

	m_TextureConfig4LPVAccumulatorR = std::make_shared<ElayGraphics::STexture>();
	m_TextureConfig4LPVAccumulatorG = std::make_shared<ElayGraphics::STexture>();
	m_TextureConfig4LPVAccumulatorB = std::make_shared<ElayGraphics::STexture>();
	m_TextureConfig4LPVAccumulatorR->ImageBindUnit = 0;
	m_TextureConfig4LPVAccumulatorG->ImageBindUnit = 1;
	m_TextureConfig4LPVAccumulatorB->ImageBindUnit = 2;
	m_TextureConfig4LPVAccumulatorR->TextureType = m_TextureConfig4LPVAccumulatorG->TextureType = m_TextureConfig4LPVAccumulatorB->TextureType = ElayGraphics::STexture::ETextureType::Texture3D;
	m_TextureConfig4LPVAccumulatorR->InternalFormat = m_TextureConfig4LPVAccumulatorG->InternalFormat = m_TextureConfig4LPVAccumulatorB->InternalFormat = GL_RGBA16F;
	m_TextureConfig4LPVAccumulatorR->ExternalFormat = m_TextureConfig4LPVAccumulatorG->ExternalFormat = m_TextureConfig4LPVAccumulatorB->ExternalFormat = GL_RGBA;
	m_TextureConfig4LPVAccumulatorR->DataType = m_TextureConfig4LPVAccumulatorG->DataType = m_TextureConfig4LPVAccumulatorB->DataType = GL_FLOAT;
	m_TextureConfig4LPVAccumulatorR->Width = m_TextureConfig4LPVAccumulatorG->Width = m_TextureConfig4LPVAccumulatorB->Width = m_Dimensions.x;
	m_TextureConfig4LPVAccumulatorR->Height = m_TextureConfig4LPVAccumulatorG->Height = m_TextureConfig4LPVAccumulatorB->Height = m_Dimensions.y;
	m_TextureConfig4LPVAccumulatorR->Depth = m_TextureConfig4LPVAccumulatorG->Depth = m_TextureConfig4LPVAccumulatorB->Depth = m_Dimensions.z;
	m_TextureConfig4LPVAccumulatorR->Type4WrapS = m_TextureConfig4LPVAccumulatorG->Type4WrapS = m_TextureConfig4LPVAccumulatorB->Type4WrapS = GL_CLAMP_TO_EDGE;
	m_TextureConfig4LPVAccumulatorR->Type4WrapT = m_TextureConfig4LPVAccumulatorG->Type4WrapT = m_TextureConfig4LPVAccumulatorB->Type4WrapT = GL_CLAMP_TO_EDGE;

	genTexture(m_TextureConfig4LPVAccumulatorR);
	genTexture(m_TextureConfig4LPVAccumulatorG);
	genTexture(m_TextureConfig4LPVAccumulatorB);


	m_pShader = std::make_shared<CShader>("PropagationLight_CS.glsl");
	m_pShader->activeShader();
	m_pShader->setFloatUniformValue("u_GridDim", m_Dimensions.x, m_Dimensions.y, m_Dimensions.z);
	m_pShader->setIntUniformValue("u_FirstPropStep", false);
	m_pShader->setTextureUniformValue("LPVGridR", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("LPVGridRTexture"));
	m_pShader->setTextureUniformValue("LPVGridG", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("LPVGridGTexture"));
	m_pShader->setTextureUniformValue("LPVGridB", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("LPVGridBTexture"));
	m_pShader->setTextureUniformValue("GeometryVolume", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("LPVGVTexture"));
	m_pShader->setImageUniformValue(m_TextureConfig4LPVAccumulatorR);
	m_pShader->setImageUniformValue(m_TextureConfig4LPVAccumulatorG);
	m_pShader->setImageUniformValue(m_TextureConfig4LPVAccumulatorB);

	ElayGraphics::ResourceManager::registerSharedData("LPVAccumulatorRTexture", m_TextureConfig4LPVAccumulatorR);
	ElayGraphics::ResourceManager::registerSharedData("LPVAccumulatorGTexture", m_TextureConfig4LPVAccumulatorG);
	ElayGraphics::ResourceManager::registerSharedData("LPVAccumulatorBTexture", m_TextureConfig4LPVAccumulatorB);
}

//************************************************************************************
//Function:
void CPropagationLightPass::updateV()
{
	
	ClearTexture(m_TextureConfig4LPVAccumulatorR, GL_TEXTURE_3D);
	ClearTexture(m_TextureConfig4LPVAccumulatorG, GL_TEXTURE_3D);
	ClearTexture(m_TextureConfig4LPVAccumulatorB, GL_TEXTURE_3D);
	m_pShader->activeShader();

	int DispatchX = m_Dimensions.x % 4 == 0 ? m_Dimensions.x / 4 : m_Dimensions.x / 4 + 1;
	int DispatchY = m_Dimensions.y % 4 == 0 ? m_Dimensions.y / 4 : m_Dimensions.y / 4 + 1;
	int DispatchZ = m_Dimensions.z % 4 == 0 ? m_Dimensions.z / 4 : m_Dimensions.z / 4 + 1;

	glDispatchCompute(DispatchX, DispatchY, DispatchZ);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	

}