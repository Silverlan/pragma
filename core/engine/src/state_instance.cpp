#include "stdafx_engine.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"

extern DLLENGINE Engine *engine;

Engine::StateInstance::StateInstance(const std::shared_ptr<MaterialManager> &matManager,Material *matErr)
	: materialManager{matManager},state(nullptr)
{
	if(matErr == nullptr)
	{
		Con::cout<<"Unable to load error material! Validate or reinstall the game files!"<<Con::endl;
		exit(EXIT_FAILURE);
	}
	materialManager->SetErrorMaterial(matErr);
}

Engine::StateInstance::~StateInstance()
{
	state = nullptr; // Has to be cleared before the material manager!
	materialManager = nullptr;
}