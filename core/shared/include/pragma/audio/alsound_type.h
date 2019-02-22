#ifndef __ALSOUND_TYPE_H__
#define __ALSOUND_TYPE_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/umath.h>

enum class DLLNETWORK ALSoundType : Int32
{
	Generic = 0,
	Effect = 1,
	Music = Effect<<1,
	Voice = Music<<1,
	Weapon = Voice<<1,
	NPC = Weapon<<1,
	Player = NPC<<1,
	Vehicle = Player<<1,
	Physics = Vehicle<<1,
	Environment = Physics<<1,
	GUI = Environment<<1,

	All = Effect | Music | Voice | Weapon | NPC | Player | Vehicle | Physics | Environment | GUI
};
REGISTER_BASIC_BITWISE_OPERATORS(ALSoundType)

#endif
