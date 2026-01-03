// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.shooter;

using namespace pragma::ecs;

using namespace baseShooterComponent;
void BaseShooterComponent::RegisterEvents(EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent)
{
	baseShooterComponent::EVENT_ON_FIRE_BULLETS = registerEvent("ON_FIRE_BULLETS", ComponentEventInfo::Type::Broadcast);
	baseShooterComponent::EVENT_ON_BULLETS_FIRED = registerEvent("ON_BULLETS_FIRED", ComponentEventInfo::Type::Broadcast);
}
BaseShooterComponent::BaseShooterComponent(BaseEntity &ent) : BaseEntityComponent(ent) {}
void BaseShooterComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	m_netEvFireBullets = SetupNetEvent("fire_bullets");

	auto &ent = GetEntity();
	ent.AddComponent("transform");
}

void BaseShooterComponent::ReceiveBulletEvent(NetPacket &packet, BasePlayerComponent *pl)
{
	m_nextBullet = std::unique_ptr<NextBulletInfo>(new NextBulletInfo);
	auto numBullets = packet->Read<uint32_t>();
	m_nextBullet->destinations.reserve(numBullets);
	for(auto i = decltype(numBullets) {0}; i < numBullets; ++i)
		m_nextBullet->destinations.push_back(packet->Read<Vector3>());
	if(pl != nullptr)
		m_nextBullet->source = pl->GetEntity().GetHandle();
}

std::vector<Vector3> BaseShooterComponent::GetBulletDestinations(const Vector3 &origin, const Vector3 &dir, const game::BulletInfo &bulletInfo)
{
	std::vector<Vector3> destPositions;
	destPositions.reserve(bulletInfo.bulletCount);
	for(auto i = decltype(bulletInfo.bulletCount) {0}; i < bulletInfo.bulletCount; ++i) {
		auto randSpread = EulerAngles(math::random(-bulletInfo.spread.p, bulletInfo.spread.p), math::random(-bulletInfo.spread.y, bulletInfo.spread.y), 0);
		auto bulletDir = dir;
		uvec::rotate(&bulletDir, randSpread);

		destPositions.push_back(origin + bulletDir * bulletInfo.distance);
	}
	return destPositions;
}

void BaseShooterComponent::OnFireBullets(const game::BulletInfo &bulletInfo, Vector3 &bulletOrigin, Vector3 &bulletDir, Vector3 *effectsOrigin)
{
	auto pTrComponent = GetEntity().GetTransformComponent();
	if(pTrComponent) {
		bulletOrigin = pTrComponent->GetEyePosition();
		bulletDir = pTrComponent->GetForward();
	}
	if(effectsOrigin != nullptr)
		*effectsOrigin = bulletOrigin;

	events::CEOnFireBullets evData {bulletInfo, bulletOrigin, bulletDir, effectsOrigin};
	BroadcastEvent(baseShooterComponent::EVENT_ON_FIRE_BULLETS, evData);
}

pragma::physics::RayCastHitType BaseShooterComponent::OnBulletHit(const game::BulletInfo &bulletInfo, const physics::TraceData &data, physics::PhysObj &phys, physics::ICollisionObject &col) { return physics::RayCastHitType::Block; }

void BaseShooterComponent::GetBulletTraceData(const game::BulletInfo &bulletInfo, physics::TraceData &data) const
{
	auto *attacker = bulletInfo.hAttacker.get();
	auto *inflictor = bulletInfo.hInflictor.get();
	auto *entSrc = (attacker != nullptr) ? attacker : (inflictor != nullptr) ? inflictor : &GetEntity();
	data.SetCollisionFilterMask(physics::CollisionMask::AllHitbox & ~physics::CollisionMask::Trigger); // Let everything pass (Except specific filters below)
	data.SetFilter([this, &data, attacker, inflictor, &bulletInfo](physics::IShape &shape, physics::IRigidBody &body) -> physics::RayCastHitType {
		auto *phys = body.GetPhysObj();
		auto *ent = phys ? phys->GetOwner() : nullptr;
		if(ent == nullptr || &ent->GetEntity() == &GetEntity() || &ent->GetEntity() == attacker || &ent->GetEntity() == inflictor) // Attacker can't shoot themselves or the inflictor
			return physics::RayCastHitType::None;
		auto filterGroup = phys->GetCollisionFilter();
		auto mdlComponent = ent->GetEntity().GetModelComponent();
		if(mdlComponent && mdlComponent->GetHitboxCount() > 0 && (filterGroup & physics::CollisionMask::NPC) != physics::CollisionMask::None
		  || (filterGroup & physics::CollisionMask::Player) != physics::CollisionMask::None) // Filter out player and NPC collision objects, since we only want to check their hitboxes
			return physics::RayCastHitType::None;
		return const_cast<BaseShooterComponent *>(this)->OnBulletHit(bulletInfo, data, *phys, body);
	});
	auto physComponent = GetEntity().GetPhysicsComponent();
	auto filterGroup = physics::CollisionMask::None;
	if(physComponent)
		filterGroup = physComponent->GetCollisionFilter();
	filterGroup |= physics::CollisionMask::Water | physics::CollisionMask::WaterSurface | physics::CollisionMask::PlayerHitbox | physics::CollisionMask::NPCHitbox;
	data.SetCollisionFilterGroup(filterGroup);
}

//////////////

events::CEOnFireBullets::CEOnFireBullets(const game::BulletInfo &bulletInfo, Vector3 &bulletOrigin, Vector3 &bulletDir, Vector3 *effectsOrigin) : bulletInfo {bulletInfo}, bulletOrigin {bulletOrigin}, bulletDir {bulletDir}, effectsOrigin {effectsOrigin} {}
void events::CEOnFireBullets::PushArguments(lua::State *l)
{
	Lua::Push<game::BulletInfo *>(l, &const_cast<game::BulletInfo &>(bulletInfo));
	Lua::Push<Vector3>(l, bulletOrigin);
	Lua::Push<Vector3>(l, bulletDir);
	if(effectsOrigin != nullptr)
		Lua::Push<Vector3>(l, *effectsOrigin);
	else
		Lua::PushNil(l);
}
uint32_t events::CEOnFireBullets::GetReturnCount() { return 3; }
void events::CEOnFireBullets::HandleReturnValues(lua::State *l)
{
	if(Lua::IsSet(l, -3))
		bulletOrigin = Lua::Check<Vector3>(l, -3);
	if(Lua::IsSet(l, -2))
		bulletDir = Lua::Check<Vector3>(l, -2);
	if(Lua::IsSet(l, -1) && effectsOrigin != nullptr)
		*effectsOrigin = Lua::Check<Vector3>(l, -1);
}

//////////////

events::CEOnBulletsFired::CEOnBulletsFired(const game::BulletInfo &bulletInfo, const std::vector<physics::TraceResult> &hitTargets) : bulletInfo {bulletInfo}, hitTargets {hitTargets} {}
void events::CEOnBulletsFired::PushArguments(lua::State *l)
{
	Lua::Push<game::BulletInfo *>(l, &const_cast<game::BulletInfo &>(bulletInfo));

	auto t = Lua::CreateTable(l);
	for(auto i = decltype(hitTargets.size()) {0}; i < hitTargets.size(); ++i) {
		Lua::PushInt(l, i + 1);
		Lua::Push<physics::TraceResult *>(l, const_cast<physics::TraceResult *>(&hitTargets.at(i)));
		Lua::SetTableValue(l, t);
	}
}
