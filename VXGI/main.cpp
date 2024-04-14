#include "Interface.h"
#include "Sponza.h"
#include "GBufferPass.h"
#include "ShadowMapPass.h"

#include "LightCamera.h"
#include "CustomGUI.h"
#include "VoxelizationPass.h"
#include "InjectLightPass.h"
#include "VoxelVisualizePass.h"
#include "ConeTracingPass.h"
int main()
{
	ElayGraphics::WINDOW_KEYWORD::setSampleNum(1);
	ElayGraphics::WINDOW_KEYWORD::setWindowSize(1280, 720);
	ElayGraphics::WINDOW_KEYWORD::setIsCursorDisable(false);
	ElayGraphics::COMPONENT_CONFIG::setIsEnableGUI(true);

	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CSponza>("Sponza", 1));
	ElayGraphics::ResourceManager::registerGameObject(std::make_shared<CLightCamera>("LightCamera", 2));
	
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CVoxelizationPass>("VoxelizationPass", 0, ElayGraphics::ERenderPassType::RenderPassType_Once));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<GBufferPass>("SponzaGBufferPass", 1));
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CShadowMapPass>("ShadowMapPass", 2));

	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CInjectLightPass>("InjectLightPass", 3) );
	ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CConeTracingPass>("ConeTracingPass", 4));
	

	//µ÷ÊÔ£ºÌåËØ»¯
	//ElayGraphics::ResourceManager::registerRenderPass(std::make_shared<CVoxelVisualizePass>("VoxelVisualizePass", 5));

	ElayGraphics::ResourceManager::registerSubGUI(std::make_shared<CCustomGUI>("CustomGUI", 1));

	ElayGraphics::App::initApp();
	ElayGraphics::App::updateApp();

	return 0;
}