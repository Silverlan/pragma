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
	componentManager.RegisterComponentType<pragma::DamageableComponent>("damageable");
	componentManager.RegisterComponentType<pragma::IKComponent>("ik");
	componentManager.RegisterComponentType<pragma::LogicComponent>("logic");
	componentManager.RegisterComponentType<pragma::GravityComponent>("gravity");
	componentManager.RegisterComponentType<pragma::MapComponent>("map");
	componentManager.RegisterComponentType<pragma::SubmergibleComponent>("submergible");
	componentManager.RegisterComponentType<pragma::VelocityComponent>("velocity");
	componentManager.RegisterComponentType<pragma::UsableComponent>("usable");
	componentManager.RegisterComponentType<pragma::GlobalNameComponent>("global");
	componentManager.RegisterComponentType<pragma::PanimaComponent>("panima");
	componentManager.RegisterComponentType<pragma::CompositeComponent>("composite");
	componentManager.RegisterComponentType<pragma::AnimationDriverComponent>("animation_driver");
	componentManager.RegisterComponentType<pragma::OriginComponent>("origin");
	componentManager.RegisterComponentType<pragma::ConstraintComponent>("constraint");
	componentManager.RegisterComponentType<pragma::ConstraintManagerComponent>("constraint_manager");
	componentManager.RegisterComponentType<pragma::ConstraintCopyLocationComponent>("constraint_copy_location");
	componentManager.RegisterComponentType<pragma::ConstraintCopyRotationComponent>("constraint_copy_rotation");
	componentManager.RegisterComponentType<pragma::ConstraintCopyScaleComponent>("constraint_copy_scale");
	componentManager.RegisterComponentType<pragma::ConstraintLimitDistanceComponent>("constraint_limit_distance");
	componentManager.RegisterComponentType<pragma::ConstraintLimitLocationComponent>("constraint_limit_location");
	componentManager.RegisterComponentType<pragma::ConstraintLimitRotationComponent>("constraint_limit_rotation");
	componentManager.RegisterComponentType<pragma::ConstraintLimitScaleComponent>("constraint_limit_scale");
	componentManager.RegisterComponentType<pragma::ConstraintSpaceComponent>("constraint_space");
	componentManager.RegisterComponentType<pragma::ConstraintLookAtComponent>("constraint_look_at");
	componentManager.RegisterComponentType<pragma::ConstraintChildOfComponent>("constraint_child_of");
	componentManager.RegisterComponentType<pragma::LifelineLinkComponent>("lifeline_link");
	componentManager.RegisterComponentType<pragma::IntersectionHandlerComponent>("intersection_handler");
	componentManager.RegisterComponentType<pragma::MetaRigComponent>("meta_rig");
	componentManager.RegisterComponentType<pragma::BoneMergeComponent>("bone_merge");
	componentManager.RegisterComponentType<pragma::FlexMergeComponent>("flex_merge");
	componentManager.RegisterComponentType<pragma::ParentComponent>("parent");
	componentManager.RegisterComponentType<pragma::MovementComponent>("movement");
	componentManager.RegisterComponentType<pragma::OrientationComponent>("orientation");
	componentManager.RegisterComponentType<pragma::ActionInputControllerComponent>("action_input_controller");
	componentManager.RegisterComponentType<pragma::InputMovementControllerComponent>("input_movement_controller");

	pragma::BaseEntityComponent::RegisterEvents(componentManager, [&componentManager](const std::string &evName, pragma::ComponentEventInfo::Type type) -> pragma::ComponentEventId { return componentManager.RegisterEvent(evName, typeid(pragma::BaseEntityComponent), type); });
	BaseEntity::RegisterEvents(componentManager);
}
