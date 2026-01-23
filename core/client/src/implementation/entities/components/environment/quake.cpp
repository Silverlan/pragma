// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "noise/noise.h"

module pragma.client;

import :entities.components.env_quake;
import :client_state;
import :engine;
import :entities.components.player;
import :game;

using namespace pragma;

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
	auto perlin = pragma::util::make_shared<noise::module::Perlin>();
	perlin->SetSeed(CInt32(get_cengine()->GetTickCount()));
	m_tStartShake = CFloat(get_cgame()->CurTime());
	m_cbScreenShake = get_cgame()->AddCallback("CalcView",
	  FunctionCallback<void, std::reference_wrapper<Vector3>, std::reference_wrapper<Quat>, std::reference_wrapper<Quat>>::Create([this, perlin](std::reference_wrapper<Vector3> refPos, std::reference_wrapper<Quat>, std::reference_wrapper<Quat>) {
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

		  auto &t = get_cgame()->CurTime();
		  auto tDelta = t - m_tStartShake;
		  if(tDelta >= duration) {
			  if(ShouldRemoveOnComplete())
				  entThis.RemoveSafely();
			  m_cbScreenShake.Remove();
			  return;
		  }
		  auto bGlobal = IsGlobal();
		  auto *pl = get_cgame()->GetLocalPlayer();
		  if(pl == nullptr || (radius == 0.f && bGlobal == false))
			  return;
		  auto &ent = pl->GetEntity();
		  auto pPhysComponent = ent.GetPhysicsComponent();
		  auto pTrComponentEnt = ent.GetTransformComponent();
		  if((InAir() == false && (pPhysComponent == nullptr || pPhysComponent->IsOnGround() == false)) || !pTrComponentEnt)
			  return;
		  auto dist = uvec::distance(origin, pos); //pTrComponentEnt->GetPosition());
		  auto scale = (bGlobal == true) ? 1.f : (1.f - math::clamp(dist / radius, 0.f, 1.f));
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
void CQuakeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CEnvQuake::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CQuakeComponent>();
}
