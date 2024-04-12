#pragma once
#include "RenderPass.h"
#include <GL/glew.h>


//¹âÏß×¢Èë
class CInjectLightPass : public IRenderPass
{
public:
	CInjectLightPass(const std::string& vPassName, int vExecutionOrder);
	virtual ~CInjectLightPass();
	void drawVPLVAO();
	virtual void initV() override;
	virtual void updateV() override;
private:
	GLuint m_VPLVAO = -1;
	glm::ivec3 m_Dimensions;
	int RSMResolution;

	std::shared_ptr<ElayGraphics::STexture> TextureConfig4LPVGridR;
	std::shared_ptr<ElayGraphics::STexture> TextureConfig4LPVGridG;
	std::shared_ptr<ElayGraphics::STexture> TextureConfig4LPVGridB;
	std::shared_ptr<ElayGraphics::STexture> TextureConfig4LPVGV;

	GLuint m_FBO;
};