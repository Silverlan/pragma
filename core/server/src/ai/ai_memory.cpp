/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/ai/ai_memory.h"
#include <pragma/entities/baseentity.h>
#include <pragma/entities/baseentity_handle.h>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/entities/components/velocity_component.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <algorithm>

using namespace pragma;

extern SGame *s_game;

ai::Memory::Fragment::Fragment() : hEntity(), lastSeen(0.f), visible(false), lastCheck(0.f), occupied(false), lastHeared(0.f) { lastDistance = std::numeric_limits<float>::max(); }

void ai::Memory::Fragment::Clear()
{
	hEntity = EntityHandle {};
	occupied = false;
	lastDistance = std::numeric_limits<float>::max();
	lastSeen = 0.f;
	lastCheck = 0.f;
	visible = false;
	lastHeared = 0.f;
}

float ai::Memory::Fragment::GetLastTimeSensed() const { return umath::max(lastSeen, lastHeared); }

void ai::Memory::Fragment::UpdateVisibility(float dist)
{
	if(!hEntity.valid())
		return;
	auto *ent = hEntity.get();

	auto pTrComponent = ent->GetTransformComponent();
	lastPosition = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3 {};

	auto pVelComponent = ent->GetComponent<pragma::VelocityComponent>();
	lastVelocity = pVelComponent.valid() ? pVelComponent->GetVelocity() : Vector3 {};

	lastDistance = dist;
	lastSeen = CFloat(s_game->CurTime());
	lastHeared = lastSeen; // TODO
	lastCheck = lastSeen;
}

///////////////////////

ai::Memory::Memory() : occupiedFragmentCount(0) {}

void ai::Memory::Update()
{
	for(auto &fragment : fragments) {
		if(fragment.occupied == true && fragment.hEntity.valid()) {
			fragment.Clear();
			--occupiedFragmentCount;
		}
	}
}

void ai::Memory::Memorize(const BaseEntity &ent, MemoryType memType, const Vector3 &pos, float dist, const Vector3 &vel, int idx, ai::Memory::Fragment **out)
{
	auto &fragment = fragments[idx];
	fragment.lastPosition = pos;
	fragment.lastVelocity = vel;
	fragment.lastDistance = dist;
	fragment.lastCheck = CFloat(s_game->CurTime());
	switch(memType) {
	case MemoryType::Visual:
		{
			fragment.lastSeen = fragment.lastCheck;
			fragment.visible = true;
			break;
		}
	case MemoryType::Sound:
		{
			fragment.lastHeared = fragment.lastSeen;
			fragment.visible = false;
			break;
		}
	case MemoryType::Smell:
		break; // Not yet implemented
	}

	if(out != nullptr)
		*out = &fragment;
}

void ai::Memory::Clear(Fragment &fragment)
{
	if(fragment.occupied == true)
		--occupiedFragmentCount;
	fragment.Clear();
}

void ai::Memory::Clear()
{
	for(auto &fragment : fragments) {
		if(fragment.occupied == true)
			fragment.Clear();
	}
	occupiedFragmentCount = 0;
}

ai::Memory::Fragment *ai::Memory::FindFragment(const BaseEntity &ent)
{
	auto it = std::find_if(fragments.begin(), fragments.end(), [&ent](const ai::Memory::Fragment &fragment) { return (fragment.occupied == true && fragment.hEntity.get() == &ent) ? true : false; });
	if(it == fragments.end())
		return nullptr;
	return &(*it);
}

void ai::Memory::Forget(const BaseEntity &ent)
{
	auto *fragment = FindFragment(ent);
	if(fragment == nullptr)
		return;
	fragment->Clear();
	--occupiedFragmentCount;
}

bool ai::Memory::Memorize(const BaseEntity &ent, MemoryType memType, const Vector3 &pos, float dist, const Vector3 &vel, ai::Memory::Fragment **out)
{
	int32_t freeIndex = -1;
	for(auto i = decltype(fragments.size()) {0}; i < fragments.size(); ++i) {
		auto &fragment = fragments[i];
		if(fragment.occupied == false) {
			if(freeIndex == -1)
				freeIndex = static_cast<int32_t>(i);
		}
		else if(fragment.hEntity.get() == &ent) {
			Memorize(ent, memType, pos, dist, vel, static_cast<int32_t>(i), out);
			return false;
		}
	}
	if(freeIndex == -1)
		return false;
	++occupiedFragmentCount;
	fragments[freeIndex].hEntity = ent.GetHandle();
	fragments[freeIndex].occupied = true;
	if(out != nullptr)
		*out = &fragments[freeIndex];
	Memorize(ent, memType, pos, dist, vel, freeIndex);
	return true;
}
