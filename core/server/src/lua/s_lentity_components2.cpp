/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/s_lentity_components.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/entities/point/constraints/s_point_constraint_ballsocket.h"
#include "pragma/entities/point/constraints/s_point_constraint_dof.h"
#include "pragma/entities/point/constraints/s_point_constraint_conetwist.h"
#include "pragma/entities/point/constraints/s_point_constraint_fixed.h"
#include "pragma/entities/point/constraints/s_point_constraint_hinge.h"
#include "pragma/entities/point/constraints/s_point_constraint_slider.h"
#include "pragma/entities/components/liquid/s_liquid_control_component.hpp"
#include "pragma/entities/components/liquid/s_liquid_surface_simulation_component.hpp"
// --template-include-location
#include "pragma/entities/point/s_point_path_node.h"
#include "pragma/entities/point/s_point_rendertarget.h"
#include "pragma/entities/point/s_point_target.h"
#include "pragma/entities/prop/s_prop_base.hpp"
#include "pragma/entities/prop/s_prop_dynamic.hpp"
#include "pragma/entities/prop/s_prop_physics.h"
#include "pragma/entities/trigger/s_trigger_gravity.h"
#include "pragma/entities/trigger/s_trigger_hurt.h"
#include "pragma/entities/trigger/s_trigger_push.h"
#include "pragma/entities/trigger/s_trigger_remove.h"
#include "pragma/entities/trigger/s_trigger_teleport.h"
#include "pragma/entities/trigger/s_trigger_touch.h"
#include "pragma/entities/s_skybox.h"
#include "pragma/entities/s_flashlight.h"
#include "pragma/entities/components/s_model_component.hpp"
#include "pragma/entities/components/s_animated_component.hpp"
#include "pragma/entities/components/s_io_component.hpp"
#include "pragma/entities/components/s_time_scale_component.hpp"
#include "pragma/entities/components/s_attachment_component.hpp"
#include "pragma/entities/components/s_child_component.hpp"
#include "pragma/entities/components/s_generic_component.hpp"
#include "pragma/entities/components/s_point_at_target_component.hpp"
#include "pragma/entities/components/s_ownable_component.hpp"
#include "pragma/entities/components/s_debug_component.hpp"
#include "pragma/entities/info/s_info_landmark.hpp"
#include <pragma/physics/raytraces.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/classes/ldef_quaternion.h>
#include <pragma/lua/classes/ldef_angle.h>
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lentity_components.hpp>
#include <pragma/physics/movetypes.h>
#include <pragma/lua/lua_call.hpp>
#include <sharedutils/netpacket.hpp>

#include <pragma/lua/ostream_operator_alias.hpp>

#ifdef __linux__
DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(pragma, BaseEntityComponent);
#endif

extern DLLSERVER SGame *s_game;

