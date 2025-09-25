// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:physics.enums;

export {
    enum class MOVETYPE : int { NONE, WALK, FLY, NOCLIP, FREE, PHYSICS };

    enum class CollisionMask : uint32_t {
        None = 0,
        Static = 1,
        Dynamic = Static << 1,
        Generic = Dynamic << 1,
        Player = Generic << 1,
        NPC = Player << 1,
        Vehicle = NPC << 1,
        Item = Vehicle << 1,
        Particle = Item << 1,
        Character = Particle << 1,
        Trigger = Character << 1,
        Water = Trigger << 1,
        WaterSurface = Water << 1,
        All = (WaterSurface << 1) - 1,
        Default = WaterSurface << 1,

        PlayerHitbox = Default << 1,
        NPCHitbox = PlayerHitbox << 1,
        CharacterHitbox = NPCHitbox | PlayerHitbox,
        AllHitbox = All & ~Player & ~NPC | CharacterHitbox,

        NoCollision = NPCHitbox << 1 // Only use as collision group, never as collision mask
    };
    REGISTER_BASIC_ARITHMETIC_OPERATORS(CollisionMask);

    enum class COLLISIONTYPE : int { NONE, AABB, OBB, BRUSH };

    enum class PHYSICSTYPE : int { NONE, DYNAMIC, STATIC, SOFTBODY, BOXCONTROLLER, CAPSULECONTROLLER };
};
