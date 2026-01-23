// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.audio.sound_scape;
import :game;

using namespace pragma;

namespace pragma {
	using ::operator<<;
};

CSoundScapeComponent *CSoundScapeComponent::s_active = nullptr;

CSoundScapeComponent::~CSoundScapeComponent()
{
	if(s_active == this)
		s_active = nullptr;
	StopSoundScape();
}

void CSoundScapeComponent::Initialize()
{
	BaseEnvSoundScapeComponent::Initialize();
	SetTickPolicy(TickPolicy::Always); // TODO
}

void CSoundScapeComponent::OnTick(double dt)
{
	if(m_sound.get() == nullptr)
		return;
	auto &entThis = GetEntity();
	auto pTrComponent = entThis.GetTransformComponent();
	SetNextTick(get_cgame()->CurTime() + 0.25f);
	if(pTrComponent != nullptr && IsPlayerInRange()) {
		if(s_active != this) {
			auto *pl = get_cgame()->GetLocalPlayer();
			auto &ent = pl->GetEntity();
			auto charComponentEnt = ent.GetCharacterComponent();
			auto pTrComponentEnt = ent.GetTransformComponent();
			if(charComponentEnt.valid() || pTrComponentEnt) {
				physics::TraceData tr;
				tr.SetSource(charComponentEnt.valid() ? charComponentEnt->GetEyePosition() : pTrComponentEnt->GetPosition());
				tr.SetTarget(pTrComponent->GetPosition());
				tr.SetFlags(physics::RayCastFlags::Default | physics::RayCastFlags::IgnoreDynamic);
				auto result = get_cgame()->RayCast(tr);
				if(result.hitType == physics::RayCastHitType::None)
					StartSoundScape();
			}
		}
	}
	if(s_active == this)
		UpdateTargetPositions();
}

void CSoundScapeComponent::OnEntitySpawn()
{
	BaseEnvSoundScapeComponent::OnEntitySpawn();
	for(auto &pair : m_positions) {
		ecs::EntityIterator itEnt {*get_cgame()};
		itEnt.AttachFilter<EntityIteratorFilterEntity>(pair.second);
		auto it = itEnt.begin();
		if(it != itEnt.end())
			m_targets.insert(std::unordered_map<unsigned int, EntityHandle>::value_type(pair.first, (*it)->GetHandle()));
	}
	m_sound = nullptr;
	auto &ent = GetEntity();
	auto pSoundEmitterComponent = ent.GetComponent<CSoundEmitterComponent>();
	if(pSoundEmitterComponent.valid()) {
		std::shared_ptr<audio::ALSound> snd = pSoundEmitterComponent->CreateSound(m_kvSoundScape, audio::ALSoundType::Environment);
		if(snd.get() == nullptr) {
			Con::CWAR << "Invalid soundscape '" << m_kvSoundScape << "' for entity " << this << Con::endl;
			return;
		}
		audio::ALSoundScript *al = dynamic_cast<audio::ALSoundScript *>(snd.get());
		if(al == nullptr) {
			Con::CWAR << "Invalid soundscape '" << m_kvSoundScape << "' for entity " << this << Con::endl;
			return;
		}
		snd->SetRelative(true);
		m_sound = snd;
	}
	SetNextTick(get_cgame()->CurTime() + math::random(0.f, 0.25f)); // Spread out think time between entities
}

void CSoundScapeComponent::ReceiveData(NetPacket &packet)
{
	m_kvSoundScape = packet->ReadString();
	m_kvRadius = packet->Read<float>();
	unsigned int numPositions = packet->Read<unsigned int>();
	for(unsigned int i = 0; i < numPositions; i++) {
		unsigned int id = packet->Read<unsigned int>();
		std::string name = packet->ReadString();
		m_positions.insert(std::unordered_map<unsigned int, std::string>::value_type(id, name));
	}
}

void CSoundScapeComponent::UpdateTargetPositions()
{
	if(m_sound.get() == nullptr)
		return;
	audio::ALSoundScript *al = dynamic_cast<audio::ALSoundScript *>(m_sound.get());
	std::unordered_map<unsigned int, EntityHandle>::iterator it;
	for(it = m_targets.begin(); it != m_targets.end(); it++) {
		EntityHandle &hEnt = it->second;
		if(hEnt.valid()) {
			auto pTrComponent = hEnt->GetTransformComponent();
			if(pTrComponent != nullptr)
				al->SetTargetPosition(it->first, pTrComponent->GetPosition());
		}
	}
}

void CSoundScapeComponent::StartSoundScape()
{
	if(s_active == this)
		return;
	if(s_active != nullptr) {
		if(s_active->IsPlayerInRange())
			return;
		s_active->StopSoundScape();
	}
	s_active = this;
	m_sound->SetGain(1);
	m_sound->FadeIn(2);
}

void CSoundScapeComponent::StopSoundScape()
{
	if(s_active != this)
		return;
	s_active = nullptr;
	m_sound->FadeOut(2);
}

bool CSoundScapeComponent::IsPlayerInRange()
{
	auto *pl = get_cgame()->GetLocalPlayer();
	if(pl == nullptr)
		return false;
	auto &ent = GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pTrComponentPl = pl->GetEntity().GetTransformComponent();
	if(pTrComponent == nullptr || !pTrComponentPl)
		return false;
	auto &origin = pTrComponent->GetPosition();
	auto &pos = pTrComponentPl->GetPosition();
	return (glm::distance(origin, pos) <= m_kvRadius) ? true : false;
}
void CSoundScapeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

/////////////

void CEnvSoundScape::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSoundScapeComponent>();
}
