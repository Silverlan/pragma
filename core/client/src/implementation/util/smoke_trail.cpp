// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :util.smoke_trail;
pragma::ecs::CParticleSystemComponent *pragma::util::create_smoke_trail_particle(float distance, float speed, float minSpriteSize, float maxSpriteSize, const std::string &material)
{
	std::unordered_map<std::string, std::string> values = {{"maxparticles", "150"}, {"emission_rate", "50"}, {"material", material}, {"sort_particles", "1"}};
	auto *pt = ecs::CParticleSystemComponent::Create(values);
	if(pt == nullptr)
		return nullptr;
	auto lifeTime = distance / speed;
	auto strLifeTime = std::to_string(lifeTime);
	auto spread = (minSpriteSize / 45.f) * 0.1f;
	auto strSpread = std::to_string(spread);
	pt->AddInitializer("radius_random", std::unordered_map<std::string, std::string> {{"radius_min", std::to_string(minSpriteSize)}, {"radius_max", std::to_string(maxSpriteSize)}});
	pt->AddInitializer("lifetime_random", std::unordered_map<std::string, std::string> {{"lifetime_min", strLifeTime}, {"lifetime_max", strLifeTime}});
	pt->AddInitializer("initial_velocity", std::unordered_map<std::string, std::string> {{"velocity", "0 " + std::to_string(speed) + " 0"}, {"spread_min", "-" + strSpread + "0 -" + strSpread}, {"spread_max", strSpread + " 0 " + strSpread}});
	pt->AddInitializer("position_random_sphere", std::unordered_map<std::string, std::string> {{"distance_min", "0"}, {"distance_max", "5"}});
	pt->AddInitializer("rotation_random", std::unordered_map<std::string, std::string> {{"rotation_min", "0"}, {"rotation_max", "360"}});
	pt->AddOperator("color_fade", std::unordered_map<std::string, std::string> {{"color", "255 255 255 0"}, {"fade_start", std::to_string(math::max(lifeTime - 0.66f, 0.f))}, {"fade_end", strLifeTime}});
	pt->AddRenderer("sprite", std::unordered_map<std::string, std::string> {});

	pt->SetContinuous(true);
	return pt;
}
