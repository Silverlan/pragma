#ifndef __CollisionMaskS_H__
#define __CollisionMaskS_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class CollisionMask : uint32_t
{
	None = 0,
	Static = 1,
	Dynamic = Static<<1,
	Generic = Dynamic<<1,
	Player = Generic<<1,
	NPC = Player<<1,
	Vehicle = NPC<<1,
	Item = Vehicle<<1,
	Particle = Item<<1,
	Character = Particle<<1,
	Trigger = Character<<1,
	Water = Trigger<<1,
	WaterSurface = Water<<1,
	All = (WaterSurface<<1) -1,
	Default = WaterSurface<<1,

	PlayerHitbox = Default<<1,
	NPCHitbox = PlayerHitbox<<1,
	CharacterHitbox = NPCHitbox | PlayerHitbox,
	AllHitbox = All &~Player &~NPC | CharacterHitbox,

	NoCollision = NPCHitbox<<1 // Only use as collision group, never as collision mask
};
REGISTER_BASIC_ARITHMETIC_OPERATORS(CollisionMask);

#endif