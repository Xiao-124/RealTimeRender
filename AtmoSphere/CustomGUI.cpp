
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
	ElayGraphics::ResourceManager::registerSharedData("uiCamHeight", uiCamHeight);
	SetupEarthAtmosphere(AtmosphereInfos);
}

void CCustomGUI::updateV()
{

	IGUI::sliderFloat("Height", &uiCamHeight, 0.001f, 2.0f * (AtmosphereInfos.top_radius - AtmosphereInfos.bottom_radius));
	ElayGraphics::ResourceManager::updateSharedDataByName("uiCamHeight", uiCamHeight);

	
}