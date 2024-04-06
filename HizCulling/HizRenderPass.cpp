#include "HizRenderPass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
//#include "ResourceManager.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
HizRenderPass::HizRenderPass(const std::string& vPassName, int vExcutionOrder):IRenderPass(vPassName, vExcutionOrder)
{
}

HizRenderPass::~HizRenderPass()
{
}

static const float  globalscale = 0.1;
inline float frand()
{
    return float(rand() % RAND_MAX) / float(RAND_MAX);
}

void HizRenderPass::initV()
{
	m_pShader = std::make_shared<CShader>("RenderCube_VS.glsl", "RenderCube_FS.glsl");
    downSampleShader = std::make_shared<CShader>("DownSample_VS.glsl", "DownSample_FS.glsl");
    visibleShader = std::make_shared<CShader>("HizVisible_CS.glsl");
    screenShader = std::make_shared<CShader>("ScreenRender_VS.glsl", "ScreenRender_FS.glsl");

    
    auto TextureConfigColor = std::make_shared<ElayGraphics::STexture>();
    genTexture(TextureConfigColor);
    auto TextureConfigDepth = std::make_shared<ElayGraphics::STexture>();
    TextureConfigDepth->InternalFormat = GL_DEPTH_COMPONENT32F;
    TextureConfigDepth->ExternalFormat = GL_DEPTH_COMPONENT;
    TextureConfigDepth->DataType = GL_FLOAT;
    TextureConfigDepth->Type4MinFilter = GL_LINEAR;
    TextureConfigDepth->Type4MagFilter = GL_LINEAR;
    //TextureConfig4Depth->Width = TextureConfig4Depth->Height = m_ShadowmapResolution;
    TextureConfigDepth->Type4WrapS = TextureConfigDepth->Type4WrapT = GL_CLAMP_TO_BORDER;
    TextureConfigDepth->BorderColor = { 0,0,0,0 };
    TextureConfigDepth->TextureAttachmentType = ElayGraphics::STexture::ETextureAttachmentType::DepthTexture;
    TextureConfigDepth->isMipmap = true;
    genTexture(TextureConfigDepth);
    genGenerateMipmap(TextureConfigDepth);
    ElayGraphics::ResourceManager::registerSharedData("TextureConfigDepth", TextureConfigDepth);
    m_FBO = genFBO({ TextureConfigColor, TextureConfigDepth });


    //auto DownSampleDepth = std::make_shared<ElayGraphics::STexture>();
    //DownSampleDepth->InternalFormat = GL_DEPTH_COMPONENT32F;
    //DownSampleDepth->ExternalFormat = GL_DEPTH_COMPONENT;
    //DownSampleDepth->DataType = GL_FLOAT;
    //DownSampleDepth->Type4MinFilter = GL_LINEAR;
    //DownSampleDepth->Type4MagFilter = GL_LINEAR;
    ////TextureConfig4Depth->Width = TextureConfig4Depth->Height = m_ShadowmapResolution;
    //DownSampleDepth->Type4WrapS = TerrianDepth->Type4WrapT = GL_CLAMP_TO_BORDER;
    //DownSampleDepth->BorderColor = { 0,0,0,0 };
    //DownSampleDepth->TextureAttachmentType = ElayGraphics::STexture::ETextureAttachmentType::DepthTexture;
    //DownSampleDepth->isMipmap = true;
    //genTexture(DownSampleDepth);
    //genGenerateMipmap(DownSampleDepth);
    //ElayGraphics::ResourceManager::registerSharedData("DownSampleDepth", DownSampleDepth);

    downSampleShader->activeShader();
    downSampleShader->setTextureUniformValue("depthTex", TextureConfigDepth);

    visibleShader->activeShader();
    visibleShader->setTextureUniformValue("depthTex", TextureConfigDepth);

    screenShader->activeShader();
    screenShader->setTextureUniformValue("u_Texture2D", TextureConfigColor);

    int obj = 0;
    
    int grid = 32;
    for (int i = 0; i < grid * grid * grid; i++)
    {
        CullBbox bbox;
        bbox.min = glm::vec4(-1, -1, -1, 1);
        bbox.max = glm::vec4(1, 1, 1, 1);
        glm::vec3 pos(i % grid, (i / grid) % grid, i / (grid * grid));
        pos -= glm::vec3(grid / 2, grid / 2, grid / 2);
        pos += (glm::vec3(frand(), frand(), frand()) * 2.0f) - glm::vec3(1.0f);
        pos /= float(grid);
        float scale;
        if (glm::length(pos) < 0.52f)
        {
            scale = globalscale * 0.35f;
            pos *= globalscale * 0.5f;
        }
        else
        {
            scale = globalscale;
            pos *= globalscale;
        }

        glm::mat4 matrix = glm::translate(glm::mat4(1.f), pos) 
        *glm::rotate(glm::mat4(1), frand() * glm::pi<float>(), glm::vec3(0, 1, 0))
            * glm::scale(glm::mat4(1.f), (glm::vec3(scale) * (glm::vec3(0.25f) + glm::vec3(frand(), frand(), frand()) * 0.5f)) / float(grid));

        modelMatrixs.push_back(matrix);
        //modelMatrixs.push_back(glm::transpose(glm::inverse(matrix)));

        // all have same bbox

        glm::vec4 tempmin = matrix * bbox.min;
        glm::vec4 tempmax = matrix * bbox.max;


        bbox.min = glm::min(tempmin, tempmax);
        bbox.max = glm::max(tempmin, tempmax);

        bboxes.push_back(bbox);
        obj++;
    }


    cubeVisible.resize(obj, 1);
    visibleBuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, cubeVisible.size() * sizeof(int), cubeVisible.data(), GL_STATIC_DRAW, 0);
    bbxoBuffer = genBuffer(GL_SHADER_STORAGE_BUFFER, bboxes.size() * sizeof(CullBbox), bboxes.data(), GL_STATIC_DRAW, 1);

}

