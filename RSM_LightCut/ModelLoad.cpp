#include "ModelLoad.h"
#include "Interface.h"

CModelLoad::CModelLoad(const std::string& vGameObjectName, int vExecutionOrder) : IGameObject(vGameObjectName, vExecutionOrder)
{
}

CModelLoad::~CModelLoad()
{
}

void CModelLoad::initV()
{
	setModel(ElayGraphics::ResourceManager::getOrCreateModel("../Model/sponza/sponza.obj"));
}

void CModelLoad::updateV()
{
}