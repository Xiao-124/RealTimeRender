#pragma once
#include "RenderPass.h"
#include <GL/glew.h>

struct SHData
{
	std::vector<std::vector<int>> all_coefficientSH9;
};

class CReLightProbePass : public IRenderPass
{
public:
	CReLightProbePass(const std::string& vPassName, int vExcutionOrder);
	virtual ~CReLightProbePass();
	virtual void initV();
	virtual void updateV();
private:
	GLuint shBuffer;
	GLuint shBufferFloat;
	GLuint surfelBuffer;
	std::shared_ptr<SHData> shDatas;
};