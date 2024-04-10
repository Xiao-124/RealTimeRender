#pragma once
#pragma once
#include "RenderPass.h"
#include <GL/glew.h>

class CSponza;



struct Surfel
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 albedo;
	float skyMask;
};

struct SurfelData
{
	std::vector<std::vector<Surfel>> all_surfelData;
};

class CLightProbePass : public IRenderPass
{
public:
	CLightProbePass(const std::string& vPassName, int vExcutionOrder, ElayGraphics::ERenderPassType vtype);
	virtual ~CLightProbePass();

	virtual void initV();
	virtual void updateV();

private:
	std::shared_ptr<CSponza> m_pSponza;

	std::vector<std::shared_ptr<ElayGraphics::STexture>> m_TextureConfig4Position;
	std::vector<std::shared_ptr<ElayGraphics::STexture>> m_TextureConfig4Albedos;
	std::vector<std::shared_ptr<ElayGraphics::STexture>> m_TextureConfig4Normals;
	std::vector<std::shared_ptr<ElayGraphics::STexture>> m_TextureConfig4Depths;
	std::vector<std::shared_ptr<ElayGraphics::STexture>> m_TextureConfig4Chebyshevs;
	glm::ivec3 m_MinAABB;
	glm::ivec3 m_MaxAABB;
	std::vector<int> m_FBOs;
	int m_BakeResolution = 128;
	glm::vec3 m_LightUpVector = glm::vec3(0, 1, 0);
	glm::vec3 m_BakeDir[6] = {
		glm::vec3(1,0,0),//+X
		glm::vec3(-1,0,0),//-X
		glm::vec3(0,1,0),//+Y
		glm::vec3(0,-1,0),//-Y
		glm::vec3(0,0,1),//+Z
		glm::vec3(0,0,-1)//+Z
	};

	float m_Fovy = glm::radians(45.0f);
	float m_Aspect = 1.0f;
	float m_Near = 0.1f;
	float m_Far = 100.0f;
	float step_probe = 2;

	std::shared_ptr<SurfelData> surfelDatas;
	GLuint surfelBuffer;
};