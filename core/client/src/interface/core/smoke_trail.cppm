// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:util.smoke_trail;

export import std;

export namespace pragma::ecs {
	class CParticleSystemComponent;
}
export namespace pragma::util {
	DLLCLIENT ecs::CParticleSystemComponent *create_smoke_trail_particle(float distance, float speed, float minSpriteSize, float maxSpriteSize, const std::string &material = "particles/smoke_sprites_dense");
};
