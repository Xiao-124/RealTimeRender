#pragma once
#include "RenderPass.h"

class CModelLoad;

class CSponzaGBufferPass : public IRenderPass
{
public:
	CSponzaGBufferPass(const std::string& vPassName, int vExcutionOrder);
	virtual ~CSponzaGBufferPass();

	virtual void initV();
	virtual void updateV();

private:
	std::shared_ptr<CModelLoad> m_pSponza;
	int m_FBO;
};