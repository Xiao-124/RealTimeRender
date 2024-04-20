#include "LightProbePass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include "Sponza.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>


CLightProbePass::CLightProbePass(const std::string& vPassName, int vExcutionOrder, ElayGraphics::ERenderPassType vtype) : IRenderPass(vPassName, vExcutionOrder, vtype)
{


}

CLightProbePass::~CLightProbePass()
{
}

void CLightProbePass::initV()
{
	m_pShader = std::make_shared<CShader>("BakeProbe_VS.glsl", "BakeProbe_FS.glsl");
	m_pSponza = std::dynamic_pointer_cast<CSponza>(ElayGraphics::ResourceManager::getGameObjectByName("Sponza"));
	m_MinAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MinAABB");
	m_MaxAABB = ElayGraphics::ResourceManager::getSharedDataByName<glm::vec3>("MaxAABB");
	

	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	{
		for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
		{
			for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
			{
				auto TextureConfig4Position = std::make_shared<ElayGraphics::STexture>();
				auto TextureConfig4Albedo = std::make_shared<ElayGraphics::STexture>();
				auto TextureConfig4Normal = std::make_shared<ElayGraphics::STexture>();
				auto TextureConfig4Chebyshev = std::make_shared<ElayGraphics::STexture>();

				TextureConfig4Position->InternalFormat = GL_RGB32F;
				TextureConfig4Position->ExternalFormat = GL_RGB;
				TextureConfig4Position->DataType  = GL_FLOAT;
				TextureConfig4Position->TextureType = ElayGraphics::STexture::ETextureType::TextureCubeMap;
				TextureConfig4Position->Width = m_BakeResolution;
				TextureConfig4Position->Height = m_BakeResolution;
				TextureConfig4Position->BorderColor = std::vector<float>{ 0,0,0 };
				TextureConfig4Position->Type4WrapR = GL_CLAMP_TO_EDGE;
				TextureConfig4Position->Type4WrapS = GL_CLAMP_TO_EDGE;
				TextureConfig4Position->Type4WrapT = GL_CLAMP_TO_EDGE;

				TextureConfig4Albedo->InternalFormat = TextureConfig4Normal->InternalFormat = GL_RGBA32F;
				TextureConfig4Albedo->ExternalFormat = TextureConfig4Normal->ExternalFormat = GL_RGBA;
				TextureConfig4Albedo->DataType = TextureConfig4Normal->DataType = GL_FLOAT;
				TextureConfig4Albedo->TextureType = TextureConfig4Normal->TextureType = ElayGraphics::STexture::ETextureType::TextureCubeMap;
				TextureConfig4Albedo->Width = TextureConfig4Normal->Width = m_BakeResolution;
				TextureConfig4Albedo->Height = TextureConfig4Normal->Height = m_BakeResolution;
				TextureConfig4Albedo->Type4WrapR = TextureConfig4Normal->Type4WrapR =  GL_CLAMP_TO_EDGE;
				TextureConfig4Albedo->Type4WrapS = TextureConfig4Normal->Type4WrapS = GL_CLAMP_TO_EDGE;
				TextureConfig4Albedo->Type4WrapT = TextureConfig4Normal->Type4WrapT = GL_CLAMP_TO_EDGE;

				TextureConfig4Chebyshev->InternalFormat = GL_RGB32F;
				TextureConfig4Chebyshev->ExternalFormat = GL_RGB;
				TextureConfig4Chebyshev->DataType = GL_FLOAT;
				TextureConfig4Chebyshev->TextureType = ElayGraphics::STexture::ETextureType::TextureCubeMap;
				TextureConfig4Chebyshev->Width = m_BakeResolution;
				TextureConfig4Chebyshev->Height = m_BakeResolution;
				TextureConfig4Chebyshev->Type4WrapR = GL_CLAMP_TO_EDGE;
				TextureConfig4Chebyshev->Type4WrapS = GL_CLAMP_TO_EDGE;
				TextureConfig4Chebyshev->Type4WrapT = GL_CLAMP_TO_EDGE;

				genTexture(TextureConfig4Chebyshev);
				genTexture(TextureConfig4Albedo);
				genTexture(TextureConfig4Normal);
				genTexture(TextureConfig4Position);

				auto TextureConfig4Depth = std::make_shared<ElayGraphics::STexture>();
				TextureConfig4Depth->InternalFormat = GL_RGBA32F;
				TextureConfig4Depth->ExternalFormat = GL_RGBA;
				TextureConfig4Depth->DataType = GL_FLOAT;
				TextureConfig4Depth->TextureType = ElayGraphics::STexture::ETextureType::DepthCubeMap;
				TextureConfig4Depth->TextureAttachmentType = ElayGraphics::STexture::ETextureAttachmentType::CubeDepthTexture;
				TextureConfig4Depth->Width = m_BakeResolution;
				TextureConfig4Depth->Height = m_BakeResolution;
				genTexture(TextureConfig4Depth);

				m_TextureConfig4Position.push_back(TextureConfig4Position);
				m_TextureConfig4Albedos.push_back(TextureConfig4Albedo);
				m_TextureConfig4Normals.push_back(TextureConfig4Normal);
				m_TextureConfig4Depths.push_back(TextureConfig4Depth);
				m_TextureConfig4Chebyshevs.push_back(TextureConfig4Chebyshev);
				auto FBO = genFBO({ TextureConfig4Position, TextureConfig4Albedo, TextureConfig4Normal,TextureConfig4Chebyshev, TextureConfig4Depth });

				m_FBOs.push_back(FBO);
			}
		}
	}



	int Index = 0;
	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	{
		for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
		{
			for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
			{				
				Index++;
			}
		}
	}


	TextureSamplePosition = std::make_shared<ElayGraphics::STexture>();
	TextureSampleAlbedo = std::make_shared<ElayGraphics::STexture>();
	TextureSampleNormal = std::make_shared<ElayGraphics::STexture>();
	TextureSampleChebyshev = std::make_shared<ElayGraphics::STexture>();

	TextureSamplePosition->InternalFormat = GL_RGBA32F;
	TextureSamplePosition->ExternalFormat = GL_RGBA;
	TextureSamplePosition->DataType = GL_FLOAT;
	TextureSamplePosition->TextureType = ElayGraphics::STexture::ETextureType::Texture2D;
	TextureSamplePosition->Width = mSampleNum;
	TextureSamplePosition->Height = Index;
	TextureSamplePosition->BorderColor = std::vector<float>{ 0,0,0 };
	TextureSamplePosition->Type4WrapR = GL_CLAMP_TO_EDGE;
	TextureSamplePosition->Type4WrapS = GL_CLAMP_TO_EDGE;
	TextureSamplePosition->Type4WrapT = GL_CLAMP_TO_EDGE;

	TextureSampleAlbedo->InternalFormat = TextureSampleNormal->InternalFormat = GL_RGBA32F;
	TextureSampleAlbedo->ExternalFormat = TextureSampleNormal->ExternalFormat = GL_RGBA;
	TextureSampleAlbedo->DataType = TextureSampleNormal->DataType = GL_FLOAT;
	TextureSampleAlbedo->TextureType = TextureSampleNormal->TextureType = ElayGraphics::STexture::ETextureType::Texture2D;
	TextureSampleAlbedo->Width = TextureSampleNormal->Width = mSampleNum;
	TextureSampleAlbedo->Height = TextureSampleNormal->Height = Index;
	TextureSampleAlbedo->Type4WrapR = TextureSampleNormal->Type4WrapR = GL_CLAMP_TO_EDGE;
	TextureSampleAlbedo->Type4WrapS = TextureSampleNormal->Type4WrapS = GL_CLAMP_TO_EDGE;
	TextureSampleAlbedo->Type4WrapT = TextureSampleNormal->Type4WrapT = GL_CLAMP_TO_EDGE;

	TextureSampleChebyshev->InternalFormat = GL_RGB32F;
	TextureSampleChebyshev->ExternalFormat = GL_RGB;
	TextureSampleChebyshev->DataType = GL_FLOAT;
	TextureSampleChebyshev->TextureType = ElayGraphics::STexture::ETextureType::Texture2D;
	TextureSampleChebyshev->Width = mSampleNum;
	TextureSampleChebyshev->Height = Index;
	TextureSampleChebyshev->Type4WrapR = GL_CLAMP_TO_EDGE;
	TextureSampleChebyshev->Type4WrapS = GL_CLAMP_TO_EDGE;
	TextureSampleChebyshev->Type4WrapT = GL_CLAMP_TO_EDGE;

	genTexture(TextureSamplePosition);
	genTexture(TextureSampleAlbedo);
	genTexture(TextureSampleNormal);
	genTexture(TextureSampleChebyshev);
	

	ElayGraphics::ResourceManager::registerSharedData("BakeResolution", m_BakeResolution);
	ElayGraphics::ResourceManager::registerSharedData("step_probe", step_probe);
	ElayGraphics::ResourceManager::registerSharedData("TextureSamplePosition", TextureSamplePosition);
	ElayGraphics::ResourceManager::registerSharedData("TextureSampleAlbedo", TextureSampleAlbedo);
	ElayGraphics::ResourceManager::registerSharedData("TextureSampleNormal", TextureSampleNormal);
	ElayGraphics::ResourceManager::registerSharedData("TextureSampleChebyshev", TextureSampleChebyshev);

	m_pShader->activeShader();
	m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(m_pSponza->getModelMatrix()));
	m_pShader->setFloatUniformValue("u_Near", ElayGraphics::Camera::getMainCameraNear());
	m_pShader->setFloatUniformValue("u_Far", ElayGraphics::Camera::getMainCameraFar());
	m_pSponza->initModel(*m_pShader);
}

