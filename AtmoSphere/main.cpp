#include "Interface.h"
//#include "TerrainRenderPass.h"
#include "RenderTransmittanceLutPass.h"
#include "CustomGUI.h"
int main()
{

	ElayGraphics::WINDOW_KEYWORD::setWindowSize(1280, 720);
	ElayGraphics::WINDOW_KEYWORD::setSampleNum(0);
	//ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(true);
	ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(false);
	//ElayGraphics::COMPONENT_CONFIG::setIsEnableGUI(true);


	//ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<TerrainRenderPass>("TerrainRenderPass", 0));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<RenderTransmittanceLutPass>("RenderTransmittanceLutPass", 0));
	ElayGraphics::ResourceManager::registerSubGUI(std::make_shared<CCustomGUI>("CustomGUI", 1));

	ElayGraphics::App::initApp();
	ElayGraphics::App::updateApp();

	return 0;
}