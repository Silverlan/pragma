// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <ostream>

#include <vector>

#include <string>

module pragma.shared;

import :game.game;

Bool pragma::Game::Overlap(const TraceData &data, std::vector<TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->Overlap(data, optOutResults);
}
Bool pragma::Game::RayCast(const TraceData &data, std::vector<TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->RayCast(data, optOutResults);
}
Bool pragma::Game::Sweep(const TraceData &data, std::vector<TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->Sweep(data, optOutResults);
}
TraceResult pragma::Game::Overlap(const TraceData &data) const
{
	std::vector<TraceResult> results {};
	if(Overlap(data, &results) == false) {
		TraceResult result {};
		result.hitType = pragma::physics::RayCastHitType::None;
		return result;
	}
	return results.front();
}
TraceResult pragma::Game::RayCast(const TraceData &data) const
{
	std::vector<TraceResult> results {};
	if(RayCast(data, &results) == false) {
		TraceResult result {};
		result.hitType = pragma::physics::RayCastHitType::None;
		return result;
	}
	return results.front();
}
TraceResult pragma::Game::Sweep(const TraceData &data) const
{
	std::vector<TraceResult> results {};
	if(Sweep(data, &results) == false) {
		TraceResult result {};
		result.hitType = pragma::physics::RayCastHitType::None;
		return result;
	}
	return results.front();
}