void CLightProbePass::updateV()
{
	int Index = 0;


	//先做烘焙
	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	{
		for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
		{
			for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
			{				
				glBindFramebuffer(GL_FRAMEBUFFER, m_FBOs[Index]);
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glEnable(GL_DEPTH_TEST);
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				glViewport(0, 0, m_BakeResolution, m_BakeResolution);
				glm::vec3 ViewPos = glm::vec3(i, j, k);
				for (int i = 0; i < 6; i++)
				{
					glm::vec3 ViewDir = m_BakeDir[i];
					if (abs(ViewDir) == m_LightUpVector)
						ViewDir.z += 0.01f;
					glm::mat4x4 ViewMatrix = glm::lookAt(ViewPos, ViewPos + ViewDir, m_LightUpVector);
					glm::mat4x4 ProjectionMatrix = glm::perspective(m_Fovy, m_Aspect, m_Near, m_Far);

					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_TextureConfig4Position[Index]->TextureID, 0);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_TextureConfig4Albedos[Index]->TextureID, 0);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_TextureConfig4Normals[Index]->TextureID, 0);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_TextureConfig4Chebyshevs[Index]->TextureID, 0);
					glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_TextureConfig4Depths[Index]->TextureID, 0);
					m_pShader->activeShader();
					m_pShader->setMat4UniformValue("u_BakeViewMatrix", glm::value_ptr(ViewMatrix));
					m_pShader->setMat4UniformValue("u_BakeProjectionMatrix", glm::value_ptr(ProjectionMatrix));
					m_pSponza->updateModel(*m_pShader);
				}
				Index++;
			}
		}
	}
	

	glFlush();
	glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	glm::vec3 _coefficientVoxelSize(0);
	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	{
		_coefficientVoxelSize.x++;
	}

	for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
	{		
		_coefficientVoxelSize.y++;
	}

	for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
	{
		_coefficientVoxelSize.z++;
	}


	//发射光线采样到纹理
	auto surfelSampleCS = std::make_shared<CShader>("SurfelSample_CS.glsl");
	glm::vec3 probeIndex3(0, 0, 0);
	Index = 0;
	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	{
		probeIndex3.y = 0;
		for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
		{
			probeIndex3.z = 0;
			for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
			{

				surfelSampleCS->activeShader();

				glm::vec3 _probePos = glm::vec3(i , j , k );
				surfelSampleCS->setFloatUniformValue("_probePos", _probePos[0], _probePos[1], _probePos[2], 1);
				float _randSeed = rand() / (float)(RAND_MAX);
				surfelSampleCS->setFloatUniformValue("_randSeed", _randSeed);
				surfelSampleCS->setFloatUniformValue("_probeIndex3", probeIndex3.x, probeIndex3.y, probeIndex3.z);
				surfelSampleCS->setFloatUniformValue("_coefficientVoxelSize", _coefficientVoxelSize.x, _coefficientVoxelSize.y, _coefficientVoxelSize.z, 1.0);

				surfelSampleCS->setTextureUniformValue("_worldPosCubemap", m_TextureConfig4Position[Index]);
				surfelSampleCS->setTextureUniformValue("_normalCubemap", m_TextureConfig4Normals[Index]);
				surfelSampleCS->setTextureUniformValue("_albedoCubemap", m_TextureConfig4Albedos[Index]);
				surfelSampleCS->setTextureUniformValue("_chebyshevsCubemap", m_TextureConfig4Chebyshevs[Index]);

				glBindImageTexture(0, TextureSamplePosition->TextureID, 0, GL_FALSE, 0, GL_READ_WRITE, TextureSamplePosition->InternalFormat);
				glBindImageTexture(1, TextureSampleAlbedo->TextureID, 0, GL_FALSE, 0, GL_READ_WRITE, TextureSampleAlbedo->InternalFormat);
				glBindImageTexture(2, TextureSampleNormal->TextureID, 0, GL_FALSE, 0, GL_READ_WRITE, TextureSampleNormal->InternalFormat);
				glBindImageTexture(3, TextureSampleChebyshev->TextureID, 0, GL_FALSE, 0, GL_READ_WRITE, TextureSampleChebyshev->InternalFormat);

				glDispatchCompute(1, 1, 1);
				//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
				glFlush();

				probeIndex3.z++;
				Index++;
			}
			probeIndex3.y++;
		}
		probeIndex3.x++;
	}

	for (int i = 0; i < m_TextureConfig4Position.size(); i++)
	{
		GLuint tId = (m_TextureConfig4Position[i]->TextureID);
		glDeleteTextures(1, &tId);

		tId = (m_TextureConfig4Albedos[i]->TextureID);
		glDeleteTextures(1, &tId);

		tId = (m_TextureConfig4Normals[i]->TextureID);
		glDeleteTextures(1, &tId);

		tId = (m_TextureConfig4Depths[i]->TextureID);
		glDeleteTextures(1, &tId);

		tId = (m_TextureConfig4Chebyshevs[i]->TextureID);
		glDeleteTextures(1, &tId);
	}


}