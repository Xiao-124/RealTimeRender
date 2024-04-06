
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
	ElayGraphics::ResourceManager::registerSharedData("useHiz", useHiz);
	ElayGraphics::ResourceManager::registerSharedData("useTemporal", useTemporal);
}

void CCustomGUI::updateV()
{
	IGUI::checkBox("useHiz", useHiz);
	ElayGraphics::ResourceManager::updateSharedDataByName("useHiz", useHiz);
	if (useHiz)
	{
		IGUI::checkBox("useTemporal", useTemporal);
	}
	ElayGraphics::ResourceManager::updateSharedDataByName("useTemporal", useTemporal);
}