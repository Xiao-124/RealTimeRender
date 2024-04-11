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

private:
	glm::ivec3 m_Dimensions;

	std::shared_ptr<ElayGraphics::STexture> m_TextureConfig4LPVAccumulatorR;
	std::shared_ptr<ElayGraphics::STexture> m_TextureConfig4LPVAccumulatorG;
	std::shared_ptr<ElayGraphics::STexture> m_TextureConfig4LPVAccumulatorB;
};