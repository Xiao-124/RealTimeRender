#include "Interface.h"
#include "LightProbePass.h"
#include "Sponza.h"
#include "ReLightProbePass.h"
#include "CustomGUI.h"
#include "LightCamera.h"
#include "ShadowMapPass.h"
#include "GBufferPass.h"
#include "DirectLightPass.h"
#include "IndirectLightPass.h"
#include "ScreenQuadPass.h"
#include "ProbeDebugPass.h"

int main()
{
	ElayGraphics::WINDOW_KEYWORD::setWindowSize(1280, 720);
	ElayGraphics::WINDOW_KEYWORD::setSampleNum(0);
	ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(false);
	ElayGraphics::COMPONENT_CONFIG::setIsEnableGUI(true);
	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CSponza>("Sponza", 1));
	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CLightCamera>("LightCamera", 2));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CLightProbePass>("LightProbePass", 0, ElayGraphics::ERenderPassType::RenderPassType_Once));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CShadowMapPass>("ShadowMapPass", 1));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CReLightProbePass>("ReLightProbePass", 2));
	


	ElayGraphics::ResourceManager::registerSubGUI(std::make_shared<CCustomGUI>("CustomGUI", 1));
	ElayGraphics::App::initApp();
	ElayGraphics::App::updateApp();
	return 0;
}