// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.ai;

import :entities.components.character;

using namespace pragma;

void SAIComponent::OnTakenDamage(game::DamageInfo &info, unsigned short oldHealth, unsigned short newHealth)
{
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.valid() && charComponent->IsAlive() == false)
		return;
	std::array<ecs::BaseEntity *, 2> ents = {info.GetAttacker(), info.GetInflictor()};
	//auto &t = SGame::Get()->CurTime();
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

void SAIComponent::OnKilled(game::DamageInfo *damageInfo)
{
	auto *nw = GetEntity().GetNetworkState();
	auto *game = nw->GetGameState();
	game->CallCallbacks<void, BaseAIComponent *, game::DamageInfo *>("OnNPCDeath", this, damageInfo);
	auto charComponent = GetEntity().GetCharacterComponent();
	if(charComponent.valid())
		charComponent->RemoveWeapons();
}

void SAIComponent::OnTakeDamage(game::DamageInfo &info)
{
	auto *charComponent = static_cast<SCharacterComponent *>(GetEntity().GetCharacterComponent().get());
	if(charComponent != nullptr && charComponent->GetGodMode() == true)
		info.SetDamage(0);
}
