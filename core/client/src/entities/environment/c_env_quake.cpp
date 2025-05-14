/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/environment/c_env_quake.h"
#include "pragma/entities/c_entityfactories.h"
#include "noise/noise.h"
#include "pragma/entities/components/c_player_component.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(env_quake, CEnvQuake);

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

CQuakeComponent::~CQuakeComponent() { CancelScreenShake(); }
void CQuakeComponent::ReceiveData(NetPacket &packet)
{
	m_quakeFlags = packet->Read<UInt32>();
	SetFrequency(packet->Read<Float>());
	SetAmplitude(packet->Read<Float>());
	SetRadius(packet->Read<Float>());
	SetDuration(packet->Read<Float>());
	SetFadeInDuration(packet->Read<Float>());
	SetFadeOutDuration(packet->Read<Float>());
}
void CQuakeComponent::StartShake()
{
	if(IsShakeActive() == true)
		return;
	BaseEnvQuakeComponent::StartShake();
	if(ShouldShakeView() == false)
		return;
	auto perlin = std::make_shared<noise::module::Perlin>();
	perlin->SetSeed(CInt32(c_engine->GetTickCount()));
	m_tStartShake = CFloat(c_game->CurTime());
	m_cbScreenShake
	  = c_game->AddCallback("CalcView", FunctionCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>, std::reference_wrapper<Quat>>::Create([this, perlin](std::reference_wrapper<Vector3> refPos, std::reference_wrapper<Quat>, std::reference_wrapper<Quat>) {
		    auto &pos = refPos.get();
		    //auto &rot = refRot.get();

		    auto &entThis = GetEntity();
		    auto pTrComponent = entThis.GetTransformComponent();
		    if(pTrComponent == nullptr)
			    return;
		    auto &origin = pTrComponent->GetPosition();
		    auto &radius = m_radius;
		    auto &duration = m_duration;
		    auto &tFadeIn = m_tFadeIn;
		    auto &tFadeOut = m_tFadeOut;
		    auto frequency = m_frequency;
		    auto amplitude = m_amplitude;

		    auto &t = c_game->CurTime();
		    auto tDelta = t - m_tStartShake;
		    if(tDelta >= duration) {
			    if(ShouldRemoveOnComplete())
				    entThis.RemoveSafely();
			    m_cbScreenShake.Remove();
			    return;
		    }
		    auto bGlobal = IsGlobal();
		    auto *pl = c_game->GetLocalPlayer();
		    if(pl == nullptr || (radius == 0.f && bGlobal == false))
			    return;
		    auto &ent = pl->GetEntity();
		    auto pPhysComponent = ent.GetPhysicsComponent();
		    auto pTrComponentEnt = ent.GetTransformComponent();
		    if((InAir() == false && (pPhysComponent == nullptr || pPhysComponent->IsOnGround() == false)) || !pTrComponentEnt)
			    return;
		    auto dist = uvec::distance(origin, pos); //pTrComponentEnt->GetPosition());
		    auto scale = (bGlobal == true) ? 1.f : (1.f - umath::clamp(dist / radius, 0.f, 1.f));
		    scale *= util::get_faded_time_factor(CFloat(tDelta), duration, tFadeIn, tFadeOut);
		    frequency *= CFloat(scale);
		    amplitude *= CFloat(scale);
		    perlin->SetFrequency(frequency);
		    pos.x += CFloat(perlin->GetValue(t, 0.0, 0.0)) * amplitude;
		    pos.y += CFloat(perlin->GetValue(0.0, t, 0.0)) * amplitude;
		    pos.z += CFloat(perlin->GetValue(0.0, 0.0, t)) * amplitude;
	    }));
}

void CQuakeComponent::CancelScreenShake()
{
	StopShake();
	if(m_cbScreenShake.IsValid())
		m_cbScreenShake.Remove();
}
void CQuakeComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CEnvQuake::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CQuakeComponent>();
}
