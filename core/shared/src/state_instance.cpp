// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"
#include <material_manager2.hpp>

Engine::StateInstance::StateInstance(const std::shared_ptr<msys::MaterialManager> &matManager, Material *matErr) : materialManager {matManager}, state(nullptr)
{
	if(matErr == nullptr) {
		Con::cout << "Unable to load error material! Validate or reinstall the game files!" << Con::endl;
		exit(EXIT_FAILURE);
	}
	materialManager->SetErrorMaterial(matErr);
}

Engine::StateInstance::~StateInstance()
{
	state = nullptr; // Has to be cleared before the material manager!
	materialManager = nullptr;
}
