
#include <GL/glew.h>
#include <iostream>
#include "RenderPass.h"
class CVoxelVisualizePass :public IRenderPass
{
public:
	CVoxelVisualizePass(const std::string& vPassName, int vExecutionOrder);
	~CVoxelVisualizePass();
	virtual void initV() override;
	virtual void updateV() override;
private:
	std::shared_ptr<CShader> visFaceShader;
	std::shared_ptr<ElayGraphics::STexture> VoxelVisFrontFace;
	std::shared_ptr<ElayGraphics::STexture> VoxelVisBackFace;
	std::shared_ptr<ElayGraphics::STexture> VoxelVisOut;
		 
	GLuint mFBO;
	GLuint cubeVAO = 0;
	GLuint cubeVBO = 0;
};