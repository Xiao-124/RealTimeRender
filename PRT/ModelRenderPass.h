#pragma once
#include "RenderPass.h"
#include <GL/glew.h>

class CSponza;
class CModelRenderPass : public IRenderPass
{
public:
	CModelRenderPass(const std::string& vPassName, int vExecutionOrder);
	virtual ~CModelRenderPass();
	virtual void initV() override;
	virtual void updateV() override;
private:
	std::shared_ptr<CSponza> m_pSponza;
	GLuint all_shbuffer;
}; 
