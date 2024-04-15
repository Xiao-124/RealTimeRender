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
	ElayGraphics::ResourceManager::registerSharedData("GIIntensity", _GIIntensity);
	ElayGraphics::ResourceManager::registerSharedData("useIndirect", useIndirect);
	ElayGraphics::ResourceManager::registerSharedData("useLastIndirect", useLastIndirect);
}

void CCustomGUI::updateV()
{

	checkBox("useIndirect", useIndirect);
	checkBox("useLastIndirect", useLastIndirect);
	glm::vec3 MinLightPos = glm::vec3(-1000), MaxLightPos = glm::vec3(1000);
	float LightPosDragSpeed = 0.01f;
	if (IGUI::dragScalarN("LightPos", ElayGraphics::EDataType::DataType_Float, &m_LightPos, 3, LightPosDragSpeed, &MinLightPos, &MaxLightPos))
		ElayGraphics::ResourceManager::updateSharedDataByName("LightPos", m_LightPos);
	
	IGUI::sliderFloat("GIIntensity", &_GIIntensity, 0, 10);
	directionWidget("LightDir", &m_LightDir[0]);
	ElayGraphics::ResourceManager::updateSharedDataByName("LightDir", normalize(m_LightDir));
	ElayGraphics::ResourceManager::updateSharedDataByName("GIIntensity", _GIIntensity);
	ElayGraphics::ResourceManager::updateSharedDataByName("useIndirect", useIndirect);
	ElayGraphics::ResourceManager::updateSharedDataByName("useLastIndirect", useLastIndirect);
}