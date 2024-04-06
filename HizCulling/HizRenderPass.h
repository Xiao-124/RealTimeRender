#pragma once

#include "RenderPass.h"


struct CullBbox
{
	glm::vec4 min;
	glm::vec4 max;
};


class HizRenderPass : public IRenderPass
{
public:
	HizRenderPass(const std::string& vPassName, int vExcutionOrder);
	~HizRenderPass();
	virtual void initV();
	virtual void updateV();

protected:
	GLuint m_FBO;

	std::shared_ptr<CShader> downSampleShader;
	std::shared_ptr<CShader> visibleShader;
	std::shared_ptr<CShader> screenShader;

	std::vector<glm::mat4> modelMatrixs;
	std::vector<CullBbox> bboxes;
	std::vector<int> cubeVisible;
	GLuint bbxoBuffer;
	GLuint visibleBuffer;



	std::vector<int> _calculateVisibleCube();
};