#pragma once
#include "GUI.h"
#include <GLM/glm.hpp>

class CCustomGUI : public IGUI
{
public:
	CCustomGUI(const std::string& vName, int vExcutionOrder);
	virtual ~CCustomGUI();

	virtual void initV() override;
	virtual void updateV() override;

private:
	glm::vec3 m_LightPos = glm::vec3(-0.15, -1.13, -0.58);	//对应1.0的LightCameraSize，去除红光的话z=0.19
	glm::vec3 m_LightDir = m_LightDir = glm::normalize(glm::vec3(-0.5, -1.0, 0));	//-0.3, -1, 0
	float m_Intensity = 10.0f;
	bool useIndirect = true;
	float specularFactor = 0.2;
	
	int directType = 0;

	float GI_SpecularOffsetFactor = 3.0;
	float GI_SpecularAperture = 0.10;
	float GI_SpecularMaxT = 1.0;
	float GI_DiffuseOffsetFactor = 1.5;
	float GI_DiffuseAperture = 1.04;
	float GI_DiffuseConeAngleMix = 0.666;
	float GI_DiffuseMaxT = 1.0;
	float GI_OcclusionOffsetFactor = 3.0;
	float GI_OcculsionAperture = 0.1;
	float GI_DirectionalMaxT = 1.0;
	float GI_stepSize = 0.3;

};