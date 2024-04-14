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
	//ElayGraphics::Camera::setMainCameraPos({ 0, 0, 0 });

	ElayGraphics::Camera::setMainCameraFront({ 0.0633111, 0.0078539, -0.997963 });
	ElayGraphics::Camera::setMainCameraMoveSpeed(1.0);
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

	float MaxCoord = ElayGraphics::ResourceManager::getSharedDataByName<float>("MaxCoord");
	auto LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	glm::vec3 DPos = 1.5f * MaxCoord * -glm::normalize(LightDir);
	glm::vec3 up = glm::vec3(1.0f, 0.0f, 0.0f);
	float coord = 1.5 * MaxCoord;
	glm::mat4 DirectionalShadowProj = glm::ortho(-coord, coord, -coord, coord, 0.01f, 2 * coord);
	glm::mat4 DView = glm::lookAt(DPos, glm::vec3(0.0f, 0.0f, 0.0f), up);



	//m_LightDir = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightDir");
	//m_LightPos = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("LightPos");
	//if (abs(m_LightDir) == m_LightUpVector)
	//	m_LightDir.z += 0.01f;
	//m_LightViewMatrix = glm::lookAt(m_LightPos, m_LightPos + m_LightDir, m_LightUpVector);
	//m_LightProjectionMatrix = glm::ortho(-m_CameraSizeExtent, m_CameraSizeExtent, -m_CameraSizeExtent, m_CameraSizeExtent, 0.1f, 1000.0f);
	//ElayGraphics::ResourceManager::updateSharedDataByName("LightViewMatrix", m_LightViewMatrix);
	//ElayGraphics::ResourceManager::updateSharedDataByName("LightProjectionMatrix", m_LightProjectionMatrix);

	m_LightViewMatrix = DView;
	m_LightProjectionMatrix = DirectionalShadowProj;
	ElayGraphics::ResourceManager::updateSharedDataByName("LightViewMatrix", DView);
	ElayGraphics::ResourceManager::updateSharedDataByName("LightProjectionMatrix", DirectionalShadowProj);



}