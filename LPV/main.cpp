#include "Interface.h"
#include "Sponza.h"
#include "GBufferPass.h"
#include "ScreenQuadPass.h"
#include "RSMBufferPass.h"
#include "LightCamera.h"
#include "CustomGUI.h"
#include "DirectLightPass.h"
#include "AABBDebugPass.h"
#include "InjectLightPass.h"
#include "PropagationLightPass.h"
#include "IndirectLightPass.h"

int main()
{
	ElayGraphics::WINDOW_KEYWORD::setSampleNum(1);
	ElayGraphics::WINDOW_KEYWORD::setWindowSize(1280, 720);
	ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(false);
	ElayGraphics::COMPONENT_CONFIG::setIsEnableGUI(true);

	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CSponza>("Sponza", 1));
	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CLightCamera>("LightCamera", 2));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<GBufferPass>("SponzaGBufferPass", 0));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CRSMBufferPass>("RSMBufferPass", 1));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CDirectLightPass>("DirectLightPass", 2));

	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CInjectLightPass>("InjectLightPass", 3));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CPropagationLightPass>("PropagationLightPass", 4));


	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CIndirectLightPass>("IndirectLightPass", 5));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CScreenQuadPass>("ScreenQuadPass", 6));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CAABBDebugPass>("AABBDebugPass", 7));


	ElayGraphics::ResourceManager::registerSubGUI(std::make_shared<CCustomGUI>("CustomGUI", 1));

	ElayGraphics::App::initApp();
	ElayGraphics::App::updateApp();

	return 0;
}