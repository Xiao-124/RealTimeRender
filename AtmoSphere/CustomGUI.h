#pragma once

#include "GUI.h"
#include <GLM/glm.hpp>
#include <vector>
#include "AtmosphereParameter.h"
class CCustomGUI : public IGUI
{
public:
    CCustomGUI(const std::string& vName, int vExcutionOrder);
    virtual ~CCustomGUI();

    virtual void initV() override;
    virtual void updateV() override;
private:
    float uiCamHeight = 0.5f;
    AtmosphereInfo AtmosphereInfos;

};