std::vector<int> HizRenderPass::_calculateVisibleCube()
{

    std::shared_ptr<ElayGraphics::STexture> TextureConfigDepth = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("TextureConfigDepth");
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthFunc(GL_ALWAYS);
    glm::vec2 srcResolution = glm::vec2(ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
    
    int level = 0;
    int dim = srcResolution.x > srcResolution.y ? srcResolution.x : srcResolution.y;
    int twidth = srcResolution.x;
    int theight = srcResolution.y;
    int wasEven = 0;
    
    while (dim)
    {
        if (level)
        {
            twidth = twidth < 1 ? 1 : twidth;
            theight = theight < 1 ? 1 : theight;
            glViewport(0, 0, twidth, theight);
            GLuint temp_FBO;
            glGenFramebuffers(1, &(GLuint&)temp_FBO);
            glBindFramebuffer(GL_FRAMEBUFFER, temp_FBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TextureConfigDepth->TextureID, level);
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                std::cerr << "Error::FBO:: Framebuffer Is Not Complete." << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
            }
            downSampleShader->activeShader();
            downSampleShader->setIntUniformValue("depthLod", level-1);
            downSampleShader->setIntUniformValue("evenLod", wasEven);
            drawQuad();
            glFlush();
            glDeleteFramebuffers(1, &temp_FBO);
        }

        wasEven = (twidth % 2 == 0) && (theight % 2 == 0);

        dim /= 2;
        twidth /= 2;
        theight /= 2;
        level++;
    }

    //for (int i = 1; i < 11; i++)
    //{
    //    int viewportResolutionX = srcResolution.x * 0.5;
    //    int viewportResolutionY = srcResolution.y * 0.5;
    //    viewportResolutionX = viewportResolutionX > 0 ? viewportResolutionX : 1;
    //    viewportResolutionY = viewportResolutionY > 0 ? viewportResolutionY : 1;
    //
    //    glViewport(0, 0, viewportResolutionX, viewportResolutionY);
    //    GLuint temp_FBO;
    //    glGenFramebuffers(1, &(GLuint&)temp_FBO);
    //    glBindFramebuffer(GL_FRAMEBUFFER, temp_FBO);
    //    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TextureConfigDepth->TextureID, i);
    //    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    //    {
    //        std::cerr << "Error::FBO:: Framebuffer Is Not Complete." << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
    //    }
    //    downSampleShader->activeShader();
    //    downSampleShader->setIntUniformValue("depthLod", i);
    //    downSampleShader->setFloatUniformValue("srcResolution", srcResolution.x, srcResolution.y);
    //
    //    drawQuad();
    //    glFlush();
    //    srcResolution *= 0.5;
    //    srcResolution[0] = srcResolution[0] > 0 ? srcResolution[0] : 1;
    //    srcResolution[1] = srcResolution[1] > 0 ? srcResolution[1] : 1;
    //    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //    glDeleteFramebuffers(1, &temp_FBO);
    //    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //}
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
    glDepthFunc(GL_LESS);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    visibleShader->activeShader();
    glDispatchCompute(1, 1, bboxes.size() / 64 + 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glFlush();


    std::vector<int> currentVisible(cubeVisible.size());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, visibleBuffer);
    GLvoid* p = (GLvoid*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
    memcpy(&currentVisible[0], p, currentVisible.size() * sizeof(int));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    
    return currentVisible;
}



void HizRenderPass::updateV()
{


    static int currentFrame = 0;
    std::shared_ptr<ElayGraphics::STexture> TextureConfigDepth = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("TextureConfigDepth");
    bool useHiz = ElayGraphics::ResourceManager::getSharedDataByName<bool>("useHiz");
    bool temporal = ElayGraphics::ResourceManager::getSharedDataByName<bool>("useTemporal");
    //temporal = false;
    if (useHiz)
    {
        //拿上一帧的算出需要显示的cube。
        std::vector<int> currentVisible(cubeVisible.size(), 1);
        if (currentFrame != 0)
        {
            currentVisible = _calculateVisibleCube();
        }

        //渲染
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_pShader->activeShader();
        for (int i = 0; i < modelMatrixs.size(); i++)
        {
            if (currentVisible[i])
            {
                m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(modelMatrixs[i]));
                drawCube();
            }
        }

        //再拿这帧渲染完的深度图再去做一遍计算
        if (temporal)
        {
            std::vector<int> nextVisible = _calculateVisibleCube();
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glDepthFunc(GL_LESS);
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
            glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
            m_pShader->activeShader();
            for (int i = 0; i < modelMatrixs.size(); i++)
            {
                //渲染第一次没有通过的，但是第二次通过了的
                if (currentVisible[i] == 0 && nextVisible[i] == 1)
                {
                    m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(modelMatrixs[i]));
                    drawCube();
                }
            }
            int visinum = 0;
            for (int i = 0; i < cubeVisible.size(); i++)
            {
                cubeVisible[i] = 0;
                if (currentVisible[i] == 1 || nextVisible[i] == 1)
                {
                    visinum++;
                    cubeVisible[i] = 1;
                }
            }

            std::cout << "visible_num: " << visinum << "," << "visible_rate: " << (float)visinum / cubeVisible.size() << std::endl;
        }
        else
        {
            int visinum = 0;
            for (int i = 0; i < cubeVisible.size(); i++)
            {
                cubeVisible[i] = 0;
                if (currentVisible[i] == 1)
                {
                    visinum++;
                    cubeVisible[i] = 1;
                }
            }
            std::cout << "visible_num: " << visinum << "," << "visible_rate: " << (float)visinum / cubeVisible.size() << std::endl;

        }
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_pShader->activeShader();
        for (int i = 0; i < modelMatrixs.size(); i++)
        {        
            m_pShader->setMat4UniformValue("u_ModelMatrix", glm::value_ptr(modelMatrixs[i]));
            drawCube();          
        }
    }
    //最后渲染屏幕空间
    screenShader->activeShader();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawQuad();


    currentFrame++;
}

