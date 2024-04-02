#include "Interface.h"
#include "ModelLoad.h"
#include "SponzaGBufferPass.h"
#include "ScreenQuadPass.h"
#include "RSMBufferPass.h"
#include "ShadingWithRSMPass.h"
#include "CalculateVPLRadiusPass.h"
#include "ShadowmapPass.h"
#include "LightCamera.h"
#include "CustomGUI.h"

int main()
{
	ElayGraphics::WINDOW_KEYWORD::setWindowSize(1280, 760);
	ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(false);
	ElayGraphics::COMPONENT_CONFIG::setIsEnableGUI(true);

	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CModelLoad>("ModelLoad", 1));
	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CLightCamera>("LightCamera", 2));

	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CSponzaGBufferPass>("SponzaGBufferPass", 1));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CRSMBufferPass>("RSMBufferPass", 2));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CShadowmapPass>("ShadowmapPass", 3));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CCalculateVPLRadiusPass>("CalculateVPLRadiusPass", 4));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CShadingWithRSMPass>("ShadingWithRSMPass", 5));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CScreenQuadPass>("ScreenQuadPass", 6));

	ElayGraphics::ResourceManager::registerSubGUI(std::make_shared<CCustomGUI>("CustomGUI", 1));

	ElayGraphics::App::initApp();
	ElayGraphics::App::updateApp();

	return 0;
}