// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.ai;

import :game;

using namespace pragma;

void SAIComponent::UpdateMemory()
{
	double t = SGame::Get()->CurTime();
	for(auto &fragment : m_memory.fragments) {
		if(fragment.occupied == true) {
			float dist;
			if(!fragment.hEntity.valid() || (fragment.hEntity->IsCharacter() && fragment.hEntity->GetCharacterComponent()->IsAlive() == false) || (!fragment.visible && t - fragment.GetLastTimeSensed() >= m_memoryDuration) || HasCharacterNoTargetEnabled(*fragment.hEntity.get()) == true)
				m_memory.Clear(fragment);
			else if(t - fragment.lastSeen >= (fragment.visible ? AI_MEMORY_NEXT_CHECK_IF_HIDDEN : AI_MEMORY_NEXT_CHECK_IF_VISIBLE)) {
				if(!IsInViewCone(fragment.hEntity.get(), &dist)) {
					auto bVisible = fragment.visible;
					fragment.visible = false;
					if(bVisible == true)
						OnTargetVisibilityLost(fragment);
				}
				else {
					auto bVisible = fragment.visible;
					fragment.visible = true;
					fragment.UpdateVisibility(dist);
					if(bVisible == false)
						OnTargetVisibilityReacquired(fragment);
				}
				fragment.lastCheck = CFloat(t);
			}
		}
	}
}

void SAIComponent::OnTargetVisibilityLost(const ai::Memory::Fragment &fragment)
{
	CEMemoryData evData {&fragment};
	BroadcastEvent(sAIComponent::EVENT_ON_TARGET_VISIBILITY_LOST, evData);
}
void SAIComponent::OnTargetVisibilityReacquired(const ai::Memory::Fragment &fragment)
{
	CEMemoryData evData {&fragment};
	BroadcastEvent(sAIComponent::EVENT_ON_TARGET_VISIBILITY_REACQUIRED, evData);
}

bool SAIComponent::IsInMemory(ecs::BaseEntity *ent) { return GetMemory(ent); }

void SAIComponent::OnPrimaryTargetChanged(const ai::Memory::Fragment *fragment)
{
	CEMemoryData evData {fragment};
	BroadcastEvent(sAIComponent::EVENT_ON_PRIMARY_TARGET_CHANGED, evData);
}

void SAIComponent::SelectPrimaryTarget()
{
	// TODO: Check relationship intensity (priority), as well as additional conditions
	float dClosest = std::numeric_limits<float>::max();
	int iPrimary = -1;
	for(int i = 0; i < MAX_AIMEMORY_FRAGMENTS; i++) {
		auto &fragment = m_memory.fragments[i];
		if(fragment.occupied == true && fragment.hEntity.valid() && HasCharacterNoTargetEnabled(*fragment.hEntity.get()) == false) {
			if(fragment.lastDistance < dClosest) {
				auto charComponent = fragment.hEntity.get()->GetCharacterComponent();
				if(charComponent.expired() || charComponent->IsAlive()) {
					dClosest = fragment.lastDistance;
					iPrimary = i;
				}
			}
		}
	}
	auto *tgt = m_primaryTarget;
	if(iPrimary == -1)
		m_primaryTarget = nullptr;
	else
		m_primaryTarget = &m_memory.fragments[iPrimary];
	if(tgt == m_primaryTarget)
		return;
	OnPrimaryTargetChanged(m_primaryTarget);
}

ai::Memory &SAIComponent::GetMemory() { return m_memory; }
ai::Memory::Fragment *SAIComponent::GetMemory(ecs::BaseEntity *ent)
{
	auto it = std::find_if(m_memory.fragments.begin(), m_memory.fragments.end(), [ent](const ai::Memory::Fragment &fragment) { return (fragment.occupied == true && fragment.hEntity.get() == ent) ? true : false; });
	if(it == m_memory.fragments.end())
		return nullptr;
	return &(*it);
}

float SAIComponent::GetMemoryDuration() { return m_memoryDuration; }
void SAIComponent::SetMemoryDuration(float dur) { m_memoryDuration = dur; }

ai::Memory::Fragment *SAIComponent::Memorize(ecs::BaseEntity *ent, ai::Memory::MemoryType memType, const Vector3 &pos, const Vector3 &vel)
{
	if(&GetEntity() == ent || HasCharacterNoTargetEnabled(*ent) == true)
		return nullptr;
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent == nullptr)
		return nullptr;
	ai::Memory::Fragment *fragment = nullptr;
	if(m_memory.Memorize(*ent, memType, pos, uvec::distance(pTrComponent->GetEyePosition(), pos), vel, &fragment) == true)
		OnMemoryGained(*fragment);
	return fragment;
}
ai::Memory::Fragment *SAIComponent::Memorize(ecs::BaseEntity *ent, ai::Memory::MemoryType memType)
{
	auto pTrComponent = ent->GetTransformComponent();
	if(pTrComponent == nullptr)
		return nullptr;
	auto pVelComponent = ent->GetComponent<VelocityComponent>();
	return Memorize(ent, memType, pTrComponent->GetEyePosition(), pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {});
}
void SAIComponent::Forget(ecs::BaseEntity *ent)
{
	auto *fragment = m_memory.FindFragment(*ent);
	if(fragment == nullptr)
		return;
	if(m_primaryTarget == fragment)
		m_primaryTarget = nullptr;
	OnMemoryLost(*fragment);
	m_memory.Clear(*fragment);
	//m_memory.Forget(ent);
}
void SAIComponent::ClearMemory()
{
	for(auto &fragment : m_memory.fragments) {
		if(fragment.occupied == true)
			OnMemoryLost(fragment);
	}
	m_memory.Clear();
	m_primaryTarget = nullptr;
}

void SAIComponent::OnMemoryGained(const ai::Memory::Fragment &fragment)
{
	CEMemoryData evData {&fragment};
	BroadcastEvent(sAIComponent::EVENT_ON_MEMORY_GAINED, evData);
}
void SAIComponent::OnMemoryLost(const ai::Memory::Fragment &fragment)
{
	CEMemoryData evData {&fragment};
	BroadcastEvent(sAIComponent::EVENT_ON_MEMORY_LOST, evData);
}

const ai::Memory::Fragment *SAIComponent::GetPrimaryTarget() const { return m_primaryTarget; }

uint32_t SAIComponent::GetMemoryFragmentCount() const { return m_memory.occupiedFragmentCount; }
