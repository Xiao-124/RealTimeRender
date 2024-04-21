#pragma once
#include "RenderPass.h"
#include <GLM/glm.hpp>
#include <GL/glew.h>


class CScreenSpacePass : public IRenderPass
{
public:
	CScreenSpacePass(const std::string& vPassName, int vExcutionOrder);
	virtual ~CScreenSpacePass();

	virtual void initV();
	virtual void updateV();

private:
	int m_FBO = -1;
};