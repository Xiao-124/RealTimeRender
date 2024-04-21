#include "Interface.h"
#include "Sponza.h"
#include "GBufferPass.h"
#include "HizPass.h"
#include "ScreenSpacePass.h"
int main()
{
	ElayGraphics::WINDOW_KEYWORD::setWindowSize(1280, 720);
	ElayGraphics::WINDOW_KEYWORD::setSampleNum(0);
	ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(false);
	ElayGraphics::COMPONENT_CONFIG::setIsEnableGUI(true);
	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CSponza>("Sponza", 1));

	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<GBufferPass>("GBufferPass", 0));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CHizPass>("CHizPass", 1));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CScreenSpacePass>("CScreenSpacePass", 2));


	ElayGraphics::App::initApp();
	ElayGraphics::App::updateApp();
	return 0;
}