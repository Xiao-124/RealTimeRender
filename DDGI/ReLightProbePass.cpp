
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
	//m_pShader = std::make_shared<CShader>("SurfelLight_CS.glsl");
	//
	//
	//std::vector<Surfel> pushsurfelDatas;
	//auto suefelDatas = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<SurfelData>>("SurfelDatas");
	//for (int i = 0; i < suefelDatas->all_surfelData.size(); i++)
	//{
	//	for (int j = 0; j < suefelDatas->all_surfelData[i].size(); j++)
	//	{
	//		pushsurfelDatas.push_back(suefelDatas->all_surfelData[i][j]);
	//	}
	//}
	//surfelBuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, pushsurfelDatas.size() * sizeof(Surfel), &pushsurfelDatas[0], GL_STATIC_DRAW, 0);
	//ElayGraphics::ResourceManager::registerSharedData("surfelBuffer", surfelBuffer);
	//
	//
	//
	//shDatas = std::make_shared<SHData>();
	//shDatas->all_coefficientSH9.resize(suefelDatas->all_surfelData.size());
	//int all_num = suefelDatas->all_surfelData.size();
	//for (int i = 0; i < suefelDatas->all_surfelData.size(); i++)
	//{
	//	shDatas->all_coefficientSH9[i].resize(27);
	//}
	//
	////shDatas = std::make_shared<SHData>();
	////shDatas->all_coefficientSH9.resize(suefelDatas->all_surfelData.size());
	//
	//std::vector<float> cleanDataFloat(27 * all_num, 0);
	//shBufferFloat = genBuffer(GL_SHADER_STORAGE_BUFFER, 27 * sizeof(float) * all_num, &cleanDataFloat[0], GL_STATIC_DRAW, 1);
	//ElayGraphics::ResourceManager::registerSharedData("AllshBufferFloat", shBufferFloat);
	//lastshBufferFloat = genBuffer(GL_SHADER_STORAGE_BUFFER, 27 * sizeof(float) * all_num, &cleanDataFloat[0], GL_STATIC_DRAW, 3);
	//
	//
	//std::vector<float> cleanDataRadiance(3 * pushsurfelDatas.size(), 0);
	//SurfelRadianceBuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, sizeof(glm::vec3) * pushsurfelDatas.size(), &cleanDataRadiance[0], GL_STATIC_DRAW, 2);
	//ElayGraphics::ResourceManager::registerSharedData("SurfelRadianceBuffer", SurfelRadianceBuffer);
	//
	//
	//ElayGraphics::ResourceManager::registerSharedData("shDatas", shDatas);
	//auto LightDepthTexture = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("LightDepthTexture");
	//m_pShader->activeShader();
	//m_pShader->setTextureUniformValue("u_LightDepthTexture", LightDepthTexture);

	

}

void CReLightProbePass::updateV()
{

	//std::vector<Surfel> pushsurfelDatas;
	//auto suefelDatas = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<SurfelData>>("SurfelDatas");
	//for (int i = 0; i < suefelDatas->all_surfelData.size(); i++)
	//{
	//	for (int j = 0; j < suefelDatas->all_surfelData[i].size(); j++)
	//	{
	//		pushsurfelDatas.push_back(suefelDatas->all_surfelData[i][j]);
	//	}
	//}
	////x, y, z
	//updateSSBOBuffer(surfelBuffer, pushsurfelDatas.size() * sizeof(Surfel), &pushsurfelDatas[0], GL_STATIC_DRAW);
	//
	//glm::ivec3 m_MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	//glm::ivec3 m_MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");
	//float step_probe = ElayGraphics::ResourceManager::getSharedDataByName<float>("step_probe");
	//
	//
	//glm::mat4 LightProjectionMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightProjectionMatrix");
	//glm::mat4 LightViewMatrix = ElayGraphics::ResourceManager::getSharedDataByName<glm::mat4>("LightViewMatrix");
	//float LightIntensity = ElayGraphics::ResourceManager::getSharedDataByName<float>("LightIntensity");
	//bool useLastIndirect = ElayGraphics::ResourceManager::getSharedDataByName<bool>("useLastIndirect");
	//float GIIntensity = ElayGraphics::ResourceManager::getSharedDataByName<float>("GIIntensity");
	//
	//
	//
	//int DispatchX = 0, DispatchY = 0, DispatchZ = 0;
	//glm::vec3 _coefficientVoxelSize(0);
	//for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	//{
	//	DispatchX++;
	//	_coefficientVoxelSize.x++;
	//}
	//
	//for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
	//{
	//	DispatchY++;
	//	_coefficientVoxelSize.y++;
	//}
	//
	//for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
	//{
	//	DispatchZ++;
	//	_coefficientVoxelSize.z++;
	//}
	//
	//auto LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	//int all_num = shDatas->all_coefficientSH9.size();
	//std::vector<float> cleanData(27 * all_num, 0);
	//updateSSBOBuffer(shBufferFloat, 27 * sizeof(float) * all_num, &cleanData[0], GL_STATIC_DRAW);
	//m_pShader->activeShader();
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surfelBuffer);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shBufferFloat);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, SurfelRadianceBuffer);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, lastshBufferFloat);
	//
	//LightDir = glm::normalize(glm::vec3(-LightDir));
	//m_pShader->setFloatUniformValue("lightDirection", LightDir.x, LightDir.y, LightDir.z);
	//m_pShader->setMat4UniformValue("u_LightVPMatrix", glm::value_ptr(LightProjectionMatrix * LightViewMatrix));
	//m_pShader->setFloatUniformValue("LightIntensity", LightIntensity);
	//m_pShader->setIntUniformValue("useLastIndirect", useLastIndirect);
	//m_pShader->setFloatUniformValue("_GIIntensity", GIIntensity);
	////大小
	//m_pShader->setFloatUniformValue("_coefficientVoxelGridSize", step_probe);
	////minAABB
	//m_pShader->setFloatUniformValue("_coefficientVoxelCorner", m_MinAABB[0], m_MinAABB[1], m_MinAABB[2], 1);
	////x,y,z方向的个数
	//m_pShader->setFloatUniformValue("_coefficientVoxelSize", _coefficientVoxelSize.x, _coefficientVoxelSize.y, _coefficientVoxelSize.z, 1.0);
	//
	//
	//glDispatchCompute(DispatchX, DispatchY, DispatchZ);
	//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	//glFlush();
	//
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, shBufferFloat);
	//GLvoid* p = (GLvoid*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	//std::vector<float> readShData(27 * all_num, 0);
	//memcpy(&readShData[0], p, readShData.size() *sizeof(float));
	//glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//updateSSBOBuffer(lastshBufferFloat, 27 * sizeof(float) * all_num, &readShData[0], GL_STATIC_DRAW);


}

