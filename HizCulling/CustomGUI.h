#pragma once

#include "GUI.h"
#include <GLM/glm.hpp>
#include <vector>
class CCustomGUI : public IGUI
{
public:
    CCustomGUI(const std::string& vName, int vExcutionOrder);
    virtual ~CCustomGUI();

    virtual void initV() override;
    virtual void updateV() override;
private:
    bool useHiz = true;
    bool useTemporal = true;
};