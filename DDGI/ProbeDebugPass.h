#pragma once
#pragma once
#include "RenderPass.h"
#include <GL/glew.h>
class CSponza;

class CProbeDebugPass : public IRenderPass
{
public:
	CProbeDebugPass(const std::string& vPassName, int vExecutionOrder);
	virtual ~CProbeDebugPass();

	virtual void initV() override;
	virtual void updateV() override;

private:
	glm::ivec3 m_Min;
	glm::ivec3 m_Max;

	std::shared_ptr<CShader> probeRenderShader;
};