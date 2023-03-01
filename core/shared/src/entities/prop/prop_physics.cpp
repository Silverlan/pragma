/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/prop/prop_physics.hpp"
#include "pragma/entities/prop/prop_base.h"
#include "pragma/entities/components/base_model_component.hpp"
#include <pragma/physics/movetypes.h>

using namespace pragma;

void BasePropPhysicsComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEventUnhandled(BaseModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &ent = GetEntity();
		auto mdlComponent = ent.GetModelComponent();
		if(!mdlComponent || mdlComponent->HasModel() == false || !ent.IsSpawned())
			return util::EventReply::Unhandled;
		auto *pPropComponent = static_cast<pragma::BasePropComponent *>(ent.FindComponent("prop").get());
		if(pPropComponent != nullptr)
			pPropComponent->UpdatePhysicsType(&ent);
		return util::EventReply::Unhandled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("prop");
}

void BasePropPhysicsComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto *pPropComponent = static_cast<pragma::BasePropComponent *>(GetEntity().FindComponent("prop").get());
	auto physType = PHYSICSTYPE::NONE;
	if(pPropComponent == nullptr)
		return;
	physType = pPropComponent->UpdatePhysicsType(&GetEntity());
	pPropComponent->Setup(physType, MOVETYPE::PHYSICS);
}
