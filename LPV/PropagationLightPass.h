#pragma once
#include "RenderPass.h"
#include <GL/glew.h>

class CPropagationLightPass : public IRenderPass
{
public:
	CPropagationLightPass(const std::string& vPassName, int vExecutionOrder);
	virtual ~CPropagationLightPass();
	virtual void initV() override;
	virtual void updateV() override;


	void drawLPVVAO();
private:
	glm::ivec3 m_Dimensions;
	unsigned int m_FBO;
	int m_VPLVAO = -1;
	int m_ProgationNum = 8;
	std::vector<std::shared_ptr<ElayGraphics::STexture>> m_TextureConfig4LPVGridR;
	std::vector<std::shared_ptr<ElayGraphics::STexture>> m_TextureConfig4LPVGridG;
	std::vector<std::shared_ptr<ElayGraphics::STexture>> m_TextureConfig4LPVGridB;


	std::shared_ptr<ElayGraphics::STexture> m_TextureConfig4LPVAccumulatorR;
	std::shared_ptr<ElayGraphics::STexture> m_TextureConfig4LPVAccumulatorG;
	std::shared_ptr<ElayGraphics::STexture> m_TextureConfig4LPVAccumulatorB;
};