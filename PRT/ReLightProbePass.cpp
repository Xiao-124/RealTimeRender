
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

}

void CReLightProbePass::updateV()
{

	glm::vec3 m_MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	glm::vec3 m_MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");
	float step_probe = ElayGraphics::ResourceManager::getSharedDataByName<float>("step_probe");
	auto suefelDatas = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<SurfelData>>("SurfelDatas");
	GLuint surfelBuffer = ElayGraphics::ResourceManager::getSharedDataByName<GLuint>("SurfelBuffer");

	int Index = 0;
	int _coefficientSH9[27] = { 0 };

	std::vector<std::vector<int>> all_coefficientSH9;
	all_coefficientSH9.resize(suefelDatas->all_surfelData.size());
	for (int i = 0; i < all_coefficientSH9.size(); i++)
	{
		all_coefficientSH9[i].resize(27);
	}

	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	{
		for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
		{
			for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
			{
				
				updateSSBOBuffer(surfelBuffer, 32*16* sizeof(Surfel), &suefelDatas->all_surfelData[Index][0], GL_STATIC_DRAW);
				updateSSBOBuffer(shBuffer, 27 * sizeof(int), &_coefficientSH9[0], GL_STATIC_DRAW);
				m_pShader->activeShader();
				m_pShader->setFloatUniformValue("_probePos", i, j, k, 1);
				glDispatchCompute(1, 1, 1);
				glFlush();

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, shBuffer);
				GLvoid* p = (GLvoid*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
				memcpy(&all_coefficientSH9[Index][0], p, 27 * sizeof(int));
				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				Index++;
			}
		}
	}




}