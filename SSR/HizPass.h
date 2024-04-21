#pragma once
#include "RenderPass.h"
#include <GL/glew.h>
class CHizPass:public IRenderPass
{
public:
	CHizPass(const std::string& vPassName, int vExcutionOrder);
	virtual ~CHizPass();

	virtual void initV();
	virtual void updateV();

private:
	int m_FBO;

};