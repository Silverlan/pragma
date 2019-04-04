#include "stdafx_client.h"
#include "pragma/entities/c_flashlight.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/audio/alsound_type.h>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(flashlight,CFlashlight);

extern DLLCLIENT ClientState *client;

void CFlashlightComponent::Initialize()
{
	BaseFlashlightComponent::Initialize();
	client->PrecacheSound("fx\\flashlight_on.wav");
	client->PrecacheSound("fx\\flashlight_off.wav");

	auto &ent = GetEntity();
	BindEvent(CLightComponent::EVENT_SHOULD_PASS_ENTITY,[this](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
		auto pAttComponent = GetEntity().GetComponent<CAttachableComponent>();
		auto *pParent = pAttComponent.valid() ? pAttComponent->GetParent() : nullptr;
		if(pParent != nullptr && &static_cast<CEShouldPassEntity&>(evData.get()).entity == &pParent->GetEntity())
		{
			static_cast<CEShouldPassEntity&>(evData.get()).shouldPass = false;
			return util::EventReply::Handled;
		}
		return util::EventReply::Unhandled;
	});
}

util::EventReply CFlashlightComponent::HandleEvent(ComponentEventId eventId,ComponentEvent &evData)
{
	if(BaseFlashlightComponent::HandleEvent(eventId,evData) == util::EventReply::Handled)
		return util::EventReply::Handled;
	auto &ent = GetEntity();
	if(eventId == BaseToggleComponent::EVENT_ON_TURN_ON)
	{
		auto pSoundEmitterCompnent = ent.GetComponent<pragma::CSoundEmitterComponent>();
		if(pSoundEmitterCompnent.valid())
			pSoundEmitterCompnent->EmitSound("fx\\flashlight_on.wav",ALSoundType::Effect,1.f);
	}
	else if(eventId == BaseToggleComponent::EVENT_ON_TURN_OFF)
	{
		auto pSoundEmitterCompnent = ent.GetComponent<pragma::CSoundEmitterComponent>();
		if(pSoundEmitterCompnent.valid())
			pSoundEmitterCompnent->EmitSound("fx\\flashlight_off.wav",ALSoundType::Effect,1.f);
	}
	return util::EventReply::Unhandled;
}
luabind::object CFlashlightComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CFlashlightComponentHandleWrapper>(l);}

void CFlashlight::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CFlashlightComponent>();
}
