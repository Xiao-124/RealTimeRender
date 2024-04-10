#include "Interface.h"
//#include "TerrainRenderPass.h"
#include "LightProbePass.h"
#include "Sponza.h"
#include "ReLightProbePass.h"
#include "ModelRenderPass.h"
int main()
{

	ElayGraphics::WINDOW_KEYWORD::setWindowSize(1280, 720);
	ElayGraphics::WINDOW_KEYWORD::setSampleNum(0);
	//ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(true);
	ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(false);
	//ElayGraphics::COMPONENT_CONFIG::setIsEnableGUI(true);

	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CSponza>("Sponza", 1));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CLightProbePass>("LightProbePass", 0, ElayGraphics::ERenderPassType::RenderPassType_Once));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CReLightProbePass>("ReLightProbePass", 1));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CModelRenderPass>("CModelRenderPass", 2));

	ElayGraphics::App::initApp();
	ElayGraphics::App::updateApp();

	return 0;
}