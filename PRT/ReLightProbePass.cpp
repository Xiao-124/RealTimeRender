
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
	m_pShader = std::make_shared<CShader>("SurfelLight_CS.glsl");
	int _coefficientSH9[27] = {0};
	shBuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, 27 * sizeof(int), &_coefficientSH9[0], GL_STATIC_DRAW, 1);
	auto suefelDatas = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<SurfelData>>("SurfelDatas");
	
	shDatas = std::make_shared<SHData>();
	shDatas->all_coefficientSH9.resize(suefelDatas->all_surfelData.size());
	for (int i = 0; i < suefelDatas->all_surfelData.size(); i++)
	{
		shDatas->all_coefficientSH9[i].resize(27);
	}
	ElayGraphics::ResourceManager::registerSharedData("shDatas", shDatas);
	auto LightDepthTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("LightDepthTexture");
	m_pShader->activeShader();
	m_pShader->setTextureUniformValue("u_LightDepthTexture", LightDepthTexture);

	

}

void CReLightProbePass::updateV()
{

	glm::ivec3 m_MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	glm::ivec3 m_MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");
	float step_probe = ElayGraphics::ResourceManager::getSharedDataByName<float>("step_probe");
	auto suefelDatas = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<SurfelData>>("SurfelDatas");
	GLuint surfelBuffer = ElayGraphics::ResourceManager::getSharedDataByName<GLuint>("SurfelBuffer");
	glm::mat4 LightProjectionMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightProjectionMatrix");
	glm::mat4 LightViewMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightViewMatrix");

	
	int Index = 0;
	int _coefficientSH9[27] = { 0 };

	auto LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	{
		for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
		{
			for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
			{
				
				updateSSBOBuffer(surfelBuffer, 32*16* sizeof(Surfel), &suefelDatas->all_surfelData[Index][0], GL_STATIC_DRAW);
				updateSSBOBuffer(shBuffer, 27 * sizeof(int), &_coefficientSH9[0], GL_STATIC_DRAW);
				m_pShader->activeShader();
				glm::vec3 _probePos = glm::vec3((i + step_probe) * 0.5, (j + step_probe) * 0.5, (k + step_probe) * 0.5);
				m_pShader->setFloatUniformValue("_probePos", _probePos[0], _probePos[1], _probePos[2], 1);
				m_pShader->setFloatUniformValue("lightDirection", -LightDir[0],- LightDir[1], -LightDir[2]);
				m_pShader->setMat4UniformValue("u_LightVPMatrix", glm::value_ptr(LightProjectionMatrix * LightViewMatrix));
				glDispatchCompute(1, 1, 1);
				glFlush();

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, shBuffer);
				GLvoid* p = (GLvoid*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
				memcpy(&shDatas->all_coefficientSH9[Index][0], p, 27 * sizeof(int));
				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				Index++;
			}
		}
	}

}