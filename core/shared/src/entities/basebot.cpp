/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_shared.h"
#include "pragma/entities/basebot.h"
#include "pragma/entities/baseentity.h"
#include "pragma/entities/components/base_model_component.hpp"

using namespace pragma;

void BaseBotComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	auto &ent = GetEntity();
	auto *pMdlComponent = static_cast<BaseModelComponent*>(ent.AddComponent("model").get());
	if(pMdlComponent != nullptr)
		pMdlComponent->SetModel("player/soldier.wmd");
}
