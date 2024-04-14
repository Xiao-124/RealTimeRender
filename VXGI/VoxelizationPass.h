#pragma once

#include "RenderPass.h"
#include <GL/glew.h>

class CSponza;
class CVoxelizationPass : public IRenderPass
{
public:
	CVoxelizationPass(const std::string& vPassName, int vExecutionOrder, ElayGraphics::ERenderPassType vtype);
	virtual ~CVoxelizationPass();

	virtual void initV() override;
	virtual void updateV() override;



private:

	std::shared_ptr<CSponza> m_pSponza;
	std::shared_ptr<ElayGraphics::STexture> voxelTexture;
	std::shared_ptr<ElayGraphics::STexture> voxelNormalTexture;


	glm::vec3 min_AABB;
	glm::vec3 Dimensions;
	glm::mat4 projX;
	glm::mat4 projY;
	glm::mat4 projZ;
	glm::mat4 projectMat;

	int VoxelResolution = 256;
	float VoxelWorldSize = 150.0f;
	int isinit = 0;
};