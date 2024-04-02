#pragma once
#include "GameObject.h"

class CModelLoad : public IGameObject
{
public:
	CModelLoad(const std::string& vGameObjectName, int vExecutionOrder);
	virtual ~CModelLoad();

	virtual void initV() override;
	virtual void updateV() override;
};