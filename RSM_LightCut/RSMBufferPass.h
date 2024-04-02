#pragma once
#include "RenderPass.h"
#include <GLM/glm.hpp>

class CModelLoad;

class CRSMBufferPass : public IRenderPass
{
public:
	CRSMBufferPass(const std::string& vPassName, int vExcutionOrder);
	virtual ~CRSMBufferPass();

	virtual void initV();
	virtual void updateV();

private:
	std::shared_ptr<CModelLoad> m_pSponza;
	int m_FBO = -1;
	int m_RSMResolution = 32;	//1024
};