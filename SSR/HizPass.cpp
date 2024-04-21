#include "HizPass.h"
#include "Shader.h"
#include "Interface.h"
#include "Common.h"
#include "Utils.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>


CHizPass::CHizPass(const std::string& vPassName, int vExcutionOrder) :IRenderPass(vPassName, vExcutionOrder)
{
}

CHizPass::~CHizPass()
{
}

void CHizPass::initV()
{
    m_pShader = std::make_shared<CShader>("Hiz_VS.glsl", "Hiz_FS.glsl");
    
    std::shared_ptr<ElayGraphics::STexture> TextureConfigDepth = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("TextureConfig4Depth");
    m_pShader->activeShader();
    m_pShader->setTextureUniformValue("depthTex", TextureConfigDepth);


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
        }
        wasEven = (twidth % 2 == 0) && (theight % 2 == 0);
        dim /= 2;
        twidth /= 2;
        theight /= 2;
        level++;
    }
    ElayGraphics::ResourceManager::registerSharedData("maxLevel", level);

}

void CHizPass::updateV()
{

    std::shared_ptr<ElayGraphics::STexture> TextureConfigDepth = ElayGraphics::ResourceManager::getSharedDataByName<std::shared_ptr<ElayGraphics::STexture>>("TextureConfig4Depth");
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
            GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
            {
                                               
                std::cerr << "Error::FBO:: Framebuffer Is Not Complete." << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::endl;
            }
            m_pShader->activeShader();         
            m_pShader->setIntUniformValue("depthLod", level - 1);
            m_pShader->setIntUniformValue("evenLod", wasEven);
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
    glViewport(0, 0, ElayGraphics::WINDOW_KEYWORD::getWindowWidth(), ElayGraphics::WINDOW_KEYWORD::getWindowHeight());
    glDepthFunc(GL_LESS);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}
