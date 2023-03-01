/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"
#include <material_manager2.hpp>

extern DLLNETWORK Engine *engine;

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
