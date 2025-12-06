// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.game;

Bool pragma::Game::Overlap(const pragma::physics::TraceData &data, std::vector<pragma::physics::TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->Overlap(data, optOutResults);
}
Bool pragma::Game::RayCast(const pragma::physics::TraceData &data, std::vector<pragma::physics::TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->RayCast(data, optOutResults);
}
Bool pragma::Game::Sweep(const pragma::physics::TraceData &data, std::vector<pragma::physics::TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->Sweep(data, optOutResults);
}
pragma::physics::TraceResult pragma::Game::Overlap(const pragma::physics::TraceData &data) const
{
	std::vector<pragma::physics::TraceResult> results {};
	if(Overlap(data, &results) == false) {
		pragma::physics::TraceResult result {};
		result.hitType = pragma::physics::RayCastHitType::None;
		return result;
	}
	return results.front();
}
pragma::physics::TraceResult pragma::Game::RayCast(const pragma::physics::TraceData &data) const
{
	std::vector<pragma::physics::TraceResult> results {};
	if(RayCast(data, &results) == false) {
		pragma::physics::TraceResult result {};
		result.hitType = pragma::physics::RayCastHitType::None;
		return result;
	}
	return results.front();
}
pragma::physics::TraceResult pragma::Game::Sweep(const pragma::physics::TraceData &data) const
{
	std::vector<pragma::physics::TraceResult> results {};
	if(Sweep(data, &results) == false) {
		pragma::physics::TraceResult result {};
		result.hitType = pragma::physics::RayCastHitType::None;
		return result;
	}
	return results.front();
}
