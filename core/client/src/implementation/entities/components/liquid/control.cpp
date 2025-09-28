// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_client.h"

module pragma.client;


import :entities.components.liquid_control;
import :client_state;
import :entities.components.particle_system;
import :game;


using namespace pragma;

void CLiquidControlComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CLiquidControlComponent::ReceiveData(NetPacket &packet)
{
	auto surfMat = packet->ReadString();
	GetEntity().SetKeyValue("surface_material", surfMat);
}

Bool CLiquidControlComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvCreateSplash) {
		auto origin = packet->Read<Vector3>();
		auto radius = packet->Read<float>();
		auto force = packet->Read<float>();
		CreateSplash(origin, radius, force);
	}
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}

bool CLiquidControlComponent::OnBulletHit(const BulletInfo &bulletInfo, const TraceData &data, PhysObj *phys, pragma::physics::ICollisionObject *col, const LocalRayResult &result)
{
	auto srcOrigin = data.GetSourceOrigin();
	auto dir = data.GetDirection();
	auto dist = data.GetDistance();
	auto hitPos = srcOrigin + dir * (dist * static_cast<float>(result.friction));

	auto surfMatId = col->GetSurfaceMaterial();
	auto *surfMat = pragma::get_cgame()->GetSurfaceMaterial(surfMatId);
	if(surfMat != nullptr) {
		auto &ptEffect = surfMat->GetImpactParticleEffect();
		if(ptEffect.empty() == false) {
			auto *pt = pragma::ecs::CParticleSystemComponent::Create(ptEffect);
			if(pt != nullptr) {
				auto pTrComponent = pt->GetEntity().GetTransformComponent();
				if(pTrComponent != nullptr) {
					pTrComponent->SetPosition(hitPos);

					auto up = result.hitNormalLocal;
					uvec::normalize(&up);
					const auto rot = Quat {0.5f, -0.5f, -0.5f, -0.5f};
					pTrComponent->SetRotation(uquat::create_look_rotation(uvec::get_perpendicular(up), up) * rot);
				}
				pt->SetRemoveOnComplete(true);
				pt->Start();
			}
		}

		auto &bulletImpactSnd = surfMat->GetBulletImpactSound();
		if(bulletImpactSnd.empty() == false) {
			auto snd = pragma::get_client_state()->CreateSound(bulletImpactSnd, ALSoundType::Effect | ALSoundType::Physics, ALCreateFlags::Mono);
			if(snd != nullptr) {
				snd->SetPosition(hitPos);
				snd->SetType(ALSoundType::Effect);
				snd->Play();
			}
		}
	}
	return BaseLiquidControlComponent::OnBulletHit(bulletInfo, data, phys, col, result);
}
