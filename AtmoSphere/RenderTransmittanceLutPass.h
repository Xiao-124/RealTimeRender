#pragma once
#include "RenderPass.h"
#include "AtmosphereParameter.h"


class RenderTransmittanceLutPass : public IRenderPass
{
public:
	struct CommonConstantBufferStructure
	{
		glm::mat4 gViewProjMat;
		glm::vec4 gColor;
		glm::vec3 gSunIlluminance;

		int gScatteringMaxPathDepth;
		unsigned int gResolution[2];
		float gFrameTimeSec;
		float gTimeSec;

		unsigned int gMouseLastDownPos[2];
		unsigned int gFrameId;
		unsigned int gTerrainResolution;
		float gScreenshotCaptureActive;

		float RayMarchMinMaxSPP[2];
		float pad[2];
	};

    RenderTransmittanceLutPass(const std::string& vPassName, int vExcutionOrder);
    ~RenderTransmittanceLutPass();

    virtual void initV();
    virtual void updateV();

protected:

	SkyAtmosphereConstantBufferStructure updateSkyAtmosphereConstant();
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


    GLuint m_FBO;
	std::shared_ptr<CShader> multiScatterShader;

	std::shared_ptr<CShader> skyLutShader;
	GLuint m_FBO2;

	GLuint m_FBO3;
	std::shared_ptr<CShader> cameraVolumeShader;

	std::shared_ptr<CShader> rayMarchingShader;
	

	std::shared_ptr<CShader> postShader;

	GLuint m_FBO5;
	std::shared_ptr<CShader> terrianShader;
};