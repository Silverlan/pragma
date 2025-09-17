// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <string>

export module pragma.client.util.smoke_trail;

import pragma.client.entities.components.particle_system;

export namespace util {
	DLLCLIENT ::pragma::ecs::CParticleSystemComponent *create_smoke_trail_particle(float distance, float speed, float minSpriteSize, float maxSpriteSize, const std::string &material = "particles/smoke_sprites_dense");
};
