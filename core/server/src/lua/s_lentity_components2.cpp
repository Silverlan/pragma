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
	auto defSPointConstraintBallSocket = luabind::class_<SPointConstraintBallSocketHandle,BaseEntityComponentHandle>("PointConstraintBallSocketComponent");
	Lua::register_base_point_constraint_ball_socket_component_methods<luabind::class_<SPointConstraintBallSocketHandle,BaseEntityComponentHandle>,SPointConstraintBallSocketHandle>(l,defSPointConstraintBallSocket);
	entsMod[defSPointConstraintBallSocket];

	auto defSPointConstraintConeTwist = luabind::class_<SPointConstraintConeTwistHandle,BaseEntityComponentHandle>("PointConstraintConeTwistComponent");
	Lua::register_base_point_constraint_cone_twist_component_methods<luabind::class_<SPointConstraintConeTwistHandle,BaseEntityComponentHandle>,SPointConstraintConeTwistHandle>(l,defSPointConstraintConeTwist);
	entsMod[defSPointConstraintConeTwist];

	auto defSPointConstraintDoF = luabind::class_<SPointConstraintDoFHandle,BaseEntityComponentHandle>("PointConstraintDoFComponent");
	Lua::register_base_point_constraint_dof_component_methods<luabind::class_<SPointConstraintDoFHandle,BaseEntityComponentHandle>,SPointConstraintDoFHandle>(l,defSPointConstraintDoF);
	entsMod[defSPointConstraintDoF];

	auto defSPointConstraintFixed = luabind::class_<SPointConstraintFixedHandle,BaseEntityComponentHandle>("PointConstraintFixedComponent");
	Lua::register_base_point_constraint_fixed_component_methods<luabind::class_<SPointConstraintFixedHandle,BaseEntityComponentHandle>,SPointConstraintFixedHandle>(l,defSPointConstraintFixed);
	entsMod[defSPointConstraintFixed];

	auto defSPointConstraintHinge = luabind::class_<SPointConstraintHingeHandle,BaseEntityComponentHandle>("PointConstraintHingeComponent");
	Lua::register_base_point_constraint_hinge_component_methods<luabind::class_<SPointConstraintHingeHandle,BaseEntityComponentHandle>,SPointConstraintHingeHandle>(l,defSPointConstraintHinge);
	entsMod[defSPointConstraintHinge];

	auto defSPointConstraintSlider = luabind::class_<SPointConstraintSliderHandle,BaseEntityComponentHandle>("PointConstraintSliderComponent");
	Lua::register_base_point_constraint_slider_component_methods<luabind::class_<SPointConstraintSliderHandle,BaseEntityComponentHandle>,SPointConstraintSliderHandle>(l,defSPointConstraintSlider);
	entsMod[defSPointConstraintSlider];

	auto defSPathNode = luabind::class_<SPathNodeHandle,BaseEntityComponentHandle>("PathNodeComponent");
	Lua::register_base_point_path_node_component_methods<luabind::class_<SPathNodeHandle,BaseEntityComponentHandle>,SPathNodeHandle>(l,defSPathNode);
	entsMod[defSPathNode];

	auto defSRenderTarget = luabind::class_<SRenderTargetHandle,BaseEntityComponentHandle>("RenderTargetComponent");
	Lua::register_base_point_render_target_component_methods<luabind::class_<SRenderTargetHandle,BaseEntityComponentHandle>,SRenderTargetHandle>(l,defSRenderTarget);
	entsMod[defSRenderTarget];

	auto defSPointTarget = luabind::class_<SPointTargetHandle,BaseEntityComponentHandle>("PointTargetComponent");
	Lua::register_base_point_target_component_methods<luabind::class_<SPointTargetHandle,BaseEntityComponentHandle>,SPointTargetHandle>(l,defSPointTarget);
	entsMod[defSPointTarget];

	auto defSProp = luabind::class_<PropHandle,BaseEntityComponentHandle>("PropComponent");
	Lua::register_base_prop_component_methods<luabind::class_<PropHandle,BaseEntityComponentHandle>,PropHandle>(l,defSProp);
	entsMod[defSProp];

	auto defSPropDynamic = luabind::class_<SPropDynamicHandle,BaseEntityComponentHandle>("PropDynamicComponent");
	Lua::register_base_prop_dynamic_component_methods<luabind::class_<SPropDynamicHandle,BaseEntityComponentHandle>,SPropDynamicHandle>(l,defSPropDynamic);
	entsMod[defSPropDynamic];

	auto defSPropPhysics = luabind::class_<SPropPhysicsHandle,BaseEntityComponentHandle>("PropPhysicsComponent");
	Lua::register_base_prop_physics_component_methods<luabind::class_<SPropPhysicsHandle,BaseEntityComponentHandle>,SPropPhysicsHandle>(l,defSPropPhysics);
	entsMod[defSPropPhysics];

	auto defSTriggerGravity = luabind::class_<STriggerGravityHandle,BaseEntityComponentHandle>("TriggerGravityComponent");
	Lua::register_base_trigger_gravity_component_methods<luabind::class_<STriggerGravityHandle,BaseEntityComponentHandle>,STriggerGravityHandle>(l,defSTriggerGravity);
	entsMod[defSTriggerGravity];

	auto defSTriggerHurt = luabind::class_<STriggerHurtHandle,BaseEntityComponentHandle>("TriggerHurtComponent");
	Lua::register_base_trigger_hurt_component_methods<luabind::class_<STriggerHurtHandle,BaseEntityComponentHandle>,STriggerHurtHandle>(l,defSTriggerHurt);
	entsMod[defSTriggerHurt];

	auto defSTriggerPush = luabind::class_<STriggerPushHandle,BaseEntityComponentHandle>("TriggerPushComponent");
	Lua::register_base_trigger_push_component_methods<luabind::class_<STriggerPushHandle,BaseEntityComponentHandle>,STriggerPushHandle>(l,defSTriggerPush);
	entsMod[defSTriggerPush];

	auto defSTriggerRemove = luabind::class_<STriggerRemoveHandle,BaseEntityComponentHandle>("TriggerRemoveComponent");
	Lua::register_base_trigger_remove_component_methods<luabind::class_<STriggerRemoveHandle,BaseEntityComponentHandle>,STriggerRemoveHandle>(l,defSTriggerRemove);
	entsMod[defSTriggerRemove];

	auto defSTriggerTeleport = luabind::class_<STriggerTeleportHandle,BaseEntityComponentHandle>("TriggerTeleportComponent");
	Lua::register_base_trigger_teleport_component_methods<luabind::class_<STriggerTeleportHandle,BaseEntityComponentHandle>,STriggerTeleportHandle>(l,defSTriggerTeleport);
	entsMod[defSTriggerTeleport];

	auto defSTouch = luabind::class_<STouchHandle,BaseEntityComponentHandle>("TouchComponent");
	Lua::register_base_touch_component_methods<luabind::class_<STouchHandle,BaseEntityComponentHandle>,STouchHandle>(l,defSTouch);
	entsMod[defSTouch];

	auto defSSkybox = luabind::class_<SSkyboxHandle,BaseEntityComponentHandle>("SkyboxComponent");
	Lua::register_base_skybox_component_methods<luabind::class_<SSkyboxHandle,BaseEntityComponentHandle>,SSkyboxHandle>(l,defSSkybox);
	entsMod[defSSkybox];

	auto defSWorld = luabind::class_<SWorldHandle,BaseEntityComponentHandle>("WorldComponent");
	Lua::register_base_world_component_methods<luabind::class_<SWorldHandle,BaseEntityComponentHandle>,SWorldHandle>(l,defSWorld);
	entsMod[defSWorld];

	auto defSFlashlight = luabind::class_<SFlashlightHandle,BaseEntityComponentHandle>("FlashlightComponent");
	Lua::register_base_flashlight_component_methods<luabind::class_<SFlashlightHandle,BaseEntityComponentHandle>,SFlashlightHandle>(l,defSFlashlight);
	entsMod[defSFlashlight];

	auto defSIo = luabind::class_<SIOHandle,BaseEntityComponentHandle>("IOComponent");
	Lua::register_base_io_component_methods<luabind::class_<SIOHandle,BaseEntityComponentHandle>,SIOHandle>(l,defSIo);
	entsMod[defSIo];

	auto defSModel = luabind::class_<SModelHandle,BaseEntityComponentHandle>("ModelComponent");
	Lua::register_base_model_component_methods<luabind::class_<SModelHandle,BaseEntityComponentHandle>,SModelHandle>(l,defSModel);
	entsMod[defSModel];

	auto defSAnimated = luabind::class_<SAnimatedHandle,BaseEntityComponentHandle>("AnimatedComponent");
	Lua::register_base_animated_component_methods<luabind::class_<SAnimatedHandle,BaseEntityComponentHandle>,SAnimatedHandle>(l,defSAnimated);
	entsMod[defSAnimated];

	auto defSTimeScale = luabind::class_<STimeScaleHandle,BaseEntityComponentHandle>("TimeScaleComponent");
	Lua::register_base_time_scale_component_methods<luabind::class_<STimeScaleHandle,BaseEntityComponentHandle>,STimeScaleHandle>(l,defSTimeScale);
	entsMod[defSTimeScale];

	auto defSAttachable = luabind::class_<SAttachableHandle,BaseEntityComponentHandle>("AttachableComponent");
	Lua::register_base_attachable_component_methods<luabind::class_<SAttachableHandle,BaseEntityComponentHandle>,SAttachableHandle>(l,defSAttachable);
	entsMod[defSAttachable];

	auto defSParent = luabind::class_<SParentHandle,BaseEntityComponentHandle>("ParentComponent");
	Lua::register_base_parent_component_methods<luabind::class_<SParentHandle,BaseEntityComponentHandle>,SParentHandle>(l,defSParent);
	entsMod[defSParent];

	auto defSOwnable = luabind::class_<SOwnableHandle,BaseEntityComponentHandle>("OwnableComponent");
	Lua::register_base_ownable_component_methods<luabind::class_<SOwnableHandle,BaseEntityComponentHandle>,SOwnableHandle>(l,defSOwnable);
	entsMod[defSOwnable];

	auto defSDebugText = luabind::class_<SDebugTextHandle,BaseEntityComponentHandle>("DebugTextComponent");
	Lua::register_base_debug_text_component_methods<luabind::class_<SDebugTextHandle,BaseEntityComponentHandle>,SDebugTextHandle>(l,defSDebugText);
	entsMod[defSDebugText];

	auto defSDebugPoint = luabind::class_<SDebugPointHandle,BaseEntityComponentHandle>("DebugPointComponent");
	Lua::register_base_debug_point_component_methods<luabind::class_<SDebugPointHandle,BaseEntityComponentHandle>,SDebugPointHandle>(l,defSDebugPoint);
	entsMod[defSDebugPoint];

	auto defSDebugLine = luabind::class_<SDebugLineHandle,BaseEntityComponentHandle>("DebugLineComponent");
	Lua::register_base_debug_line_component_methods<luabind::class_<SDebugLineHandle,BaseEntityComponentHandle>,SDebugLineHandle>(l,defSDebugLine);
	entsMod[defSDebugLine];

	auto defSDebugBox = luabind::class_<SDebugBoxHandle,BaseEntityComponentHandle>("DebugBoxComponent");
	Lua::register_base_debug_box_component_methods<luabind::class_<SDebugBoxHandle,BaseEntityComponentHandle>,SDebugBoxHandle>(l,defSDebugBox);
	entsMod[defSDebugBox];

	auto defSDebugSphere = luabind::class_<SDebugSphereHandle,BaseEntityComponentHandle>("DebugSphereComponent");
	Lua::register_base_debug_sphere_component_methods<luabind::class_<SDebugSphereHandle,BaseEntityComponentHandle>,SDebugSphereHandle>(l,defSDebugSphere);
	entsMod[defSDebugSphere];

	auto defSDebugCone = luabind::class_<SDebugConeHandle,BaseEntityComponentHandle>("DebugConeComponent");
	Lua::register_base_debug_cone_component_methods<luabind::class_<SDebugConeHandle,BaseEntityComponentHandle>,SDebugConeHandle>(l,defSDebugCone);
	entsMod[defSDebugCone];

	auto defSDebugCylinder = luabind::class_<SDebugCylinderHandle,BaseEntityComponentHandle>("DebugCylinderComponent");
	Lua::register_base_debug_cylinder_component_methods<luabind::class_<SDebugCylinderHandle,BaseEntityComponentHandle>,SDebugCylinderHandle>(l,defSDebugCylinder);
	entsMod[defSDebugCylinder];

	auto defSDebugPlane = luabind::class_<SDebugPlaneHandle,BaseEntityComponentHandle>("DebugPlaneComponent");
	Lua::register_base_debug_plane_component_methods<luabind::class_<SDebugPlaneHandle,BaseEntityComponentHandle>,SDebugPlaneHandle>(l,defSDebugPlane);
	entsMod[defSDebugPlane];

	auto defSPointAtTarget = luabind::class_<SPointAtTargetHandle,BaseEntityComponentHandle>("PointAtTargetComponent");
	Lua::register_base_point_at_target_component_methods<luabind::class_<SPointAtTargetHandle,BaseEntityComponentHandle>,SPointAtTargetHandle>(l,defSPointAtTarget);
	entsMod[defSPointAtTarget];

	auto defSInfoLandmark = luabind::class_<SInfoLandmarkHandle,BaseEntityComponentHandle>("InfoLandmarkComponent");
	//Lua::register_base_info_landmark_component_methods<luabind::class_<SInfoLandmarkHandle,BaseEntityComponentHandle>,SInfoLandmarkHandle>(l,defSInfoLandmark);
	entsMod[defSInfoLandmark];

	auto defSGeneric = luabind::class_<SGenericHandle,BaseEntityComponentHandle>("GenericComponent");
	//Lua::register_base_generic_component_methods<luabind::class_<SGenericHandle,BaseEntityComponentHandle>,SGenericHandle>(l,defSGeneric);
	entsMod[defSGeneric];
}
