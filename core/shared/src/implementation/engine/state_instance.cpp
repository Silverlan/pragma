// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cstdlib>

module pragma.shared;

import :engine;

pragma::Engine::StateInstance::StateInstance(const std::shared_ptr<material::MaterialManager> &matManager, material::Material *matErr) : materialManager {matManager}, state(nullptr)
{
	if(matErr == nullptr) {
		Con::COUT << "Unable to load error material! Validate or reinstall the game files!" << Con::endl;
		exit(EXIT_FAILURE);
	}
	materialManager->SetErrorMaterial(matErr);
}

pragma::Engine::StateInstance::~StateInstance()
{
	state = nullptr; // Has to be cleared before the material manager!
	materialManager = nullptr;
}
