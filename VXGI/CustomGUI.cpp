#include "CustomGUI.h"
#include "Interface.h"

CCustomGUI::CCustomGUI(const std::string& vName, int vExcutionOrder) : IGUI(vName, vExcutionOrder)
{
}

CCustomGUI::~CCustomGUI()
{
}

void CCustomGUI::initV()
{
	ElayGraphics::ResourceManager::registerSharedData("LightPos", m_LightPos);
	ElayGraphics::ResourceManager::registerSharedData("LightDir", m_LightDir);
	ElayGraphics::ResourceManager::registerSharedData("LightIntensity", m_Intensity);
	ElayGraphics::ResourceManager::registerSharedData("useIndirect", useIndirect);
	ElayGraphics::ResourceManager::registerSharedData("specularFactor", specularFactor);

	ElayGraphics::ResourceManager::registerSharedData("GI_SpecularOffsetFactor", GI_SpecularOffsetFactor);
	ElayGraphics::ResourceManager::registerSharedData("GI_SpecularAperture", GI_SpecularAperture );
	ElayGraphics::ResourceManager::registerSharedData("GI_SpecularMaxT", GI_SpecularMaxT);
	ElayGraphics::ResourceManager::registerSharedData("GI_DiffuseOffsetFactor", GI_DiffuseOffsetFactor);
	ElayGraphics::ResourceManager::registerSharedData("GI_DiffuseAperture", GI_DiffuseAperture);
	ElayGraphics::ResourceManager::registerSharedData("GI_DiffuseConeAngleMix", GI_DiffuseConeAngleMix);
	ElayGraphics::ResourceManager::registerSharedData("GI_DiffuseMaxT", GI_DiffuseMaxT);
	ElayGraphics::ResourceManager::registerSharedData("GI_OcclusionOffsetFactor", GI_OcclusionOffsetFactor);
	ElayGraphics::ResourceManager::registerSharedData("GI_OcculsionAperture", GI_OcculsionAperture);
	ElayGraphics::ResourceManager::registerSharedData("GI_DirectionalMaxT", GI_DirectionalMaxT);
	ElayGraphics::ResourceManager::registerSharedData("GI_stepSize", GI_stepSize);

	ElayGraphics::ResourceManager::registerSharedData("DirectLightType", directType);
}

void CCustomGUI::updateV()
{

	checkBox("useIndirect", useIndirect);
	glm::vec3 MinLightPos = glm::vec3(-1000), MaxLightPos = glm::vec3(1000);
	float LightPosDragSpeed = 0.01f;
	if (IGUI::dragScalarN("LightPos", ElayGraphics::EDataType::DataType_Float, &m_LightPos, 3, LightPosDragSpeed, &MinLightPos, &MaxLightPos))
		ElayGraphics::ResourceManager::updateSharedDataByName("LightPos", m_LightPos);

	std::vector<std::string> directLight = { "VoxelSample", "ShadowMap" };
	combo("DirectLightType", directType, directLight);
	
	directionWidget("LightDir", &m_LightDir[0]);
	sliderFloat("LightIntensity", &m_Intensity, 0, 100);

	sliderFloat("specularFactor", &specularFactor, 0, 1);
	

	float vMin0 = 0.02f, vMax0 = 1.5f;
	IGUI::dragScalarN("Specular Aperture", ElayGraphics::EDataType::DataType_Float, &GI_SpecularAperture, 1, 0.02f, &vMin0, &vMax0);

	float vMin1 = 0.1f, vMax1 = 10.0f;
	IGUI::dragScalarN("Specular Offset", ElayGraphics::EDataType::DataType_Float, &GI_SpecularOffsetFactor, 1, 0.1f, &vMin1, &vMax1);
	
	float vMin2 = 0.01f, vMax2 = 2.0f;
	IGUI::dragScalarN("Specular Max T", ElayGraphics::EDataType::DataType_Float, &GI_SpecularMaxT, 1, 0.01f, &vMin2, &vMax2);
	
	float vMin3 = 0.02f, vMax3 = 1.5f;
	IGUI::dragScalarN("Diffuse Aperture", ElayGraphics::EDataType::DataType_Float, &GI_DiffuseAperture, 1, 0.02f, &vMin3, &vMax3);
	
	float vMin4 = 0.01f, vMax4 = 1.0f;
	IGUI::dragScalarN("Diffuse Cone Angle", ElayGraphics::EDataType::DataType_Float, &GI_DiffuseConeAngleMix, 1, 0.01f, &vMin4, &vMax4);
	
	float vMin5 = 0.1f, vMax5 = 10.0f;
	IGUI::dragScalarN("Diffuse Offset", ElayGraphics::EDataType::DataType_Float, &GI_DiffuseOffsetFactor, 1, 0.1f, &vMin5, &vMax5);
	
	float vMin6 = 0.01f, vMax6 = 2.0f;
	IGUI::dragScalarN("Diffuse Max T", ElayGraphics::EDataType::DataType_Float, &GI_DiffuseMaxT, 1, 0.01f, &vMin6, &vMax6);
	
	float vMin7 = 0.01f, vMax7 = 1.5f;
	IGUI::dragScalarN("Occulsion Aperture", ElayGraphics::EDataType::DataType_Float, &GI_OcculsionAperture, 1, 0.01f, &vMin7, &vMax7);
	
	float vMin8 = 0.1f, vMax8 = 10.0f;
	IGUI::dragScalarN("Occulsion Offset", ElayGraphics::EDataType::DataType_Float, &GI_OcclusionOffsetFactor, 1, 0.1f, &vMin8, &vMax8);
	
	float vMin9 = 0.1f, vMax9 = 2.0f;
	IGUI::dragScalarN("DirectionalMaxT", ElayGraphics::EDataType::DataType_Float, &GI_DirectionalMaxT, 1, 0.1f, &vMin9, &vMax9);
	
	float vMin10 = 0.02f, vMax10 = 10.0f;
	IGUI::dragScalarN("Step Size", ElayGraphics::EDataType::DataType_Float, &GI_stepSize, 1, 0.1f, &vMin10, &vMax10);



	ElayGraphics::ResourceManager::updateSharedDataByName("LightIntensity", m_Intensity);
	ElayGraphics::ResourceManager::updateSharedDataByName("LightDir", normalize(m_LightDir));
	ElayGraphics::ResourceManager::updateSharedDataByName("useIndirect", useIndirect);
	ElayGraphics::ResourceManager::updateSharedDataByName("specularFactor", specularFactor);
	ElayGraphics::ResourceManager::updateSharedDataByName("DirectLightType", directType);

	ElayGraphics::ResourceManager::updateSharedDataByName("GI_SpecularOffsetFactor", GI_SpecularOffsetFactor);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_SpecularAperture", GI_SpecularAperture);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_SpecularMaxT", GI_SpecularMaxT);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_DiffuseOffsetFactor", GI_DiffuseOffsetFactor);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_DiffuseAperture", GI_DiffuseAperture);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_DiffuseConeAngleMix", GI_DiffuseConeAngleMix);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_DiffuseMaxT", GI_DiffuseMaxT);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_OcclusionOffsetFactor", GI_OcclusionOffsetFactor);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_OcculsionAperture", GI_OcculsionAperture);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_DirectionalMaxT", GI_DirectionalMaxT);
	ElayGraphics::ResourceManager::updateSharedDataByName("GI_stepSize", GI_stepSize);



}