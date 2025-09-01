// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include <string>

export module pragma.client.util.smoke_trail;

export namespace util {
	DLLCLIENT ::pragma::CParticleSystemComponent *create_smoke_trail_particle(float distance, float speed, float minSpriteSize, float maxSpriteSize, const std::string &material = "particles/smoke_sprites_dense");
};
