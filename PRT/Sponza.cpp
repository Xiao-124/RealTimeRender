#include "Sponza.h"
#include "Interface.h"
#include "AABB.h"
#include <GLM/gtc/type_ptr.hpp>
#include <GLM/glm.hpp>
#include <GLM/gtx/component_wise.hpp> 
CSponza::CSponza(const std::string& vGameObjectName, int vExecutionOrder) : IGameObject(vGameObjectName, vExecutionOrder)
{
}

CSponza::~CSponza()
{
}

void CSponza::initV()
{
	setModel(ElayGraphics::ResourceManager::getOrCreateModel("../Model/sponza/Sponza.obj"));
	m_AABB = getAABB();
	m_Min = m_AABB->getMin();
	m_Max = m_AABB->getMax();
	ElayGraphics::ResourceManager::registerSharedData("MinAABB", m_Min);
	ElayGraphics::ResourceManager::registerSharedData("MaxAABB", m_Max);

	float MaxCoord = 0;
	glm::vec3 midpoint = (m_Min + m_Max) * 0.5f;
	MaxCoord = glm::compMax(glm::abs(midpoint - m_Max)) + 1.0;
	ElayGraphics::ResourceManager::registerSharedData("MaxCoord", MaxCoord);

}

void CSponza::updateV()
{
}