#pragma once

#include "RenderPass.h"
#include <vector>
#include <GLM/glm.hpp>
#include <GL/glew.h>
class CConeTracingPass : public IRenderPass
{
public:
	CConeTracingPass(const std::string& vPassName, int vExcutionOrder);
	virtual ~CConeTracingPass();

	virtual void initV() override;

	virtual void updateV() override;

private:
	
};