// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <cstdlib>
#include <cassert>

module pragma.shared;

import :game.animation_update_manager;

pragma::AnimationUpdateManager::AnimationUpdateManager(Game &game) : game {game}
{
	auto &componentManager = game.GetEntityComponentManager();
	auto r = componentManager.GetComponentTypeId("animated", m_animatedComponentId);
	r = r && componentManager.GetComponentTypeId("panima", m_panimaComponentId);
	r = r && componentManager.GetComponentTypeId("animation_driver", m_animationDriverComponentId);
	r = r && componentManager.GetComponentTypeId("constraint_manager", m_constraintManagerComponentId);
	assert(r);
	if(!r) {
		Con::CRIT << "Unable to determine animated component ids!" << Con::endl;
		exit(EXIT_FAILURE);
	}
}
void pragma::AnimationUpdateManager::UpdateEntityState(ecs::BaseEntity &ent)
{
	auto animC = ent.GetAnimatedComponent();
	if(animC.valid() && math::is_flag_set(animC->GetStateFlags(), BaseEntityComponent::StateFlags::Removed))
		animC = pragma::ComponentHandle<BaseAnimatedComponent> {};

	auto panimaC = ent.GetComponent<PanimaComponent>();
	if(panimaC.valid() && math::is_flag_set(panimaC->GetStateFlags(), BaseEntityComponent::StateFlags::Removed))
		panimaC = pragma::ComponentHandle<PanimaComponent> {};

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
	for(auto *ent : ecs::EntityIterator {game, m_animationDriverComponentId})
		ent->GetComponent<AnimationDriverComponent>()->ApplyDriver();
}
void pragma::AnimationUpdateManager::UpdateConstraints(double dt) { ConstraintManagerComponent::ApplyConstraints(*game.GetNetworkState()); }

static auto cvDisableAnimUpdates = pragma::console::get_con_var("debug_disable_animation_updates");
void pragma::AnimationUpdateManager::UpdateAnimations(double dt)
{
	m_channelQueueProcessor.Reset();

	if(cvDisableAnimUpdates->GetBool())
		return;
	auto t = std::chrono::steady_clock::now();
	game.StartProfilingStage("UpdateAnimations");
	for(auto &entInfo : m_animatedEntities) {
		game.StartProfilingStage("UpdateSkeletalAnimation");
		auto maintainAnimations = entInfo.animatedC ? entInfo.animatedC->PreMaintainAnimations(dt) : false;
		if(maintainAnimations)
			entInfo.animatedC->UpdateAnimations(dt);
		game.StopProfilingStage();

		if(entInfo.panimaC) {
			game.StartProfilingStage("UpdatePanimaAnimation");
			// This will calculate the new animation data (multi-threaded), but not apply the values yet
			entInfo.panimaC->UpdateAnimations(m_channelQueueProcessor, dt);
			game.StopProfilingStage();
		}
	}
	game.StopProfilingStage();

	game.StartProfilingStage("ApplyAnimationValues");
	for(auto &entInfo : m_animatedEntities) {
		if(entInfo.panimaC) {
			// Apply the new animation values
			entInfo.panimaC->ApplyAnimationValues(&m_channelQueueProcessor);
		}
		if(entInfo.animatedC && entInfo.animatedC->IsPostAnimationUpdateEnabled())
			m_postAnimListenerQueue.push_back(entInfo.animatedC);
	}
	game.StopProfilingStage();

	// The remaining steps have to be executed on the main thread because
	// they may affect arbitrary component properties or call listeners and events,
	// which can't be guaranteed to be thread-safe in all cases.
	game.StartProfilingStage("UpdateEntityAnimationDrivers");
	UpdateEntityAnimationDrivers(dt);
	game.StopProfilingStage();

	// This will also update IK solvers
	game.StartProfilingStage("UpdateConstraints");
	UpdateConstraints(dt);
	game.StopProfilingStage();

	for(auto *animC : m_postAnimListenerQueue)
		animC->PostAnimationsUpdated();
	m_postAnimListenerQueue.clear();

	game.StartProfilingStage("HandleAnimationEvents");
	{
		ecs::EntityIterator entIt {game, m_animatedComponentId};
		for(auto *ent : entIt) {
			auto animC = ent->GetAnimatedComponent();
			animC->HandleAnimationEvents();
		}
	}
	game.StopProfilingStage();
}
