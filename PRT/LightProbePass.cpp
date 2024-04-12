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

				TextureConfig4Position->InternalFormat = TextureConfig4Normal->InternalFormat = GL_RGB32F;
				TextureConfig4Position->ExternalFormat = TextureConfig4Normal->ExternalFormat = GL_RGB;
				TextureConfig4Position->DataType = TextureConfig4Normal->DataType = GL_FLOAT;
				TextureConfig4Position->TextureType = TextureConfig4Normal->TextureType = ElayGraphics::STexture::ETextureType::TextureCubeMap;
				TextureConfig4Position->Width = TextureConfig4Normal->Width = m_BakeResolution;
				TextureConfig4Position->Height = TextureConfig4Normal->Height = m_BakeResolution;


				TextureConfig4Albedo->InternalFormat = TextureConfig4Normal->InternalFormat = GL_RGBA32F;
				TextureConfig4Albedo->ExternalFormat = TextureConfig4Normal->ExternalFormat = GL_RGBA;
				TextureConfig4Albedo->DataType = TextureConfig4Normal->DataType = GL_FLOAT;
				TextureConfig4Albedo->TextureType = TextureConfig4Normal->TextureType = ElayGraphics::STexture::ETextureType::TextureCubeMap;
				TextureConfig4Albedo->Width = TextureConfig4Normal->Width = m_BakeResolution;
				TextureConfig4Albedo->Height = TextureConfig4Normal->Height = m_BakeResolution;

				TextureConfig4Chebyshev->InternalFormat = GL_RGB32F;
				TextureConfig4Chebyshev->ExternalFormat = GL_RGB;
				TextureConfig4Chebyshev->DataType = GL_FLOAT;
				TextureConfig4Chebyshev->TextureType = ElayGraphics::STexture::ETextureType::TextureCubeMap;
				TextureConfig4Chebyshev->Width = m_BakeResolution;
				TextureConfig4Chebyshev->Height = m_BakeResolution;


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

	surfelDatas = std::make_shared<SurfelData>();
	surfelDatas->all_surfelData.resize(m_FBOs.size());
	for (int i = 0; i < m_FBOs.size(); i++)
	{
		surfelDatas->all_surfelData[i].resize(32 * 16);
	}

	std::vector<Surfel> surfelData(32 * 16);
	surfelBuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, surfelData.size() * sizeof(Surfel), &surfelData[0], GL_STATIC_DRAW, 0);

	ElayGraphics::ResourceManager::registerSharedData("SurfelBuffer", surfelBuffer);
	ElayGraphics::ResourceManager::registerSharedData("SurfelDatas", surfelDatas);
	ElayGraphics::ResourceManager::registerSharedData("BakeResolution", m_BakeResolution);

	ElayGraphics::ResourceManager::registerSharedData("BakePositionTextures", m_TextureConfig4Position);
	ElayGraphics::ResourceManager::registerSharedData("BakeAlbedoTextures", m_TextureConfig4Albedos);
	ElayGraphics::ResourceManager::registerSharedData("BakeNormalTextures", m_TextureConfig4Normals);
	ElayGraphics::ResourceManager::registerSharedData("BakeChebyshevsTextures", m_TextureConfig4Chebyshevs);
	ElayGraphics::ResourceManager::registerSharedData("step_probe", step_probe);

	m_pShader->activeShader();
	m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(m_pSponza->getModelMatrix()));
	m_pShader->setFloatUniformValue("u_Near", ElayGraphics::Camera::getMainCameraNear());
	m_pShader->setFloatUniformValue("u_Far", ElayGraphics::Camera::getMainCameraFar());
	m_pSponza->initModel(*m_pShader);
}

void CLightProbePass::updateV()
{

	int Index = 0;
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

	
	auto surfelSampleCS = std::make_shared<CShader>("SurfelSample_CS.glsl");
	Index = 0;
	for (int i = m_MinAABB.x; i < m_MaxAABB.x; i += step_probe)
	{
		for (int j = m_MinAABB.y; j < m_MaxAABB.y; j += step_probe)
		{
			for (int k = m_MinAABB.z; k < m_MaxAABB.z; k += step_probe)
			{
				surfelSampleCS->activeShader();
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, surfelBuffer);

				glm::vec3 _probePos = glm::vec3((i + step_probe) * 0.5, (j + step_probe) * 0.5, (k + step_probe) * 0.5);
				surfelSampleCS->setFloatUniformValue("_probePos", _probePos[0], _probePos[1], _probePos[2], 1);

				float _randSeed = rand() / (float)(RAND_MAX);
				surfelSampleCS->setFloatUniformValue("_randSeed", _randSeed);

				surfelSampleCS->setTextureUniformValue("_worldPosCubemap", m_TextureConfig4Position[Index]);
				surfelSampleCS->setTextureUniformValue("_normalCubemap", m_TextureConfig4Normals[Index]);
				surfelSampleCS->setTextureUniformValue("_albedoCubemap", m_TextureConfig4Albedos[Index]);
				glDispatchCompute(1, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
				glFlush();

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, surfelBuffer);
				GLvoid* p = (GLvoid*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
				memcpy(&surfelDatas->all_surfelData[Index][0], p, 32*16 * sizeof(Surfel));
				glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
				Index ++;
			}
		}
	}

}