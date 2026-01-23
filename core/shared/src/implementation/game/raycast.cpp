// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.game;

Bool pragma::Game::Overlap(const physics::TraceData &data, std::vector<physics::TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->Overlap(data, optOutResults);
}
Bool pragma::Game::RayCast(const physics::TraceData &data, std::vector<physics::TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->RayCast(data, optOutResults);
}
Bool pragma::Game::Sweep(const physics::TraceData &data, std::vector<physics::TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->Sweep(data, optOutResults);
}
pragma::physics::TraceResult pragma::Game::Overlap(const physics::TraceData &data) const
{
	std::vector<physics::TraceResult> results {};
	if(Overlap(data, &results) == false) {
		physics::TraceResult result {};
		result.hitType = physics::RayCastHitType::None;
		return result;
	}
	return results.front();
}
pragma::physics::TraceResult pragma::Game::RayCast(const physics::TraceData &data) const
{
	std::vector<physics::TraceResult> results {};
	if(RayCast(data, &results) == false) {
		physics::TraceResult result {};
		result.hitType = physics::RayCastHitType::None;
		return result;
	}
	return results.front();
}
pragma::physics::TraceResult pragma::Game::Sweep(const physics::TraceData &data) const
{
	std::vector<physics::TraceResult> results {};
	if(Sweep(data, &results) == false) {
		physics::TraceResult result {};
		result.hitType = physics::RayCastHitType::None;
		return result;
	}
	return results.front();
}
