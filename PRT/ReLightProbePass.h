#pragma once
#include "RenderPass.h"
#include <GL/glew.h>
class CReLightProbePass : public IRenderPass
{
public:
	CReLightProbePass(const std::string& vPassName, int vExcutionOrder);
	virtual ~CReLightProbePass();

	virtual void initV();
	virtual void updateV();


private:
	GLuint shBuffer;




};