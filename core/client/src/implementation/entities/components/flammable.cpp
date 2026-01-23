// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.flammable;
import :entities.components.sound_emitter;
import :util;

using namespace pragma;

CFlammableComponent::IgniteInfo::IgniteInfo() {}

void CFlammableComponent::IgniteInfo::Clear()
{
	if(sound != nullptr) {
		sound->FadeOut(0.5f);
		sound = nullptr;
	}
	for(auto &p : flameParticles) {
		if(p.hParticle.valid())
			p.hParticle->Die();
	}
}

CFlammableComponent::IgniteInfo::~IgniteInfo() { Clear(); }

CFlammableComponent::IgniteInfo::Particle::Particle(ecs::CParticleSystemComponent &pt, uint32_t _boneId) : hParticle(std::static_pointer_cast<ecs::CParticleSystemComponent>(pt.shared_from_this())), boneId(_boneId) {}

CFlammableComponent::IgniteInfo::Particle::Particle(ecs::CParticleSystemComponent &pt, const Vector3 &_offset) : hParticle(std::static_pointer_cast<ecs::CParticleSystemComponent>(pt.shared_from_this())), offset(_offset), boneId(0) {}

//////////////////////////////////

void CFlammableComponent::Initialize()
{
	BaseFlammableComponent::Initialize();
	auto &ent = GetEntity();
	ent.AddComponent<CSoundEmitterComponent>();
}
void CFlammableComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
Bool CFlammableComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetIgnitable)
		SetIgnitable(packet->Read<bool>());
	else if(eventId == m_netEvIgnite) {
		auto duration = packet->Read<float>();
		auto *attacker = networking::read_entity(packet);
		auto *inflictor = networking::read_entity(packet);
		Ignite(duration, attacker, inflictor);
	}
	else if(eventId == m_netEvExtinguish)
		Extinguish();
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
void CFlammableComponent::OnTick(double dt)
{
	BaseFlammableComponent::OnTick(dt);
	UpdateFlameParticlePositions();
}
void CFlammableComponent::ReceiveData(NetPacket &packet)
{
	SetIgnitable(packet->Read<bool>());
	auto bOnFire = packet->Read<bool>();
	if(bOnFire == true) {
		auto t = packet->Read<float>();
		auto *attacker = networking::read_entity(packet);
		auto *inflictor = networking::read_entity(packet);
		Ignite(t, attacker, inflictor);
	}
}
void CFlammableComponent::UpdateFlameParticlePositions()
{
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(!IsOnFire() || pTrComponent == nullptr)
		return;
	auto &rot = pTrComponent->GetRotation();
	for(auto &info : m_igniteInfo.flameParticles) {
		if(info.hParticle.expired())
			continue;
		auto pTrComponent = info.hParticle->GetEntity().GetTransformComponent();
		if(pTrComponent == nullptr)
			continue;
		Vector3 pos {};
		if(info.boneId == 0) {
			pos = info.offset;
			uvec::rotate(&pos, rot);
			pos += pTrComponent->GetPosition();
		}
		else {
			auto animComponent = ent.GetAnimatedComponent();
			if(animComponent.expired() || animComponent->GetBonePos(info.boneId, pos, math::CoordinateSpace::World) == false)
				pos = ent.GetCenter();
		}
		pTrComponent->SetPosition(pos);
	}
}
util::EventReply CFlammableComponent::Ignite(float duration, ecs::BaseEntity *attacker, ecs::BaseEntity *inflictor)
{
	auto bOnFire = IsOnFire();
	if(BaseFlammableComponent::Ignite(duration, attacker, inflictor) == util::EventReply::Handled)
		return util::EventReply::Handled;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	if(bOnFire == true || pTrComponent == nullptr)
		return util::EventReply::Handled;
	auto pSndComponent = ent.GetComponent<CSoundEmitterComponent>();
	if(pSndComponent.valid()) {
		auto snd = pSndComponent->CreateSound("fx.fire_small", audio::ALSoundType::Effect);
		if(snd != nullptr) {
			snd->FadeIn(0.6f);
			snd->SetLooping(true);
			m_igniteInfo.sound = snd;
		}
	}
	const std::unordered_map<std::string, std::string> values = {{"maxparticles", "40"}, {"emission_rate", "20"}, {"material", "particles/fire_sprites_v3"}, {"sort_particles", "1"}, {"bloom_scale", "0.8"}};
	const std::unordered_map<std::string, std::string> lifetimeRandom = {{"lifetime_min", "0.3"}, {"lifetime_max", "0.3"}};
	const std::unordered_map<std::string, std::string> initialVelocity = {{"velocity", "0 50 0"}, {"spread_min", "-0.3 0 -0.3"}, {"spread_max", "0.3 0 0.3"}};
	const std::unordered_map<std::string, std::string> rotationRandom = {{"rotation_min", "-45"}, {"rotation_max", "45"}};
	const std::unordered_map<std::string, std::string> colorFade = {{"color", "255 255 255 0"}, {"fade_start", "0.2"}, {"fade_end", "0.3"}};
	auto pPhysComponent = ent.GetPhysicsComponent();
	auto extents = pPhysComponent != nullptr ? pPhysComponent->GetCollisionExtents() : Vector3 {};
	auto radius = uvec::length(extents);
	auto particleDistance = radius * 0.06f;
	auto distSqr = particleDistance * particleDistance;
	auto particleCount = math::clamp(math::ceil(radius / 10.f), 12, 20);
	auto particleRadius = math::max(radius / 3.f, 14.f);

	const std::unordered_map<std::string, std::string> radiusRandom = {{"radius_min", std::to_string(particleRadius)}, {"radius_max", std::to_string(particleRadius + 25)}};

	struct ParticleInfo {
		ParticleInfo(const Vector3 &_position, uint32_t _boneId) : position(_position), boneId(_boneId) {}
		Vector3 position;
		uint32_t boneId;
	};

	std::vector<ParticleInfo> particlePositions;
	particlePositions.reserve(particleCount);
	decltype(particleCount) particleId = 0;
	auto &mdl = ent.GetModel();
	if(mdl != nullptr) {
		auto &skeleton = mdl->GetSkeleton();
		auto &bones = skeleton.GetBones();
		auto &reference = mdl->GetReference();
		for(auto &bone : bones) {
			if(bone->ID == 0)
				continue;
			auto *pos = reference.GetBonePosition(bone->ID);
			if(pos != nullptr) {
				auto it = std::find_if(particlePositions.begin(), particlePositions.end(), [pos, distSqr](const ParticleInfo &info) { return (uvec::length_sqr(info.position - (*pos)) < distSqr) ? true : false; });
				if(it == particlePositions.end())
					particlePositions.push_back({*pos, bone->ID});
			}
		}
	}

	// Create remaining particles
	Vector3 min {};
	Vector3 max {};
	if(pPhysComponent != nullptr)
		pPhysComponent->GetCollisionBounds(&min, &max);
	for(auto i = particleId; i < particleCount; ++i) {
		Vector3 pos {};
		for(uint8_t i = 0; i < 3; ++i) {
			auto r = math::random(0.f, 1.f);
			pos[i] = min[i] + (max[i] - min[i]) * r;
		}
		particlePositions.push_back({pos, 0}); // TODO Check pos distance
	}
	//

	auto &origin = pTrComponent->GetPosition();
	auto &rot = pTrComponent->GetRotation();
	for(auto &info : particlePositions) {
		auto pos = info.position;
		uvec::rotate(&pos, rot);
		pos += origin;
		auto *pt = ecs::CParticleSystemComponent::Create(values);
		if(pt != nullptr) {
			pt->AddRenderer("sprite", {});
			pt->AddInitializer("radius_random", radiusRandom);
			pt->AddInitializer("lifetime_random", lifetimeRandom);
			pt->AddInitializer("initial_velocity", initialVelocity);
			pt->AddInitializer("rotation_random", rotationRandom);
			pt->AddInitializer("color_fade", colorFade);
			auto pTrComponent = pt->GetEntity().GetTransformComponent();
			if(pTrComponent != nullptr) {
				pTrComponent->SetPosition(pos);
				pTrComponent->SetRotation(rot);
			}
			pt->SetRemoveOnComplete(true);
			pt->SetContinuous(true);
			pt->Start();
			if(info.boneId != 0)
				m_igniteInfo.flameParticles.push_back({*pt, info.boneId});
			else
				m_igniteInfo.flameParticles.push_back({*pt, info.position});
		}
		pt = util::create_smoke_trail_particle(60.f, 80.f, 22.f, 50.f);
		if(pt != nullptr) {
			auto pTrComponent = pt->GetEntity().GetTransformComponent();
			if(pTrComponent != nullptr)
				pTrComponent->SetPosition(pos);
			pt->SetRemoveOnComplete(true);
			pt->Start();
			if(info.boneId != 0)
				m_igniteInfo.flameParticles.push_back({*pt, info.boneId});
			else
				m_igniteInfo.flameParticles.push_back({*pt, info.position});
		}
	}
	return util::EventReply::Handled;
}
void CFlammableComponent::Extinguish()
{
	if(!IsOnFire())
		return;
	BaseFlammableComponent::Extinguish();
	m_igniteInfo.Clear();
}
