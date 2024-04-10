#include "LightCamera.h"
#include "Interface.h"
#include <glm/gtc/matrix_transform.hpp>

CLightCamera::CLightCamera(const std::string& vGameObjectName, int vExecutionOrder) : IGameObject(vGameObjectName, vExecutionOrder)
{
}

CLightCamera::~CLightCamera()
{
}

void CLightCamera::initV()
{
	ElayGraphics::Camera::setMainCameraFarPlane(1000);
	ElayGraphics::Camera::setMainCameraPos({ -0.0730926, -1.18774, 2.28317 });
	ElayGraphics::Camera::setMainCameraFront({ 0.0633111, 0.0078539, -0.997963 });
	ElayGraphics::Camera::setMainCameraMoveSpeed(0.1);
	m_LightPos = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightPos");
	m_LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	if (abs(m_LightDir) == m_LightUpVector)
		m_LightDir.z += 0.01f;
	m_LightViewMatrix = glm::lookAt(m_LightPos, m_LightPos + m_LightDir, m_LightUpVector);
	m_LightProjectionMatrix = glm::ortho(-m_CameraSizeExtent, m_CameraSizeExtent, -m_CameraSizeExtent, m_CameraSizeExtent, 0.1f, 1000.0f);
	ElayGraphics::ResourceManager::registerSharedData("LightViewMatrix", m_LightViewMatrix);
	ElayGraphics::ResourceManager::registerSharedData("LightProjectionMatrix", m_LightProjectionMatrix);
	ElayGraphics::ResourceManager::registerSharedData("LightCameraAreaInWorldSpace", (2.0f * m_CameraSizeExtent) * (2.0f * m_CameraSizeExtent));
	ElayGraphics::ResourceManager::registerSharedData("m_CameraSizeExtent", m_CameraSizeExtent);
	ElayGraphics::ResourceManager::registerSharedData("Exposure", m_Exposure);


}

void CLightCamera::updateV()
{
//#ifdef _DEBUG
	m_LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	m_LightPos = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightPos");
	if (abs(m_LightDir) == m_LightUpVector)
		m_LightDir.z += 0.01f;

	glm::vec3 m_MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	glm::vec3 m_MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");
	glm::vec3 pos = (m_MinAABB + m_MaxAABB)*glm::vec3(0.5);

	//pos -= m_LightDir * 100.0f;

	m_LightViewMatrix = glm::lookAt(pos, glm::vec3(0,0,0), m_LightUpVector);


	m_LightProjectionMatrix = glm::ortho(-m_CameraSizeExtent, m_CameraSizeExtent, -m_CameraSizeExtent, m_CameraSizeExtent, 0.1f, 1000.0f);
	ElayGraphics::ResourceManager::updateSharedDataByName("LightViewMatrix", m_LightViewMatrix);
	ElayGraphics::ResourceManager::updateSharedDataByName("LightProjectionMatrix", m_LightProjectionMatrix);
//#endif
}