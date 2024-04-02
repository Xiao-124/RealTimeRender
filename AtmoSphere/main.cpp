#include "Interface.h"
#include "TerrainRenderPass.h"

int main()
{

	ElayGraphics::WINDOW_KEYWORD::setWindowSize(1280, 760);
	ElayGraphics::WINDOW_KEYWORD::setSampleNum(0);
	//ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(true);
	ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(false);
	//ElayGraphics::COMPONENT_CONFIG::setIsEnableGUI(true);


	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<TerrainRenderPass>("TerrainRenderPass", 0));

	ElayGraphics::App::initApp();
	ElayGraphics::App::updateApp();

	return 0;
}