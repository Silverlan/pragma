// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :game.game;

void pragma::Game::InitializeEntityComponents(EntityComponentManager &componentManager)
{
	constexpr auto hideInEditor = ComponentRegInfo::Flags::HideInEditor;
	componentManager.RegisterComponentType<DamageableComponent>("damageable", {"gameplay"});
	// componentManager.RegisterComponentType<pragma::IKComponent>("ik", {"animation"});
	componentManager.RegisterComponentType<LogicComponent>("logic", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<GravityComponent>("gravity", {"physics"});
	componentManager.RegisterComponentType<MapComponent>("map", {"world", hideInEditor});
	componentManager.RegisterComponentType<SubmergibleComponent>("submergible", {"physics"});
	componentManager.RegisterComponentType<VelocityComponent>("velocity", {"physics"});
	componentManager.RegisterComponentType<UsableComponent>("usable", {"gameplay"});
	componentManager.RegisterComponentType<GlobalNameComponent>("global", {"core", hideInEditor});
	componentManager.RegisterComponentType<PanimaComponent>("panima", {"animation"});
	componentManager.RegisterComponentType<ecs::CompositeComponent>("composite", {"core", hideInEditor});
	componentManager.RegisterComponentType<AnimationDriverComponent>("animation_driver", {"animation"});
	componentManager.RegisterComponentType<OriginComponent>("origin", {"world/transform"});
	componentManager.RegisterComponentType<ConstraintComponent>("constraint", {"animation/constraints"});
	componentManager.RegisterComponentType<ConstraintManagerComponent>("constraint_manager", {"animation/constraints", hideInEditor});
	componentManager.RegisterComponentType<ConstraintCopyLocationComponent>("constraint_copy_location", {"animation/constraints"});
	componentManager.RegisterComponentType<ConstraintCopyRotationComponent>("constraint_copy_rotation", {"animation/constraints"});
	componentManager.RegisterComponentType<ConstraintCopyScaleComponent>("constraint_copy_scale", {"animation/constraints"});
	componentManager.RegisterComponentType<ConstraintLimitDistanceComponent>("constraint_limit_distance", {"animation/constraints"});
	componentManager.RegisterComponentType<ConstraintLimitLocationComponent>("constraint_limit_location", {"animation/constraints"});
	componentManager.RegisterComponentType<ConstraintLimitRotationComponent>("constraint_limit_rotation", {"animation/constraints"});
	componentManager.RegisterComponentType<ConstraintLimitScaleComponent>("constraint_limit_scale", {"animation/constraints"});
	componentManager.RegisterComponentType<ConstraintSpaceComponent>("constraint_space", {"animation/constraints", hideInEditor});
	componentManager.RegisterComponentType<ConstraintLookAtComponent>("constraint_look_at", {"animation/constraints"});
	componentManager.RegisterComponentType<ConstraintChildOfComponent>("constraint_child_of", {"animation/constraints"});
	componentManager.RegisterComponentType<LifelineLinkComponent>("lifeline_link", {"core", hideInEditor});
	componentManager.RegisterComponentType<IntersectionHandlerComponent>("intersection_handler", {"core", hideInEditor});
	componentManager.RegisterComponentType<MetaRigComponent>("meta_rig", {"animation"});
	componentManager.RegisterComponentType<BoneMergeComponent>("bone_merge", {"animation"});
	componentManager.RegisterComponentType<FlexMergeComponent>("flex_merge", {"animation"});
	componentManager.RegisterComponentType<ParentComponent>("parent", {"core", hideInEditor});
	componentManager.RegisterComponentType<MovementComponent>("movement", {"physics", hideInEditor});
	componentManager.RegisterComponentType<OrientationComponent>("orientation", {"world/transform"});
	componentManager.RegisterComponentType<ActionInputControllerComponent>("action_input_controller", {"input", hideInEditor});
	componentManager.RegisterComponentType<InputMovementControllerComponent>("input_movement_controller", {"input", hideInEditor});

	BaseEntityComponent::RegisterEvents(componentManager, [&componentManager](const std::string &evName, ComponentEventInfo::Type type) -> ComponentEventId { return componentManager.RegisterEvent(evName, typeid(BaseEntityComponent), type); });
	ecs::BaseEntity::RegisterEvents(componentManager);
}
