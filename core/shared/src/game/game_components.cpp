/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include <pragma/game/game.h>
#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/components/global_component.hpp"
#include "pragma/entities/components/ik_component.hpp"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/usable_component.hpp"
#include "pragma/entities/components/panima_component.hpp"
#include "pragma/entities/components/parent_component.hpp"
#include "pragma/entities/components/composite_component.hpp"
#include "pragma/entities/components/movement_component.hpp"
#include "pragma/entities/components/orientation_component.hpp"
#include "pragma/entities/components/action_input_controller_component.hpp"
#include "pragma/entities/components/input_movement_controller_component.hpp"
#include "pragma/entities/components/intersection_handler_component.hpp"
#include "pragma/entities/components/constraints/constraint_component.hpp"
#include "pragma/entities/components/constraints/constraint_manager_component.hpp"
#include "pragma/entities/components/constraints/constraint_space_component.hpp"
#include "pragma/entities/components/constraints/constraint_copy_location_component.hpp"
#include "pragma/entities/components/constraints/constraint_copy_rotation_component.hpp"
#include "pragma/entities/components/constraints/constraint_copy_scale_component.hpp"
#include "pragma/entities/components/constraints/constraint_limit_distance_component.hpp"
#include "pragma/entities/components/constraints/constraint_limit_location_component.hpp"
#include "pragma/entities/components/constraints/constraint_limit_rotation_component.hpp"
#include "pragma/entities/components/constraints/constraint_limit_scale_component.hpp"
#include "pragma/entities/components/constraints/constraint_look_at_component.hpp"
#include "pragma/entities/components/constraints/constraint_child_of_component.hpp"
#include "pragma/entities/components/animation_driver_component.hpp"
#include "pragma/entities/components/bone_merge_component.hpp"
#include "pragma/entities/components/flex_merge_component.hpp"
#include "pragma/entities/components/origin_component.hpp"
#include "pragma/entities/components/lifeline_link_component.hpp"
#include "pragma/entities/components/meta_rig_component.hpp"
#include "pragma/entities/components/basegravity.h"

void Game::InitializeEntityComponents(pragma::EntityComponentManager &componentManager)
{
	constexpr auto hideInEditor = pragma::ComponentRegInfo::Flags::HideInEditor;
	componentManager.RegisterComponentType<pragma::DamageableComponent>("damageable", {"gameplay"});
	// componentManager.RegisterComponentType<pragma::IKComponent>("ik", {"animation"});
	componentManager.RegisterComponentType<pragma::LogicComponent>("logic", {"gameplay", hideInEditor});
	componentManager.RegisterComponentType<pragma::GravityComponent>("gravity", {"physics"});
	componentManager.RegisterComponentType<pragma::MapComponent>("map", {"world", hideInEditor});
	componentManager.RegisterComponentType<pragma::SubmergibleComponent>("submergible", {"physics"});
	componentManager.RegisterComponentType<pragma::VelocityComponent>("velocity", {"physics"});
	componentManager.RegisterComponentType<pragma::UsableComponent>("usable", {"gameplay"});
	componentManager.RegisterComponentType<pragma::GlobalNameComponent>("global", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::PanimaComponent>("panima", {"animation"});
	componentManager.RegisterComponentType<pragma::CompositeComponent>("composite", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::AnimationDriverComponent>("animation_driver", {"animation"});
	componentManager.RegisterComponentType<pragma::OriginComponent>("origin", {"world/transform"});
	componentManager.RegisterComponentType<pragma::ConstraintComponent>("constraint", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::ConstraintManagerComponent>("constraint_manager", {"animation/constraints", hideInEditor});
	componentManager.RegisterComponentType<pragma::ConstraintCopyLocationComponent>("constraint_copy_location", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::ConstraintCopyRotationComponent>("constraint_copy_rotation", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::ConstraintCopyScaleComponent>("constraint_copy_scale", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::ConstraintLimitDistanceComponent>("constraint_limit_distance", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::ConstraintLimitLocationComponent>("constraint_limit_location", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::ConstraintLimitRotationComponent>("constraint_limit_rotation", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::ConstraintLimitScaleComponent>("constraint_limit_scale", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::ConstraintSpaceComponent>("constraint_space", {"animation/constraints", hideInEditor});
	componentManager.RegisterComponentType<pragma::ConstraintLookAtComponent>("constraint_look_at", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::ConstraintChildOfComponent>("constraint_child_of", {"animation/constraints"});
	componentManager.RegisterComponentType<pragma::LifelineLinkComponent>("lifeline_link", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::IntersectionHandlerComponent>("intersection_handler", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::MetaRigComponent>("meta_rig", {"animation"});
	componentManager.RegisterComponentType<pragma::BoneMergeComponent>("bone_merge", {"animation"});
	componentManager.RegisterComponentType<pragma::FlexMergeComponent>("flex_merge", {"animation"});
	componentManager.RegisterComponentType<pragma::ParentComponent>("parent", {"core", hideInEditor});
	componentManager.RegisterComponentType<pragma::MovementComponent>("movement", {"physics", hideInEditor});
	componentManager.RegisterComponentType<pragma::OrientationComponent>("orientation", {"world/transform"});
	componentManager.RegisterComponentType<pragma::ActionInputControllerComponent>("action_input_controller", {"input", hideInEditor});
	componentManager.RegisterComponentType<pragma::InputMovementControllerComponent>("input_movement_controller", {"input", hideInEditor});

	pragma::BaseEntityComponent::RegisterEvents(componentManager, [&componentManager](const std::string &evName, pragma::ComponentEventInfo::Type type) -> pragma::ComponentEventId { return componentManager.RegisterEvent(evName, typeid(pragma::BaseEntityComponent), type); });
	BaseEntity::RegisterEvents(componentManager);
}
