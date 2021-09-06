/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/s_lentity_components.hpp"
#include "pragma/networking/recipient_filter.hpp"
#include "pragma/entities/point/constraints/s_point_constraint_ballsocket.h"
#include "pragma/entities/point/constraints/s_point_constraint_dof.h"
#include "pragma/entities/point/constraints/s_point_constraint_conetwist.h"
#include "pragma/entities/point/constraints/s_point_constraint_fixed.h"
#include "pragma/entities/point/constraints/s_point_constraint_hinge.h"
#include "pragma/entities/point/constraints/s_point_constraint_slider.h"
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
#include "pragma/entities/components/s_attachable_component.hpp"
#include "pragma/entities/components/s_parent_component.hpp"
#include "pragma/entities/components/s_generic_component.hpp"
#include "pragma/entities/components/s_point_at_target_component.hpp"
#include "pragma/entities/components/s_ownable_component.hpp"
#include "pragma/entities/components/s_debug_component.hpp"
#include "pragma/entities/info/s_info_landmark.hpp"
#include <pragma/physics/raytraces.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/lua/classes/lproperty.hpp>
#include <pragma/lua/classes/ldef_quaternion.h>
#include <pragma/lua/classes/ldef_angle.h>
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lentity_components.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/physics/movetypes.h>
#include <pragma/lua/lua_call.hpp>
#include <sharedutils/netpacket.hpp>

