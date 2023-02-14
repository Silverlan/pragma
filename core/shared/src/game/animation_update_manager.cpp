/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/game/animation_update_manager.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/animation_driver_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/components/constraints/constraint_manager_component.hpp"
#include "pragma/entities/components/ik_solver_component.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/entity_component_system_t.hpp"

pragma::AnimationUpdateManager::AnimationUpdateManager(Game &game) : game {game}, m_threadPool {10, "animation_update"}
{
	auto &componentManager = game.GetEntityComponentManager();
	auto r = componentManager.GetComponentTypeId("animated", m_animatedComponentId);
	r = r && componentManager.GetComponentTypeId("panima", m_panimaComponentId);
	r = r && componentManager.GetComponentTypeId("animation_driver", m_animationDriverComponentId);
	r = r && componentManager.GetComponentTypeId("constraint_manager", m_constraintManagerComponentId);
	assert(r);
	if(!r) {
		Con::crit << "Unable to determine animated component ids!" << Con::endl;
		exit(EXIT_FAILURE);
	}
}
void pragma::AnimationUpdateManager::UpdateEntityState(BaseEntity &ent)
{
	auto animC = ent.GetAnimatedComponent();
	auto panimaC = ent.GetComponent<PanimaComponent>();
	auto it = std::find_if(m_animatedEntities.begin(), m_animatedEntities.end(), [&ent](const AnimatedEntity &animEnt) { return animEnt.entity == &ent; });
	if(animC.expired() && panimaC.expired()) {
		if(it != m_animatedEntities.end())
			m_animatedEntities.erase(it);
		return;
	}
	if(it == m_animatedEntities.end()) {
		m_animatedEntities.push_back({});
		it = m_animatedEntities.end() - 1;
	}
	it->entity = &ent;
	it->animatedC = animC.get();
	it->panimaC = panimaC.get();
}
const std::vector<pragma::AnimationUpdateManager::AnimatedEntity> &pragma::AnimationUpdateManager::GetAnimatedEntities() const { return m_animatedEntities; }
void pragma::AnimationUpdateManager::UpdateEntityAnimationDrivers(double dt)
{
	for(auto *ent : EntityIterator {game, m_animationDriverComponentId})
		ent->GetComponent<pragma::AnimationDriverComponent>()->ApplyDriver();
}
void pragma::AnimationUpdateManager::UpdateConstraints(double dt)
{
	for(auto *ent : EntityIterator {game, m_constraintManagerComponentId})
		ent->GetComponent<pragma::ConstraintManagerComponent>()->ApplyConstraints();
}
void pragma::AnimationUpdateManager::UpdateAnimations(double dt)
{
	for(auto &entInfo : m_animatedEntities) {
		auto maintainAnimations = entInfo.animatedC ? entInfo.animatedC->PreMaintainAnimations(dt) : false;
		m_threadPool.AddTask([this, &entInfo, dt, maintainAnimations]() -> pragma::ThreadPool::ResultHandler {
			if(maintainAnimations)
				entInfo.animatedC->UpdateAnimations(dt);

			if(entInfo.panimaC)
				entInfo.panimaC->UpdateAnimations(dt);
			return nullptr;
		});
	}

	m_threadPool.WaitForCompletion();

	// The remaining steps have to be executed on the main thread because
	// they may affect arbitrary component properties or call listeners and events,
	// which can't be guaranteed to be thread-safe in all cases.

	UpdateEntityAnimationDrivers(dt);

	// This will also update IK solvers
	UpdateConstraints(dt);

	{
		EntityIterator entIt {game, m_animatedComponentId};
		for(auto *ent : entIt) {
			auto animC = ent->GetAnimatedComponent();
			animC->HandleAnimationEvents();
		}
	}
}
