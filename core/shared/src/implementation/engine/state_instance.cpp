// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <material_manager2.hpp>

module pragma.shared;

import :version;

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
