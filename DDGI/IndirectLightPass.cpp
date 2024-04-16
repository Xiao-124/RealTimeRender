#include "IndirectLightPass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include "Sponza.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include "ReLightProbePass.h"
CIndirectLightPass::CIndirectLightPass(const std::string& vPassName, int vExcutionOrder) : IRenderPass(vPassName, vExcutionOrder)
{
}

CIndirectLightPass::~CIndirectLightPass()
{
}

void CIndirectLightPass::initV()
{

	m_pShader = std::make_shared<CShader>("IndirectLightPass_VS.glsl", "IndirectLightPass_FS.glsl");
	m_pSponza = std::dynamic_pointer_cast<CSponza>(ElayGraphics::ResourceManager::getGameObjectByName("Sponza"));
	auto TextureConfig4IndirectLight = std::make_shared<ElayGraphics::STexture>();
	TextureConfig4IndirectLight->InternalFormat = GL_RGBA32F;
	TextureConfig4IndirectLight->ExternalFormat = GL_RGBA;
	TextureConfig4IndirectLight->DataType = GL_FLOAT;
	TextureConfig4IndirectLight->isMipmap = false;
	genTexture(TextureConfig4IndirectLight);
	m_FBO = genFBO({ TextureConfig4IndirectLight });

	ElayGraphics::ResourceManager::registerSharedData("IndirectLightTexture", TextureConfig4IndirectLight);
	auto shDatas = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<SHData>>("shDatas");
	//all_shbuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, shDatas->all_coefficientSH9.size() * shDatas->all_coefficientSH9[0].size() * sizeof(int), nullptr, GL_STATIC_DRAW, 2);
	
	m_pShader->activeShader();
	m_pShader->setFloatUniformValue("u_Exposure", ElayGraphics::ResourceManager::getSharedDataByName<float>("Exposure"));
	m_pShader->setTextureUniformValue("u_AlbedoTexture", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("AlbedoTexture"));
	m_pShader->setTextureUniformValue("u_NormalTexture", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("NormalTexture"));
	m_pShader->setTextureUniformValue("u_PositionTexture", ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("PositionTexture"));

}

void CIndirectLightPass::updateV()
{
	glm::ivec3 m_MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	glm::ivec3 m_MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");
	float step_probe = ElayGraphics::ResourceManager::getSharedDataByName<float>("step_probe");
	float _GIIntensity = ElayGraphics::ResourceManager::getSharedDataByName<float>("GIIntensity");


	glm::vec3 _coefficientVoxelSize(0, 0, 0);
	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
		_coefficientVoxelSize.x++;
	for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
		_coefficientVoxelSize.y++;
	for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
		_coefficientVoxelSize.z++;

	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	m_pShader->activeShader();
	auto ViewMatrix = ElayGraphics::Camera::getMainCameraViewMatrix();
	m_pShader->setMat4UniformValue("u_InverseCameraViewMatrix", glm::value_ptr(glm::inverse(ViewMatrix)));

	auto LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	m_pShader->setFloatUniformValue("_GIIntensity", _GIIntensity);
	//大小
	m_pShader->setFloatUniformValue("_coefficientVoxelGridSize", step_probe);
	//minAABB
	m_pShader->setFloatUniformValue("_coefficientVoxelCorner", m_MinAABB[0], m_MinAABB[1], m_MinAABB[2], 1);
	//x,y,z方向的个数
	m_pShader->setFloatUniformValue("_coefficientVoxelSize", _coefficientVoxelSize.x, _coefficientVoxelSize.y, _coefficientVoxelSize.z, 1.0);
	
	auto shDatas = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<SHData>>("shDatas");
	drawQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}