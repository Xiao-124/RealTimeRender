#pragma once
#include "RenderPass.h"
#include <GL/glew.h>


//¹âÏß×¢Èë
class CInjectLightPass : public IRenderPass
{
public:
	CInjectLightPass(const std::string& vPassName, int vExecutionOrder);
	virtual ~CInjectLightPass();
	virtual void initV() override;
	virtual void updateV() override;
private:
	std::shared_ptr<ElayGraphics::STexture> Radiance3D;
	std::shared_ptr<CShader> MipmapShader;

};