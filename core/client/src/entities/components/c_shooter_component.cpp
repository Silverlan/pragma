/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/entities/components/c_shooter_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <pragma/physics/raytraces.h>
#include <pragma/physics/environment.hpp>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <sharedutils/scope_guard.h>
#include <pragma/audio/alsound_type.h>

using namespace pragma;

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

void CShooterComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CShooterComponent::FireBullets(const BulletInfo &bulletInfo, const Vector3 &origin, const Vector3 &effectsOrigins, const std::vector<Vector3> &destPositions, bool bTransmitToServer, std::vector<TraceResult> &outHitTargets)
{
	auto *physEnv = c_game->GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return;

	auto numBullets = destPositions.size();
	NetPacket p {};
	if(bTransmitToServer == true)
		p->Write<uint32_t>(static_cast<uint32_t>(numBullets));
	TraceData data;
	GetBulletTraceData(bulletInfo, data);
	outHitTargets.reserve(numBullets);
	for(auto i = decltype(numBullets) {0}; i < numBullets; ++i) {
		data.SetSource(origin);
		data.SetTarget(destPositions[i]);

		auto offset = outHitTargets.size();
		auto hit = physEnv->RayCast(data, &outHitTargets);
		if(outHitTargets.size() == offset)
			outHitTargets.push_back(TraceResult {data}); // Empty trace result
		for(auto j = offset; j < outHitTargets.size(); ++j) {
			auto &result = outHitTargets.at(j);
			auto &hitPos = result.position;
			auto bulletDir = hitPos - origin;
			auto l = uvec::length(bulletDir);
			if(l > 0.f)
				bulletDir /= l;
			uvec::normalize(&bulletDir);
			if(bTransmitToServer == true)
				p->Write<Vector3>(destPositions[i]);
			const auto minTracerDistance = 80.f; // Don't show a tracer if the distance is less than this, otherwise the tracer might look odd when close to a wall.
			if(bulletInfo.tracerCount > 0 && (i % bulletInfo.tracerCount) == 0 && l > minTracerDistance)
				c_game->CreateParticleTracer(effectsOrigins, hitPos, bulletInfo.tracerRadius, bulletInfo.tracerColor, bulletInfo.tracerLength, bulletInfo.tracerSpeed, bulletInfo.tracerMaterial, bulletInfo.tracerBloom);
			if(result.hitType != RayCastHitType::None) {
				auto *col = result.collisionObj.Get();
				if(col != nullptr) {
					auto surfaceMaterialId = col->GetSurfaceMaterial();
					auto *surfaceMaterial = (surfaceMaterialId != -1) ? c_game->GetSurfaceMaterial(surfaceMaterialId) : nullptr;
					auto *surfaceMaterialGeneric = c_game->GetSurfaceMaterial(0);
					if(surfaceMaterial != nullptr || surfaceMaterialGeneric != nullptr) {
						auto particleEffect = (surfaceMaterial != nullptr) ? surfaceMaterial->GetImpactParticleEffect() : "";
						if(particleEffect.empty() && surfaceMaterialGeneric != nullptr)
							particleEffect = surfaceMaterialGeneric->GetImpactParticleEffect();
						if(!particleEffect.empty()) {
							auto *pt = CParticleSystemComponent::Create(particleEffect);
							if(pt != nullptr) {
								auto pTrComponent = pt->GetEntity().GetTransformComponent();
								if(pTrComponent != nullptr) {
									pTrComponent->SetPosition(result.position);

									auto ang = uvec::to_angle(result.normal);
									auto rot = uquat::create(ang);
									pTrComponent->SetRotation(rot);
								}
								pt->SetRemoveOnComplete(true);
								pt->GetEntity().Spawn();
								pt->Start();
							}
						}

						auto sndEffect = (surfaceMaterial != nullptr) ? surfaceMaterial->GetBulletImpactSound() : "";
						if(sndEffect.empty() && surfaceMaterialGeneric != nullptr)
							sndEffect = surfaceMaterialGeneric->GetBulletImpactSound();
						if(!sndEffect.empty()) {
							auto snd = client->CreateSound(sndEffect, ALSoundType::Effect | ALSoundType::Physics, ALCreateFlags::Mono);
							if(snd != nullptr) {
								snd->SetPosition(result.position);
								snd->Play();
							}
						}
					}
				}
			}
		}
	}
	if(bTransmitToServer == true)
		static_cast<CBaseEntity &>(GetEntity()).SendNetEventUDP(m_netEvFireBullets, p);

	CEOnBulletsFired evData {bulletInfo, outHitTargets};
	BroadcastEvent(EVENT_ON_BULLETS_FIRED, evData);
}

void CShooterComponent::FireBullets(const BulletInfo &bulletInfo, std::vector<TraceResult> &outHitTargets, bool bMaster)
{
	Vector3 origin {};
	Vector3 dir {};
	Vector3 effectsOrigin {};
	OnFireBullets(bulletInfo, origin, dir, &effectsOrigin);

	util::ScopeGuard sg([this]() { m_nextBullet = nullptr; });

	auto bTransmit = true;
	if(bMaster == false) // We weren't the trigger for the bullet shot; Most likely some other client
	{
		if(m_nextBullet != nullptr) {
			FireBullets(bulletInfo, origin, effectsOrigin, m_nextBullet->destinations, false, outHitTargets);
			return;
		}
		else
			bTransmit = false; // Just do a regular shot and hope no one will notice the discrepancy
	}
	FireBullets(bulletInfo, origin, effectsOrigin, GetBulletDestinations(origin, dir, bulletInfo), bTransmit, outHitTargets);
}
Bool CShooterComponent::ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvFireBullets)
		ReceiveBulletEvent(packet);
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
