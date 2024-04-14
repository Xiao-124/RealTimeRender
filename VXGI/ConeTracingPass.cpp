#include "ConeTracingPass.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include "Shader.h"
#include <GLM/gtc/type_ptr.hpp>
CConeTracingPass::CConeTracingPass(const std::string& vPassName, int vExcutionOrder) : IRenderPass(vPassName, vExcutionOrder)
{
}

CConeTracingPass::~CConeTracingPass()
{
}

void CConeTracingPass::initV()
{
	m_pShader = std::make_shared<CShader>("ConeTracingPass_VS.glsl", "ConeTracingPass_FS.glsl");


}

void CConeTracingPass::updateV()
{
	
	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glm::vec3 temp_campos = ElayGraphics::Camera::getMainCameraPos();
	auto LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	glm::vec3 temp_dir = -glm::normalize(LightDir);
	auto AlbedoTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("AlbedoTexture");
	auto NormalTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("NormalTexture");
	auto PositionTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("PositionTexture");
	auto LightDepthTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("LightDepthTexture");
	auto VoxelRadiance3D = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("VoxelRadiance3D");
	auto directType = ElayGraphics::ResourceManager::getSharedDataByName<int>("DirectLightType");


	m_pShader->activeShader();
	m_pShader->setFloatUniformValue("DirectionalLightDirection", temp_dir.x, temp_dir.y, temp_dir.z);
	m_pShader->setFloatUniformValue("DirectionalLightStrength", ElayGraphics::ResourceManager::getSharedDataByName<float>("LightIntensity"));
	m_pShader->setIntUniformValue("directType", directType);



	m_pShader->setFloatUniformValue("viewPos", temp_campos.x, temp_campos.y, temp_campos.z);
	m_pShader->setFloatUniformValue("u_Exposure", ElayGraphics::ResourceManager::getSharedDataByName<float>("Exposure"));

	
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, PositionTexture->TextureID);
		m_pShader->setIntUniformValue("gPosition", 0);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, NormalTexture->TextureID);
		m_pShader->setIntUniformValue("gNormal", 1);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, AlbedoTexture->TextureID);
		m_pShader->setIntUniformValue("gAlbedoSpec", 2);
	}

	float MaxCoord = ElayGraphics::ResourceManager::getSharedDataByName<float>("MaxCoord");
	int VoxelSize = ElayGraphics::ResourceManager::getSharedDataByName<int>("VoxelSize");
	glm::mat4 VoxelProjectMat = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("VoxelProjectMat");

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, LightDepthTexture->TextureID);
	m_pShader->setIntUniformValue("u_LightDepthTexture", 3);
	glm::mat4 LightProjectionMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightProjectionMatrix");
	glm::mat4 LightViewMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightViewMatrix");
	m_pShader->setMat4UniformValue("u_LightVPMatrix", glm::value_ptr(LightProjectionMatrix * LightViewMatrix));

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_3D, VoxelRadiance3D->TextureID);
	m_pShader->setIntUniformValue("Radiance3D", 4);

	m_pShader->setFloatUniformValue("MaxCoord", MaxCoord);
	m_pShader->setIntUniformValue("VoxelSize", VoxelSize);
	m_pShader->setMat4UniformValue("ProjectMat", glm::value_ptr(VoxelProjectMat));


	float GI_SpecularOffsetFactor = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_SpecularOffsetFactor");
	float GI_SpecularAperture = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_SpecularAperture");
	float GI_SpecularMaxT = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_SpecularMaxT");
	float GI_DiffuseOffsetFactor = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_DiffuseOffsetFactor");
	float GI_DiffuseAperture =  ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_DiffuseAperture");
	float GI_DiffuseConeAngleMix = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_DiffuseConeAngleMix");
	float GI_DiffuseMaxT = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_DiffuseMaxT");
	float GI_OcclusionOffsetFactor = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_OcclusionOffsetFactor");
	float GI_OcculsionAperture = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_OcculsionAperture");
	float GI_DirectionalMaxT = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_DirectionalMaxT");
	float GI_stepSize = ElayGraphics::ResourceManager::getSharedDataByName<float>("GI_stepSize");
	bool useIndirect = ElayGraphics::ResourceManager::getSharedDataByName<bool>("useIndirect");
	float specularFactor = ElayGraphics::ResourceManager::getSharedDataByName<float>("specularFactor");

	m_pShader->setIntUniformValue("useIndirect", useIndirect);
	m_pShader->setFloatUniformValue("specularFactor", specularFactor);

	m_pShader->setFloatUniformValue("SpecularOffsetFactor", GI_SpecularOffsetFactor);
	m_pShader->setFloatUniformValue("SpecularAperture", GI_SpecularAperture);
	m_pShader->setFloatUniformValue("SpecularMaxT", GI_SpecularMaxT);

	m_pShader->setFloatUniformValue("DiffuseOffsetFactor", GI_DiffuseOffsetFactor);
	m_pShader->setFloatUniformValue("DiffuseAperture", GI_DiffuseAperture);
	m_pShader->setFloatUniformValue("DiffuseConeAngleMix", GI_DiffuseConeAngleMix);
	m_pShader->setFloatUniformValue("DiffuseMaxT", GI_DiffuseMaxT);

	m_pShader->setFloatUniformValue("OcclusionOffsetFactor", GI_OcclusionOffsetFactor);
	m_pShader->setFloatUniformValue("OcculsionAperture", GI_OcculsionAperture);
	m_pShader->setFloatUniformValue("DirectionalMaxT", GI_DirectionalMaxT);

	m_pShader->setFloatUniformValue("stepSize", GI_stepSize);
	float VoxelCellSize = MaxCoord * 2.0 / VoxelSize;
	m_pShader->setFloatUniformValue("VoxelCellSize", VoxelCellSize);
	drawQuad();

}
