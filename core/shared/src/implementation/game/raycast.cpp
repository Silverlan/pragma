// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/physics/raytraces.h"
#include "pragma/physics/collision_object.hpp"
#include "pragma/physics/shape.hpp"
#include "pragma/physics/environment.hpp"
#include "pragma/physics/raycast_filter.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/entity_component_system_t.hpp"
#include <pragma/math/intersection.h>

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
