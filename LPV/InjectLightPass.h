#pragma once
#include "RenderPass.h"
#include <GL/glew.h>


//¹âÏß×¢Èë
class CInjectLightPass : public IRenderPass
{
public:
	CInjectLightPass(const std::string& vPassName, int vExecutionOrder);
	virtual ~CInjectLightPass();

	virtual void initV() override;
	virtual void updateV() override;
private:
	glm::ivec3 m_Dimensions;
	std::shared_ptr<ElayGraphics::STexture> TextureConfig4LPVGridR;
	std::shared_ptr<ElayGraphics::STexture> TextureConfig4LPVGridG;
	std::shared_ptr<ElayGraphics::STexture> TextureConfig4LPVGridB;
	std::shared_ptr<ElayGraphics::STexture> TextureConfig4LPVGV;
};