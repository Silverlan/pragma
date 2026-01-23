// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.shooter;

import :entities.base;
import :entities.components;
import :game;
import :server_state;
import pragma.shared;

using namespace pragma;

Bool ecs::SShooterComponent::ReceiveNetEvent(BasePlayerComponent &pl, NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvFireBullets)
		ReceiveBulletEvent(packet, &pl);
	else
		return false;
	return true;
}
void ecs::SShooterComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void ecs::SShooterComponent::FireBullets(const game::BulletInfo &bulletInfo, const std::function<bool(game::DamageInfo &, BaseEntity *)> &fCallback, std::vector<physics::TraceResult> &outHitTargets, bool bMaster)
{
	game::DamageInfo dmg;
	dmg.SetAttacker(bulletInfo.hAttacker.valid() ? bulletInfo.hAttacker.get() : &GetEntity());
	dmg.SetInflictor(bulletInfo.hInflictor.valid() ? bulletInfo.hInflictor.get() : &GetEntity());
	dmg.SetDamageType(Bullet);

	auto bCustomForce = (isnan(bulletInfo.force) == false) ? true : false;
	if(bCustomForce == true)
		dmg.SetForce(Vector3(bulletInfo.force, 0, 0));

	auto bCustomDamageType = (bulletInfo.damageType != util::declvalue(&game::BulletInfo::damageType)) ? true : false;
	if(bCustomDamageType == true)
		dmg.SetDamageType(bulletInfo.damageType);

	auto bCustomDamage = (bulletInfo.damage != util::declvalue(&game::BulletInfo::damage)) ? true : false;
	if(bCustomDamage == true)
		dmg.SetDamage(static_cast<uint16_t>(bulletInfo.damage));

	if(!bulletInfo.ammoType.empty()) {
		auto *ammoType = SGame::Get()->GetAmmoType(bulletInfo.ammoType);
		if(ammoType != nullptr) {
			if(bCustomDamage == false)
				dmg.SetDamage(static_cast<uint16_t>(ammoType->damage));
			if(bCustomDamageType == false)
				dmg.SetDamageType(ammoType->damageType);
			if(bCustomForce == false)
				dmg.SetForce(Vector3(ammoType->force, 0, 0)); // Force is stored in x-axis, later converted to actual velocity
		}
	}
	FireBullets(bulletInfo, dmg, outHitTargets, fCallback, bMaster);
}
void ecs::SShooterComponent::FireBullets(const game::BulletInfo &bulletInfo, std::vector<physics::TraceResult> &results, bool bMaster) { FireBullets(bulletInfo, nullptr, results, bMaster); }

void ecs::SShooterComponent::FireBullets(const game::BulletInfo &bulletInfo, game::DamageInfo &dmgInfo, std::vector<physics::TraceResult> &outHitTargets, const std::function<bool(game::DamageInfo &, BaseEntity *)> &fCallback, bool bMaster)
{
	BasePlayerComponent *pl = nullptr;
	if(bMaster == false) {
		if(m_nextBullet == nullptr) // No bullet has been scheduled
			return;
		auto *ent = static_cast<SBaseEntity *>(m_nextBullet->source.get());
		if(ent == nullptr || ent->IsPlayer() == false)
			return;
		pl = ent->GetPlayerComponent().get();
	}
	Vector3 origin {};
	Vector3 dir {};
	OnFireBullets(bulletInfo, origin, dir);
	if(bMaster == true) {
		m_nextBullet = std::unique_ptr<NextBulletInfo>(new NextBulletInfo);
		m_nextBullet->destinations = GetBulletDestinations(origin, dir, bulletInfo);
	}
	util::ScopeGuard sg([this]() {
		m_nextBullet = nullptr; // We're done with this
	});

	if(m_nextBullet->destinations.size() != bulletInfo.bulletCount) // These are not the bullets we've been looking for
		return;
	dmgInfo.SetSource(origin);

	std::vector<Vector3> dstPositions;
	dstPositions.reserve(bulletInfo.bulletCount);

	physics::TraceData data;
	GetBulletTraceData(bulletInfo, data);
	outHitTargets.reserve(bulletInfo.bulletCount);
	for(auto i = decltype(bulletInfo.bulletCount) {0}; i < bulletInfo.bulletCount; ++i) {
		auto &bulletDst = m_nextBullet->destinations[i];
		auto bulletDir = bulletDst - origin;
		uvec::normalize(&bulletDir);
		// TODO: Add sanity checks to make sure client isn't cheating
		auto dst = origin + bulletDir * bulletInfo.distance;
		data.SetSource(origin);
		data.SetTarget(dst);
		dstPositions.push_back(dst);

		auto offset = outHitTargets.size();
		auto hit = SGame::Get()->RayCast(data, &outHitTargets);
		if(hit) {
			for(auto i = offset; i < outHitTargets.size(); ++i) {
				auto &result = outHitTargets.at(i);
				if(result.entity.valid() == false)
					continue;
				auto pDamageableComponent = result.entity->GetComponent<DamageableComponent>();
				if(pDamageableComponent.valid()) {
					auto hitGroup = physics::HitGroup::Generic;
					if(result.collisionObj.IsValid()) {
						auto charComponent = result.entity.get()->GetCharacterComponent();
						if(charComponent.valid())
							charComponent->FindHitgroup(*result.collisionObj.Get(), hitGroup);
					}
					dmgInfo.SetHitGroup(hitGroup);
					dmgInfo.SetForce(bulletDir * dmgInfo.GetForce().x);
					dmgInfo.SetHitPosition(result.position);
					if((fCallback == nullptr || fCallback(dmgInfo, result.entity.get()) == true) && pDamageableComponent.valid())
						pDamageableComponent->TakeDamage(dmgInfo);
				}
			}
		}
	}
	auto &ent = static_cast<SBaseEntity &>(GetEntity());
	if(ent.IsShared() == true) {
		auto numBullets = dstPositions.size();
		NetPacket p {};
		p->Write<uint32_t>(static_cast<uint32_t>(numBullets));
		for(auto &hitPos : dstPositions)
			p->Write<Vector3>(hitPos);
		if(bMaster == false) {
			auto *session = static_cast<SPlayerComponent *>(pl)->GetClientSession();
			ent.SendNetEvent(m_netEvFireBullets, p, networking::Protocol::FastUnreliable, session ? networking::ClientRecipientFilter {*session, networking::ClientRecipientFilter::FilterType::Exclude} : networking::ClientRecipientFilter {});
		}
		else
			ent.SendNetEvent(m_netEvFireBullets, p, networking::Protocol::FastUnreliable);
	}

	events::CEOnBulletsFired evData {bulletInfo, outHitTargets};
	BroadcastEvent(baseShooterComponent::EVENT_ON_BULLETS_FIRED, evData);
}
