// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.liquid_control;
import :client_state;
import :entities.components.particle_system;
import :game;

using namespace pragma;

void CLiquidControlComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CLiquidControlComponent::ReceiveData(NetPacket &packet)
{
	auto surfMat = packet->ReadString();
	GetEntity().SetKeyValue("surface_material", surfMat);
}

Bool CLiquidControlComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
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

bool CLiquidControlComponent::OnBulletHit(const game::BulletInfo &bulletInfo, const physics::TraceData &data, physics::PhysObj *phys, physics::ICollisionObject *col, const LocalRayResult &result)
{
	auto srcOrigin = data.GetSourceOrigin();
	auto dir = data.GetDirection();
	auto dist = data.GetDistance();
	auto hitPos = srcOrigin + dir * (dist * static_cast<float>(result.friction));

	auto surfMatId = col->GetSurfaceMaterial();
	auto *surfMat = get_cgame()->GetSurfaceMaterial(surfMatId);
	if(surfMat != nullptr) {
		auto &ptEffect = surfMat->GetImpactParticleEffect();
		if(ptEffect.empty() == false) {
			auto *pt = ecs::CParticleSystemComponent::Create(ptEffect);
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
			auto snd = get_client_state()->CreateSound(bulletImpactSnd, audio::ALSoundType::Effect | audio::ALSoundType::Physics, audio::ALCreateFlags::Mono);
			if(snd != nullptr) {
				snd->SetPosition(hitPos);
				snd->SetType(audio::ALSoundType::Effect);
				snd->Play();
			}
		}
	}
	return BaseLiquidControlComponent::OnBulletHit(bulletInfo, data, phys, col, result);
}
