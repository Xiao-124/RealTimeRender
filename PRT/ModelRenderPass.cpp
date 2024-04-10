#include "ModelRenderPass.h"
#include "Sponza.h"
#include "Interface.h"
#include "Shader.h"
#include "Utils.h"
#include <GLFW/glfw3.h>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include "ReLightProbePass.h"
CModelRenderPass::CModelRenderPass(const std::string& vPassName, int vExecutionOrder) : IRenderPass(vPassName, vExecutionOrder)
{
}

CModelRenderPass::~CModelRenderPass()
{
}

//************************************************************************************
//Function:
void CModelRenderPass::initV()
{

	m_pShader = std::make_shared<CShader>("ModelRender_VS.glsl", "ModelRender_FS.glsl");
	m_pSponza = std::dynamic_pointer_cast<CSponza>(ElayGraphics::ResourceManager::getGameObjectByName("Sponza"));

	auto shDatas = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<SHData>>("shDatas");


	all_shbuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, shDatas->all_coefficientSH9.size() * shDatas->all_coefficientSH9[0].size() *sizeof(int), nullptr, GL_STATIC_DRAW,  2);
}

//************************************************************************************
//Function:
void CModelRenderPass::updateV()
{

	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glm::ivec3 m_MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	glm::ivec3 m_MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");
	float step_probe = ElayGraphics::ResourceManager::getSharedDataByName<float>("step_probe");

	glm::vec3 _coefficientVoxelSize(0,0,0);
	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
		_coefficientVoxelSize.x++;
	for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
		_coefficientVoxelSize.y++;
	for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
		_coefficientVoxelSize.z++;


	m_pShader->activeShader();
	//大小
	m_pShader->setFloatUniformValue("_coefficientVoxelGridSize", step_probe);
	//minAABB
	m_pShader->setFloatUniformValue("_coefficientVoxelCorner", m_MinAABB[0], m_MinAABB[1], m_MinAABB[2], 1);
	//x,y,z方向的个数
	m_pShader->setFloatUniformValue("_coefficientVoxelSize", _coefficientVoxelSize.x, _coefficientVoxelSize.y, _coefficientVoxelSize.z, 1.0);
	m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(m_pSponza->getModelMatrix()));
	
	auto shDatas = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<SHData>>("shDatas");

	std::vector<int> all_shdata;	
	for (int i = 0; i < shDatas->all_coefficientSH9.size(); i++)
	{
		for (int j = 0; j < shDatas->all_coefficientSH9[i].size(); j++)
		{
			all_shdata.push_back(shDatas->all_coefficientSH9[i][j]);
		}	
	}
	updateSSBOBuffer(all_shbuffer, all_shdata.size() * sizeof(int), &all_shdata[0], GL_STATIC_DRAW);
	
	
	m_pSponza->updateModel(*m_pShader);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}