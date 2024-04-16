#pragma once
#include "RenderPass.h"
#include <GL/glew.h>

class CSponza;
class CIndirectLightPass : public IRenderPass
{
public:
	CIndirectLightPass(const std::string& vPassName, int vExcutionOrder);
	virtual ~CIndirectLightPass();

	virtual void initV();
	virtual void updateV();

private:
	int m_FBO;
	GLuint all_shbuffer;
	std::shared_ptr<CSponza> m_pSponza;
};