#include "stdafx_server.h"
#if 0
extern DLLSERVER SGame *s_game;

void SBaseNPC::UpdateMemory()
{
	double t = s_game->CurTime();
	for(auto &fragment : m_memory.fragments)
	{
		if(fragment.occupied == true)
		{
			float dist;
			if(!fragment.hEntity.IsValid() || (fragment.hEntity->IsCharacter() && fragment.hEntity->GetCharacter()->IsAlive() == false) || (!fragment.visible && t -fragment.GetLastTimeSensed() >= m_memoryDuration) || HasCharacterNoTargetEnabled(*fragment.hEntity.get()) == true)
				m_memory.Clear(fragment);
			else if(t -fragment.lastSeen >= (fragment.visible ? AI_MEMORY_NEXT_CHECK_IF_HIDDEN : AI_MEMORY_NEXT_CHECK_IF_VISIBLE))
			{
				if(!IsInViewCone(fragment.hEntity.get(),&dist))
				{
					auto bVisible = fragment.visible;
					fragment.visible = false;
					if(bVisible == true)
						OnTargetVisibilityLost(fragment);
				}
				else
				{
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

void SBaseNPC::OnTargetVisibilityLost(const ai::Memory::Fragment&) {}
void SBaseNPC::OnTargetVisibilityReacquired(const ai::Memory::Fragment&) {}

bool SBaseNPC::IsInMemory(BaseEntity *ent)
{
	for(auto &fragment : m_memory.fragments)
	{
		if(fragment.occupied == true && fragment.hEntity.get() == ent)
			return true;
	}
	return false;
}

void SBaseNPC::OnPrimaryTargetChanged(const ai::Memory::Fragment*) {}

void SBaseNPC::SelectPrimaryTarget()
{
	// TODO: Check relationship intensity (priority), as well as additional conditions
	float dClosest = std::numeric_limits<float>::max();
	int iPrimary = -1;
	for(int i=0;i<MAX_AIMEMORY_FRAGMENTS;i++)
	{
		auto &fragment = m_memory.fragments[i];
		if(fragment.occupied == true && fragment.hEntity.IsValid() && HasCharacterNoTargetEnabled(*fragment.hEntity.get()) == false)
		{
			if(fragment.lastDistance < dClosest)
			{
				auto *character = dynamic_cast<BaseCharacter*>(fragment.hEntity.get());
				if(character == nullptr || character->IsAlive())
				{
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

ai::Memory &SBaseNPC::GetMemory() {return m_memory;}
ai::Memory::Fragment *SBaseNPC::GetMemory(BaseEntity *ent)
{
	auto it = std::find_if(m_memory.fragments.begin(),m_memory.fragments.end(),[ent](const ai::Memory::Fragment &fragment) {
		return (fragment.occupied == true && fragment.hEntity.get() == ent) ? true : false;
	});
	if(it == m_memory.fragments.end())
		return nullptr;
	return &(*it);
}

float SBaseNPC::GetMemoryDuration() {return m_memoryDuration;}
void SBaseNPC::SetMemoryDuration(float dur) {m_memoryDuration = dur;}

ai::Memory::Fragment *SBaseNPC::Memorize(BaseEntity *ent,ai::Memory::MemoryType memType,const Vector3 &pos,const Vector3 &vel)
{
	if(this == ent || HasCharacterNoTargetEnabled(*ent) == true)
		return nullptr;
	ai::Memory::Fragment *fragment = nullptr;
	if(m_memory.Memorize(*ent,memType,pos,uvec::distance(GetEyePosition(),pos),vel,&fragment) == true)
		OnMemoryGained(*fragment);
	return fragment;
}
ai::Memory::Fragment *SBaseNPC::Memorize(BaseEntity *ent,ai::Memory::MemoryType memType) {return Memorize(ent,memType,ent->GetEyePosition(),ent->GetVelocity());}
void SBaseNPC::Forget(BaseEntity *ent)
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
void SBaseNPC::ClearMemory()
{
	for(auto &fragment : m_memory.fragments)
	{
		if(fragment.occupied == true)
			OnMemoryLost(fragment);
	}
	m_memory.Clear();
	m_primaryTarget = nullptr;
}

void SBaseNPC::OnMemoryGained(const ai::Memory::Fragment&) {}
void SBaseNPC::OnMemoryLost(const ai::Memory::Fragment&) {}

const ai::Memory::Fragment *SBaseNPC::GetPrimaryTarget() const {return m_primaryTarget;}

uint32_t SBaseNPC::GetMemoryFragmentCount() const {return m_memory.occupiedFragmentCount;}
#endif
