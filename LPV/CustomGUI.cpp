#include "CustomGUI.h"
#include "Interface.h"

CCustomGUI::CCustomGUI(const std::string& vName, int vExcutionOrder) : IGUI(vName, vExcutionOrder)
{
}

CCustomGUI::~CCustomGUI()
{
}

void CCustomGUI::initV()
{
	ElayGraphics::ResourceManager::registerSharedData("LightPos", m_LightPos);
	ElayGraphics::ResourceManager::registerSharedData("LightDir", m_LightDir);
	ElayGraphics::ResourceManager::registerSharedData("LightIntensity", m_Intensity);
	ElayGraphics::ResourceManager::registerSharedData("useIndirect", useIndirect);
}

void CCustomGUI::updateV()
{

	checkBox("useIndirect", useIndirect);
	sliderFloat("LightIntensity", &m_Intensity, 0, 100);

	glm::vec3 MinLightPos = glm::vec3(-1000), MaxLightPos = glm::vec3(1000);
	float LightPosDragSpeed = 0.01f;
	if (IGUI::dragScalarN("LightPos", ElayGraphics::EDataType::DataType_Float, &m_LightPos, 3, LightPosDragSpeed, &MinLightPos, &MaxLightPos))
		ElayGraphics::ResourceManager::updateSharedDataByName("LightPos", m_LightPos);

	
	directionWidget("LightDir", &m_LightDir[0]);

	ElayGraphics::ResourceManager::updateSharedDataByName("LightIntensity", m_Intensity);
	ElayGraphics::ResourceManager::updateSharedDataByName("LightDir", normalize(m_LightDir));
	ElayGraphics::ResourceManager::updateSharedDataByName("useIndirect", useIndirect);

}