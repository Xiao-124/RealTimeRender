#include "ProbeDebugPass.h"
#include "Interface.h"
#include "Shader.h"
#include "Utils.h"
#include <GLFW/glfw3.h>
#include "Sponza.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
CProbeDebugPass::CProbeDebugPass(const std::string& vPassName, int vExecutionOrder) : IRenderPass(vPassName, vExecutionOrder)
{
}

CProbeDebugPass::~CProbeDebugPass()
{
}

//************************************************************************************
//Function:
void CProbeDebugPass::initV()
{

	m_pShader = std::make_shared<CShader>("ProbeDebug_VS.glsl", "ProbeDebug_FS.glsl");
	probeRenderShader = std::make_shared<CShader>("ProbeSphere_VS.glsl", "ProbeSphere_FS.glsl");
}

//************************************************************************************
//Function:
void CProbeDebugPass::updateV()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);

	


	glm::ivec3 m_MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	glm::ivec3 m_MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");
	float step_probe = ElayGraphics::ResourceManager::getSharedDataByName<float>("step_probe");
	glm::vec3 _coefficientVoxelSize(0);
	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	{
		_coefficientVoxelSize.x++;
	}

	for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
	{
		_coefficientVoxelSize.y++;
	}

	for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
	{
		_coefficientVoxelSize.z++;
	}
	
	GLuint shBufferFloat = ElayGraphics::ResourceManager::getSharedDataByName<GLuint>("AllshBufferFloat");
	GLuint SurfelRadianceBuffer = ElayGraphics::ResourceManager::getSharedDataByName<GLuint>("SurfelRadianceBuffer");
	GLuint surfelBuffer = ElayGraphics::ResourceManager::getSharedDataByName<GLuint>("surfelBuffer");


	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surfelBuffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, shBufferFloat);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, SurfelRadianceBuffer);
	
	glm::vec3 probeIndex = glm::vec3(2,1,4);
	//for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe , probeIndex.x ++)
	//{
	//	for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe , probeIndex.y++)
	//	{
	//		for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe , probeIndex.z++)
			{
				m_pShader->activeShader();
				m_pShader->setFloatUniformValue("_coefficientVoxelSize", _coefficientVoxelSize.x, _coefficientVoxelSize.y, _coefficientVoxelSize.z, 1.0);

				glm::mat4 Model = glm::mat4(1);
				//glm::vec3 Position = glm::vec3(i, j, k);
				//Model = glm::translate(Model, Position);
				Model = glm::scale(Model, glm::vec3(0.05, 0.05, 0.05));
				m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(Model));
				m_pShader->setFloatUniformValue("probeIndex3", probeIndex.x, probeIndex.y, probeIndex.z);

				//drawSphere();

				glBindVertexArray(ElayGraphics::ResourceManager::getOrCretaeSphereVAO());
				glDrawElements(GL_TRIANGLE_STRIP, 8320, GL_UNSIGNED_INT, 0);
				glDrawElementsInstanced(GL_TRIANGLE_STRIP, 8320, GL_UNSIGNED_INT, 0, 256);
				glBindVertexArray(0);

				probeRenderShader->activeShader();
				glm::mat4 Model1 = glm::mat4(1);
				glm::vec3 Position = glm::vec3(m_MinAABB) + probeIndex *step_probe;
				Model1 = glm::translate(Model1, Position);
				Model1 = glm::scale(Model1, glm::vec3(0.2, 0.2, 0.2));
				probeRenderShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(Model1));
				drawSphere();


			}
	//	}
	//}
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}