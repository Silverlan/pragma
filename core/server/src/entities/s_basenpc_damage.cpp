#include "stdafx_server.h"
#if 0
#include "pragma/entities/player.h"

void SBaseNPC::OnTakenDamage(DamageInfo &info,unsigned short oldHealth,unsigned short newHealth)
{
	if(IsAlive() == false)
		return;
	Entity::OnTakenDamage(info,oldHealth,newHealth);
	std::array<BaseEntity*,2> ents = {info.GetAttacker(),info.GetInflictor()};
	//auto &t = s_game->CurTime();
	for(auto *ent : ents)
	{
		if(ent == nullptr || (!ent->IsPlayer() && !ent->IsNPC()) || HasCharacterNoTargetEnabled(*ent) == true)
			continue;
		auto *mem = GetMemory(ent);
		if(mem != nullptr)
		{
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
	Kill(&info);
}

void SBaseNPC::Kill(DamageInfo *damageInfo)
{
	auto *nw = GetNetworkState();
	auto *game = nw->GetGameState();
	game->CallCallbacks<void,pragma::BaseAIComponent*,DamageInfo*>("OnNPCDeath",this,damageInfo);
	BaseCharacter::Kill();
	RemoveWeapons();
}

void SBaseNPC::OnTakeDamage(DamageInfo &info)
{
	Entity::OnTakeDamage(info);
	if(GetGodMode() == true)
		info.SetDamage(0);
}
#endif
