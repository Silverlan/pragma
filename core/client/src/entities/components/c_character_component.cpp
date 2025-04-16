/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/c_engine.h"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/physics/collisiontypes.h"
#include "pragma/math/deltaoffset.h"
#include "pragma/math/deltatransform.h"
#include <mathutil/uquat.h>
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/rendering/c_rendermode.h"
#include <pragma/networking/nwm_util.h>
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/components/c_surface_component.hpp"
#include "pragma/entities/components/c_eye_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/submergible_component.hpp>
#include <pragma/entities/components/movement_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/model/model.h>
#include <pragma/audio/alsound_type.h>

extern DLLCLIENT ClientState *client;

using namespace pragma;

void CCharacterComponent::RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent) { BaseCharacterComponent::RegisterEvents(componentManager, registerEvent); }
CCharacterComponent::CCharacterComponent(BaseEntity &ent) : BaseCharacterComponent(ent) {}

void CCharacterComponent::Initialize()
{
	BaseCharacterComponent::Initialize();
	BindEventUnhandled(CAnimatedComponent::EVENT_ON_BLEND_ANIMATION_MT, [this](std::reference_wrapper<pragma::ComponentEvent> evData) {
		auto &ent = GetEntity();
		auto animComponent = ent.GetAnimatedComponent();
		auto &hMdl = ent.GetModel();
		if(hMdl == nullptr || animComponent.expired())
			return;
		// Apply movement blending
		auto &blendAnimInfo = static_cast<CEOnBlendAnimation &>(evData.get());
		if(&blendAnimInfo.slotInfo == &animComponent->GetBaseAnimationInfo()) // Only apply to main animation, not to gestures!
		{
			auto anim = hMdl->GetAnimation(animComponent->GetAnimation());
			auto charC = ent.GetCharacterComponent();
			auto *movementC = charC.valid() ? charC->GetMovementComponent() : nullptr;
			auto moveSpeed = movementC ? movementC->GetSpeed() : Vector2 {};
			// Animation movement blending does not work well with special movement animations (e.g. leaping),
			// so we exclude all non-looping movement animations here. (Also see BaseAIComponent::BlendAnimationMovement).
			// If the result isn't satisfactory, alternatively enable the check for the moveActivity above instead.
			// However, this requires sending the moveActivity from the server to the clients (snapshots?)
			if(anim != nullptr && anim->HasFlag(FAnim::Loop) && (moveSpeed.x > 0.f || moveSpeed.y > 0.f)) //IsMoving())
			{
				auto anim = hMdl->GetAnimation(hMdl->SelectFirstAnimation(animComponent->TranslateActivity(Activity::Idle)));
				auto frame = anim ? anim->GetFrame(0) : nullptr;
				if(frame != nullptr) {
					auto blendScale = movementC ? movementC->GetMovementBlendScale() : 0.f;
					auto &dstPoses = frame->GetBoneTransforms();
					auto &dstScales = frame->GetBoneScales();
					animComponent->BlendBonePoses(blendAnimInfo.slotInfo.bonePoses, !blendAnimInfo.slotInfo.boneScales.empty() ? &blendAnimInfo.slotInfo.boneScales : nullptr, dstPoses, &dstScales, blendAnimInfo.slotInfo.bonePoses,
					  !blendAnimInfo.slotInfo.boneScales.empty() ? &blendAnimInfo.slotInfo.boneScales : nullptr, *anim, blendScale);
				}
			}
		}
	});
	BindEventUnhandled(SubmergibleComponent::EVENT_ON_WATER_ENTERED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { CreateWaterSplash(); });
	BindEventUnhandled(SubmergibleComponent::EVENT_ON_WATER_EXITED, [this](std::reference_wrapper<pragma::ComponentEvent> evData) { CreateWaterSplash(); });
	GetEntity().AddComponent<CEyeComponent>();
}

void CCharacterComponent::CreateWaterSplash()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pSubmergibleComponent = ent.GetComponent<SubmergibleComponent>();
	auto pSoundEmitterComponent = ent.GetComponent<CSoundEmitterComponent>();
	auto *pWater = pSubmergibleComponent.valid() ? pSubmergibleComponent->GetWaterEntity() : nullptr;
	auto pSurfC = (pWater != nullptr) ? pWater->GetComponent<CSurfaceComponent>() : pragma::ComponentHandle<CSurfaceComponent> {};
	if(pSoundEmitterComponent.valid() && pTrComponent != nullptr && pSurfC.valid()) {
		auto pos = pTrComponent->GetPosition();
		pos = pSurfC->ProjectToSurface(pos);
		client->PlayWorldSound("fx.water_slosh", ALSoundType::Effect, pos);
		auto *pt = pragma::CParticleSystemComponent::Create("watersplash");
		if(pt != nullptr) {
			auto pTrComponent = pt->GetEntity().GetTransformComponent();
			if(pTrComponent != nullptr) {
				pTrComponent->SetPosition(pos);
				Vector3 n;
				float d;
				pSurfC->GetPlaneWs(n, d);
				auto up = uvec::create(n);
				uvec::normalize(&up);
				const auto rot = Quat {0.5f, -0.5f, -0.5f, -0.5f};
				pTrComponent->SetRotation(uquat::create_look_rotation(uvec::get_perpendicular(up), up) * rot);
			}
			pt->SetRemoveOnComplete(true);
			pt->Start();
		}
	}
}

void CCharacterComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CCharacterComponent::GetBaseTypeIndex(std::type_index &outTypeIndex) const { outTypeIndex = std::type_index(typeid(BaseCharacterComponent)); }
void CCharacterComponent::ReceiveData(NetPacket &packet)
{
	// Note: Change return value of ShouldTransmitNetData if data should be received
}
Bool CCharacterComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetFrozen) {
		auto b = packet->Read<bool>();
		SetFrozen(b);
	}
	else if(eventId == m_netEvSetActiveWeapon) {
		auto *ent = nwm::read_entity(packet);
		SetActiveWeapon(ent);
	}
	else if(eventId == m_netEvSetAmmoCount) {
		auto ammoType = packet->Read<uint32_t>();
		auto count = packet->Read<uint16_t>();
		SetAmmoCount(ammoType, count);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
