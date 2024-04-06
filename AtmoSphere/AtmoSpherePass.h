#pragma once
#include "RenderPass.h"
#include "AtmosphereParameter.h"


class AtmoSpherePass : public IRenderPass
{
public:
	AtmoSpherePass(const std::string& vPassName, int vExcutionOrder);
    ~AtmoSpherePass();

    virtual void initV();
    virtual void updateV();

protected:
	SkyAtmosphereConstantBufferStructure updateSkyAtmosphereConstant();
	void renderTerrian();
	void renderTransmittanceLut();
	void renderMultiScatter();
	void renderSkyLut();
	void renderCameraVolume();
	void renderRayMarching();


	bool currentShadowPermutation = false;
	float currentMultipleScatteringFactor = 1.0f;
	bool currentFastSky = true;
	bool currentAerialPerspective = true;
	bool currentColoredTransmittance = false;
	float currentAtmosphereHeight = -1.0f;
	const unsigned int MultiScatteringLUTRes = 32;
	float mSunIlluminanceScale = 1.0f;
	float viewPitch = 0.0f;
	float viewYaw = 0.0f;
	int NumScatteringOrder = 4;
	uint32 mFrameId = 0;

	int uiViewRayMarchMinSPP = 4;
	int uiViewRayMarchMaxSPP = 14;
	LookUpTablesInfo LutsInfo;
	AtmosphereInfo AtmosphereInfos;
	GLuint AtmosphereBuffer;

	CommonConstantBufferStructure mConstantBufferCPU;
	GLuint CommonConstantBuffer;

	glm::mat4 mViewProjMat;
	glm::mat4 mProjMat;
	glm::mat4 mViewMat;

	float uiCamHeight = 0.5f;
	float uiCamForward = -1.0f;
	glm::vec3 mCamPosFinal;
	glm::vec3 mViewDir = glm::vec3(0, 1, 0);
private:
	void _updateCommonConstantBuffer();


    GLuint TransmittanceFBO;
	std::shared_ptr<CShader> TransmittanceShader;


	std::shared_ptr<CShader> multiScatterShader;

	GLuint SkyLutFBO;
	std::shared_ptr<CShader> skyLutShader;
	

	
	std::shared_ptr<CShader> cameraVolumeShader;

	GLuint RayMarchingFBO;
	std::shared_ptr<CShader> rayMarchingShader;
	

	std::shared_ptr<CShader> postShader;

	GLuint TerrianFBO;
	std::shared_ptr<CShader> terrianShader;
};