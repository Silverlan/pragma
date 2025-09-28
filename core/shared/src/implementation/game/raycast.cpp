// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.shared;

import :game.game;

Bool Game::Overlap(const TraceData &data, std::vector<TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->Overlap(data, optOutResults);
}
Bool Game::RayCast(const TraceData &data, std::vector<TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->RayCast(data, optOutResults);
}
Bool Game::Sweep(const TraceData &data, std::vector<TraceResult> *optOutResults) const
{
	auto *physEnv = GetPhysicsEnvironment();
	if(physEnv == nullptr)
		return false;
	return physEnv->Sweep(data, optOutResults);
}
TraceResult Game::Overlap(const TraceData &data) const
{
	std::vector<TraceResult> results {};
	if(Overlap(data, &results) == false) {
		TraceResult result {};
		result.hitType = RayCastHitType::None;
		return result;
	}
	return results.front();
}
TraceResult Game::RayCast(const TraceData &data) const
{
	std::vector<TraceResult> results {};
	if(RayCast(data, &results) == false) {
		TraceResult result {};
		result.hitType = RayCastHitType::None;
		return result;
	}
	return results.front();
}
TraceResult Game::Sweep(const TraceData &data) const
{
	std::vector<TraceResult> results {};
	if(Sweep(data, &results) == false) {
		TraceResult result {};
		result.hitType = RayCastHitType::None;
		return result;
	}
	return results.front();
}
