/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/c_flashlight.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/components/c_attachment_component.hpp"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/components/parent_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(flashlight, CFlashlight);

extern DLLCLIENT ClientState *client;

void CFlashlightComponent::Initialize()
{
	BaseFlashlightComponent::Initialize();
	client->PrecacheSound("fx\\flashlight_on.wav");
	client->PrecacheSound("fx\\flashlight_off.wav");

	auto &ent = GetEntity();
	BindEvent(CLightComponent::EVENT_SHOULD_PASS_ENTITY, [this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto pAttComponent = GetEntity().GetComponent<CAttachmentComponent>();
		auto *pParent = pAttComponent.valid() ? pAttComponent->GetParent() : nullptr;
		if(pParent != nullptr && &static_cast<CEShouldPassEntity &>(evData.get()).entity == pParent) {
			static_cast<CEShouldPassEntity &>(evData.get()).shouldPass = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
}

util::EventReply CFlashlightComponent::HandleEvent(ComponentEventId eventId, ComponentEvent &evData)
{
	if(BaseFlashlightComponent::HandleEvent(eventId, evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	auto &ent = GetEntity();
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_ON) {
		auto pSoundEmitterCompnent = ent.GetComponent<pragma::CSoundEmitterComponent>();
		if(pSoundEmitterCompnent.valid())
			pSoundEmitterCompnent->EmitSound("fx\\flashlight_on.wav", ALSoundType::Effect, 1.f);
	}
	else if(eventId == BaseToggleComponent::EVENT_ON_TURN_OFF) {
		auto pSoundEmitterCompnent = ent.GetComponent<pragma::CSoundEmitterComponent>();
		if(pSoundEmitterCompnent.valid())
			pSoundEmitterCompnent->EmitSound("fx\\flashlight_off.wav", ALSoundType::Effect, 1.f);
	}
	return util::EventReply::Unhandled;
}
void CFlashlightComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CFlashlight::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFlashlightComponent>();
}