void RegisterLuaEntityComponents2(lua_State *l,luabind::module_ &entsMod)
{
	auto defSPointConstraintBallSocket = luabind::class_<pragma::SPointConstraintBallSocketComponent,pragma::BasePointConstraintBallSocketComponent>("PointConstraintBallSocketComponent");
	entsMod[defSPointConstraintBallSocket];

	auto defSPointConstraintConeTwist = luabind::class_<pragma::SPointConstraintConeTwistComponent,pragma::BasePointConstraintConeTwistComponent>("PointConstraintConeTwistComponent");
	entsMod[defSPointConstraintConeTwist];

	auto defSPointConstraintDoF = luabind::class_<pragma::SPointConstraintDoFComponent,pragma::BasePointConstraintDoFComponent>("PointConstraintDoFComponent");
	entsMod[defSPointConstraintDoF];

	auto defSPointConstraintFixed = luabind::class_<pragma::SPointConstraintFixedComponent,pragma::BasePointConstraintFixedComponent>("PointConstraintFixedComponent");
	entsMod[defSPointConstraintFixed];

	auto defSPointConstraintHinge = luabind::class_<pragma::SPointConstraintHingeComponent,pragma::BasePointConstraintHingeComponent>("PointConstraintHingeComponent");
	entsMod[defSPointConstraintHinge];

	auto defSPointConstraintSlider = luabind::class_<pragma::SPointConstraintSliderComponent,pragma::BasePointConstraintSliderComponent>("PointConstraintSliderComponent");
	entsMod[defSPointConstraintSlider];

	auto defSPathNode = luabind::class_<pragma::SPathNodeComponent,pragma::BasePointPathNodeComponent>("PathNodeComponent");
	entsMod[defSPathNode];

	auto defSRenderTarget = luabind::class_<pragma::SRenderTargetComponent,pragma::BasePointRenderTargetComponent>("RenderTargetComponent");
	entsMod[defSRenderTarget];

	auto defSPointTarget = luabind::class_<pragma::SPointTargetComponent,pragma::BasePointTargetComponent>("PointTargetComponent");
	entsMod[defSPointTarget];

	auto defSProp = luabind::class_<pragma::PropComponent,pragma::BasePropComponent>("PropComponent");
	entsMod[defSProp];

	auto defSPropDynamic = luabind::class_<pragma::SPropDynamicComponent,pragma::BasePropDynamicComponent>("PropDynamicComponent");
	entsMod[defSPropDynamic];

	auto defSPropPhysics = luabind::class_<pragma::SPropPhysicsComponent,pragma::BasePropPhysicsComponent>("PropPhysicsComponent");
	entsMod[defSPropPhysics];

	auto defSTriggerGravity = luabind::class_<pragma::STriggerGravityComponent,pragma::BaseEntityTriggerGravityComponent>("TriggerGravityComponent");
	entsMod[defSTriggerGravity];

	auto defSTriggerHurt = luabind::class_<pragma::STriggerHurtComponent,pragma::BaseTriggerHurtComponent>("TriggerHurtComponent");
	entsMod[defSTriggerHurt];

	auto defSTriggerPush = luabind::class_<pragma::STriggerPushComponent,pragma::BaseTriggerPushComponent>("TriggerPushComponent");
	entsMod[defSTriggerPush];

	auto defSTriggerRemove = luabind::class_<pragma::STriggerRemoveComponent,pragma::BaseTriggerRemoveComponent>("TriggerRemoveComponent");
	entsMod[defSTriggerRemove];

	auto defSTriggerTeleport = luabind::class_<pragma::STriggerTeleportComponent,pragma::BaseTriggerTeleportComponent>("TriggerTeleportComponent");
	entsMod[defSTriggerTeleport];

	auto defSTouch = luabind::class_<pragma::STouchComponent,pragma::BaseTouchComponent>("TouchComponent");
	entsMod[defSTouch];

	auto defSSkybox = luabind::class_<pragma::SSkyboxComponent,pragma::BaseSkyboxComponent>("SkyboxComponent");
	entsMod[defSSkybox];

	auto defSWorld = luabind::class_<pragma::SWorldComponent,pragma::BaseWorldComponent>("WorldComponent");
	entsMod[defSWorld];

	auto defSFlashlight = luabind::class_<pragma::SFlashlightComponent,pragma::BaseFlashlightComponent>("FlashlightComponent");
	entsMod[defSFlashlight];

	auto defSIo = luabind::class_<pragma::SIOComponent,pragma::BaseIOComponent>("IOComponent");
	entsMod[defSIo];

	auto defSModel = luabind::class_<pragma::SModelComponent,pragma::BaseModelComponent>("ModelComponent");
	entsMod[defSModel];

	auto defSAnimated = luabind::class_<pragma::SAnimatedComponent,pragma::BaseAnimatedComponent>("AnimatedComponent");
	entsMod[defSAnimated];

	auto defSTimeScale = luabind::class_<pragma::STimeScaleComponent,pragma::BaseTimeScaleComponent>("TimeScaleComponent");
	entsMod[defSTimeScale];

	auto defSAttachable = luabind::class_<pragma::SAttachableComponent,pragma::BaseAttachableComponent>("AttachableComponent");
	entsMod[defSAttachable];

	auto defSParent = luabind::class_<pragma::SParentComponent,pragma::BaseParentComponent>("ParentComponent");
	entsMod[defSParent];

	auto defSOwnable = luabind::class_<pragma::SOwnableComponent,pragma::BaseOwnableComponent>("OwnableComponent");
	entsMod[defSOwnable];

	auto defSDebugText = luabind::class_<pragma::SDebugTextComponent,pragma::BaseDebugTextComponent>("DebugTextComponent");
	entsMod[defSDebugText];

	auto defSDebugPoint = luabind::class_<pragma::SDebugPointComponent,pragma::BaseDebugPointComponent>("DebugPointComponent");
	entsMod[defSDebugPoint];

	auto defSDebugLine = luabind::class_<pragma::SDebugLineComponent,pragma::BaseDebugLineComponent>("DebugLineComponent");
	entsMod[defSDebugLine];

	auto defSDebugBox = luabind::class_<pragma::SDebugBoxComponent,pragma::BaseDebugBoxComponent>("DebugBoxComponent");
	entsMod[defSDebugBox];

	auto defSDebugSphere = luabind::class_<pragma::SDebugSphereComponent,pragma::BaseDebugSphereComponent>("DebugSphereComponent");
	entsMod[defSDebugSphere];

	auto defSDebugCone = luabind::class_<pragma::SDebugConeComponent,pragma::BaseDebugConeComponent>("DebugConeComponent");
	entsMod[defSDebugCone];

	auto defSDebugCylinder = luabind::class_<pragma::SDebugCylinderComponent,pragma::BaseDebugCylinderComponent>("DebugCylinderComponent");
	entsMod[defSDebugCylinder];

	auto defSDebugPlane = luabind::class_<pragma::SDebugPlaneComponent,pragma::BaseDebugPlaneComponent>("DebugPlaneComponent");
	entsMod[defSDebugPlane];

	auto defSPointAtTarget = luabind::class_<pragma::SPointAtTargetComponent,pragma::BasePointAtTargetComponent>("PointAtTargetComponent");
	entsMod[defSPointAtTarget];

	auto defSInfoLandmark = luabind::class_<pragma::SInfoLandmarkComponent,pragma::BaseInfoLandmarkComponent>("InfoLandmarkComponent");
	entsMod[defSInfoLandmark];

	auto defSGeneric = luabind::class_<pragma::SGenericComponent,pragma::BaseGenericComponent>("GenericComponent");
	entsMod[defSGeneric];
}
