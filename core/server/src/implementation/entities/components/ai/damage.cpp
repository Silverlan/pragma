// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "stdafx_server.h"
#include <pragma/entities/components/base_character_component.hpp>

module pragma.server.entities.components.ai;

import pragma.server.entities.components.character;

using namespace pragma;

void SAIComponent::OnTakenDamage(DamageInfo &info, unsigned short oldHealth, unsigned short newHealth)
{
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.valid() && charComponent->IsAlive() == false)
		return;
	std::array<BaseEntity *, 2> ents = {info.GetAttacker(), info.GetInflictor()};
	//auto &t = s_game->CurTime();
	for(auto *ent : ents) {
		if(ent == nullptr || (!ent->IsPlayer() && !ent->IsNPC()) || HasCharacterNoTargetEnabled(*ent) == true)
			continue;
		auto *mem = GetMemory(ent);
		if(mem != nullptr) {
			if(mem->visible == true)
				continue;
			//auto tLast = mem->GetLastTimeSensed();
			//auto tDelta = t -tLast;

			// TODO
		}
	}
	if(oldHealth == 0 || newHealth != 0)
		return;
	DisableAI();
	if(charComponent.valid())
		charComponent->Kill(&info);
}

void SAIComponent::OnKilled(DamageInfo *damageInfo)
{
	auto *nw = GetEntity().GetNetworkState();
	auto *game = nw->GetGameState();
	game->CallCallbacks<void, pragma::BaseAIComponent *, DamageInfo *>("OnNPCDeath", this, damageInfo);
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.valid())
		charComponent->RemoveWeapons();
}

void SAIComponent::OnTakeDamage(DamageInfo &info)
{
	auto *charComponent = static_cast<pragma::SCharacterComponent *>(GetEntity().GetCharacterComponent().get());
	if(charComponent != nullptr && charComponent->GetGodMode() == true)
		info.SetDamage(0);
}
