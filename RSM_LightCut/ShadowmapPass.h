#pragma once
#include "RenderPass.h"

class CModelLoad;

class CShadowmapPass : public IRenderPass
{
public:
	CShadowmapPass(const std::string& vPassName, int vExcutionOrder);
	virtual ~CShadowmapPass();

	virtual void initV() override;
	virtual void updateV() override;

private:
	std::shared_ptr<CModelLoad> m_pSponza;
	int m_FBO = -1;
	int m_ShadowmapResolution = 1024;	//1024
};