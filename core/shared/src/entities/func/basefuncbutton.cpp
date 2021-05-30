/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/func/basefuncbutton.h"
#include "pragma/util/util_handled.hpp"
#include "pragma/audio/alsound_type.h"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_sound_emitter_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/usable_component.hpp"
#include "pragma/entities/baseentity_events.hpp"
#include "pragma/entities/entity_component_system_t.hpp"

using namespace pragma;

void BaseFuncButtonComponent::Initialize()
{
	BaseEntityComponent::Initialize();

	BindEvent(BaseEntity::EVENT_HANDLE_KEY_VALUE,[this](std::reference_wrapper<pragma::ComponentEvent> evData) -> util::EventReply {
		auto &kvData = static_cast<CEKeyValueData&>(evData.get());
		if(ustring::compare(kvData.key,"use_sound",false))
			m_kvUseSound = kvData.value;
		else if(ustring::compare(kvData.key,"wait",false))
			m_kvWaitTime = util::to_float(kvData.value);
		else
			return util::EventReply::Unhandled;
		return util::EventReply::Handled;
	});

	auto &ent = GetEntity();
	ent.AddComponent("physics");
	auto whRenderComponent = ent.AddComponent("render");
	if(whRenderComponent.valid())
		static_cast<BaseRenderComponent*>(whRenderComponent.get())->SetCastShadows(true);
	ent.AddComponent("io");
	ent.AddComponent("model");
	ent.AddComponent("sound_emitter");
	ent.AddComponent<pragma::UsableComponent>();
}

util::EventReply BaseFuncButtonComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseEntityComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	if(eventId == UsableComponent::EVENT_ON_USE)
	{
		if(m_useSound != nullptr)
			m_useSound->Play();
		auto &ent = GetEntity();
		if(m_kvWaitTime > 0.f)
			m_tNextUse = CFloat(ent.GetNetworkState()->GetGameState()->CurTime()) +m_kvWaitTime;
		auto *ioComponent = static_cast<pragma::BaseIOComponent*>(GetEntity().FindComponent("io").get());
		if(ioComponent != nullptr)
			ioComponent->TriggerOutput("OnPressed",static_cast<const CEOnUseData&>(evData).entity);
	}
	return util::EventReply::Unhandled;
}

void BaseFuncButtonComponent::OnEntitySpawn()
{
	BaseEntityComponent::OnEntitySpawn();
	auto &ent = GetEntity();
	auto pPhysComponent = ent.GetPhysicsComponent();
	if(pPhysComponent != nullptr)
		pPhysComponent->InitializePhysics(PHYSICSTYPE::STATIC);
	if(!m_kvUseSound.empty())
	{
		ent.GetNetworkState()->PrecacheSound(m_kvUseSound);
		m_useSound = nullptr;
		auto pSoundEmitterComponent = static_cast<pragma::BaseSoundEmitterComponent*>(ent.FindComponent("sound_emitter").get());
		if(pSoundEmitterComponent != nullptr)
			m_useSound = pSoundEmitterComponent->CreateSound(m_kvUseSound,ALSoundType::Effect);
	}
}