void RegisterLuaEntityComponents2_sv(lua_State *l, luabind::module_ &entsMod)
{
	auto defSPointConstraintBallSocket = pragma::lua::create_entity_component_class<pragma::SPointConstraintBallSocketComponent, pragma::BasePointConstraintBallSocketComponent>("PointConstraintBallSocketComponent");
	entsMod[defSPointConstraintBallSocket];

	auto defSPointConstraintConeTwist = pragma::lua::create_entity_component_class<pragma::SPointConstraintConeTwistComponent, pragma::BasePointConstraintConeTwistComponent>("PointConstraintConeTwistComponent");
	entsMod[defSPointConstraintConeTwist];

	auto defSPointConstraintDoF = pragma::lua::create_entity_component_class<pragma::SPointConstraintDoFComponent, pragma::BasePointConstraintDoFComponent>("PointConstraintDoFComponent");
	entsMod[defSPointConstraintDoF];

	auto defSPointConstraintFixed = pragma::lua::create_entity_component_class<pragma::SPointConstraintFixedComponent, pragma::BasePointConstraintFixedComponent>("PointConstraintFixedComponent");
	entsMod[defSPointConstraintFixed];

	auto defSPointConstraintHinge = pragma::lua::create_entity_component_class<pragma::SPointConstraintHingeComponent, pragma::BasePointConstraintHingeComponent>("PointConstraintHingeComponent");
	entsMod[defSPointConstraintHinge];

	auto defSPointConstraintSlider = pragma::lua::create_entity_component_class<pragma::SPointConstraintSliderComponent, pragma::BasePointConstraintSliderComponent>("PointConstraintSliderComponent");
	entsMod[defSPointConstraintSlider];

	auto defSPathNode = pragma::lua::create_entity_component_class<pragma::SPathNodeComponent, pragma::BasePointPathNodeComponent>("PathNodeComponent");
	entsMod[defSPathNode];

	// auto defSRenderTarget = pragma::lua::create_entity_component_class<pragma::SRenderTargetComponent,pragma::BasePointRenderTargetComponent>("RenderTargetComponent");
	// entsMod[defSRenderTarget];

	auto defSPointTarget = pragma::lua::create_entity_component_class<pragma::SPointTargetComponent, pragma::BasePointTargetComponent>("PointTargetComponent");
	entsMod[defSPointTarget];

	auto defSProp = pragma::lua::create_entity_component_class<pragma::PropComponent, pragma::BasePropComponent>("PropComponent");
	entsMod[defSProp];

	auto defSPropDynamic = pragma::lua::create_entity_component_class<pragma::SPropDynamicComponent, pragma::BasePropDynamicComponent>("PropDynamicComponent");
	entsMod[defSPropDynamic];

	auto defSPropPhysics = pragma::lua::create_entity_component_class<pragma::SPropPhysicsComponent, pragma::BasePropPhysicsComponent>("PropPhysicsComponent");
	entsMod[defSPropPhysics];

	auto defSTriggerGravity = pragma::lua::create_entity_component_class<pragma::STriggerGravityComponent, pragma::BaseEntityTriggerGravityComponent>("TriggerGravityComponent");
	entsMod[defSTriggerGravity];

	auto defSTriggerHurt = pragma::lua::create_entity_component_class<pragma::STriggerHurtComponent, pragma::BaseTriggerHurtComponent>("TriggerHurtComponent");
	entsMod[defSTriggerHurt];

	auto defSTriggerPush = pragma::lua::create_entity_component_class<pragma::STriggerPushComponent, pragma::BaseTriggerPushComponent>("TriggerPushComponent");
	entsMod[defSTriggerPush];

	auto defSTriggerRemove = pragma::lua::create_entity_component_class<pragma::STriggerRemoveComponent, pragma::BaseTriggerRemoveComponent>("TriggerRemoveComponent");
	entsMod[defSTriggerRemove];

	auto defSTriggerTeleport = pragma::lua::create_entity_component_class<pragma::STriggerTeleportComponent, pragma::BaseTriggerTeleportComponent>("TriggerTeleportComponent");
	entsMod[defSTriggerTeleport];

	auto defSTouch = pragma::lua::create_entity_component_class<pragma::STouchComponent, pragma::BaseTouchComponent>("TouchComponent");
	entsMod[defSTouch];

	auto defSSkybox = pragma::lua::create_entity_component_class<pragma::SSkyboxComponent, pragma::BaseSkyboxComponent>("SkyboxComponent");
	entsMod[defSSkybox];

	auto defSWorld = pragma::lua::create_entity_component_class<pragma::SWorldComponent, pragma::BaseWorldComponent>("WorldComponent");
	entsMod[defSWorld];

	auto defSFlashlight = pragma::lua::create_entity_component_class<pragma::SFlashlightComponent, pragma::BaseFlashlightComponent>("FlashlightComponent");
	entsMod[defSFlashlight];

	auto defSIo = pragma::lua::create_entity_component_class<pragma::SIOComponent, pragma::BaseIOComponent>("IOComponent");
	entsMod[defSIo];

	auto defSModel = pragma::lua::create_entity_component_class<pragma::SModelComponent, pragma::BaseModelComponent>("ModelComponent");
	entsMod[defSModel];

	auto defSAnimated = pragma::lua::create_entity_component_class<pragma::SAnimatedComponent, pragma::BaseAnimatedComponent>("AnimatedComponent");
	entsMod[defSAnimated];

	auto defSTimeScale = pragma::lua::create_entity_component_class<pragma::STimeScaleComponent, pragma::BaseTimeScaleComponent>("TimeScaleComponent");
	entsMod[defSTimeScale];

	auto defSAttachable = pragma::lua::create_entity_component_class<pragma::SAttachmentComponent, pragma::BaseAttachmentComponent>("AttachmentComponent");
	entsMod[defSAttachable];

	auto defSChild = pragma::lua::create_entity_component_class<pragma::SChildComponent, pragma::BaseChildComponent>("ChildComponent");
	entsMod[defSChild];

	auto defSOwnable = pragma::lua::create_entity_component_class<pragma::SOwnableComponent, pragma::BaseOwnableComponent>("OwnableComponent");
	entsMod[defSOwnable];

	auto defSDebugText = pragma::lua::create_entity_component_class<pragma::SDebugTextComponent, pragma::BaseDebugTextComponent>("DebugTextComponent");
	entsMod[defSDebugText];

	auto defSDebugPoint = pragma::lua::create_entity_component_class<pragma::SDebugPointComponent, pragma::BaseDebugPointComponent>("DebugPointComponent");
	entsMod[defSDebugPoint];

	auto defSDebugLine = pragma::lua::create_entity_component_class<pragma::SDebugLineComponent, pragma::BaseDebugLineComponent>("DebugLineComponent");
	entsMod[defSDebugLine];

	auto defSDebugBox = pragma::lua::create_entity_component_class<pragma::SDebugBoxComponent, pragma::BaseDebugBoxComponent>("DebugBoxComponent");
	entsMod[defSDebugBox];

	auto defSDebugSphere = pragma::lua::create_entity_component_class<pragma::SDebugSphereComponent, pragma::BaseDebugSphereComponent>("DebugSphereComponent");
	entsMod[defSDebugSphere];

	auto defSDebugCone = pragma::lua::create_entity_component_class<pragma::SDebugConeComponent, pragma::BaseDebugConeComponent>("DebugConeComponent");
	entsMod[defSDebugCone];

	auto defSDebugCylinder = pragma::lua::create_entity_component_class<pragma::SDebugCylinderComponent, pragma::BaseDebugCylinderComponent>("DebugCylinderComponent");
	entsMod[defSDebugCylinder];

	auto defSDebugPlane = pragma::lua::create_entity_component_class<pragma::SDebugPlaneComponent, pragma::BaseDebugPlaneComponent>("DebugPlaneComponent");
	entsMod[defSDebugPlane];

	auto defSPointAtTarget = pragma::lua::create_entity_component_class<pragma::SPointAtTargetComponent, pragma::BasePointAtTargetComponent>("PointAtTargetComponent");
	entsMod[defSPointAtTarget];

	auto defSInfoLandmark = pragma::lua::create_entity_component_class<pragma::SInfoLandmarkComponent, pragma::BaseInfoLandmarkComponent>("InfoLandmarkComponent");
	entsMod[defSInfoLandmark];

	auto defSGeneric = pragma::lua::create_entity_component_class<pragma::SGenericComponent, pragma::BaseGenericComponent>("GenericComponent");
	entsMod[defSGeneric];

	auto defLiquidControl = pragma::lua::create_entity_component_class<pragma::SLiquidControlComponent, pragma::BaseLiquidControlComponent>("LiquidControlComponent");
	entsMod[defLiquidControl];

	auto defLiquidSurfaceSimulation = pragma::lua::create_entity_component_class<pragma::SLiquidSurfaceSimulationComponent, pragma::BaseLiquidSurfaceSimulationComponent>("LiquidSurfaceSimulationComponent");
	entsMod[defLiquidSurfaceSimulation];

	s_game->GetEntityComponentManager().RegisterLuaBindings(l, entsMod);

	// --template-component-register-location
}
