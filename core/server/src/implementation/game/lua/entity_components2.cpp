// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/lua/ostream_operator_alias.hpp"

// --template-include-location

module pragma.server;
import :game;

import :entities.components;

void RegisterLuaEntityComponents2_sv(lua::State *l, luabind::module_ &entsMod)
{
	auto defSPointConstraintBallSocket = pragma::LuaCore::create_entity_component_class<pragma::SPointConstraintBallSocketComponent, pragma::BasePointConstraintBallSocketComponent>("PointConstraintBallSocketComponent");
	entsMod[defSPointConstraintBallSocket];

	auto defSPointConstraintConeTwist = pragma::LuaCore::create_entity_component_class<pragma::SPointConstraintConeTwistComponent, pragma::BasePointConstraintConeTwistComponent>("PointConstraintConeTwistComponent");
	entsMod[defSPointConstraintConeTwist];

	auto defSPointConstraintDoF = pragma::LuaCore::create_entity_component_class<pragma::SPointConstraintDoFComponent, pragma::BasePointConstraintDoFComponent>("PointConstraintDoFComponent");
	entsMod[defSPointConstraintDoF];

	auto defSPointConstraintFixed = pragma::LuaCore::create_entity_component_class<pragma::SPointConstraintFixedComponent, pragma::BasePointConstraintFixedComponent>("PointConstraintFixedComponent");
	entsMod[defSPointConstraintFixed];

	auto defSPointConstraintHinge = pragma::LuaCore::create_entity_component_class<pragma::SPointConstraintHingeComponent, pragma::BasePointConstraintHingeComponent>("PointConstraintHingeComponent");
	entsMod[defSPointConstraintHinge];

	auto defSPointConstraintSlider = pragma::LuaCore::create_entity_component_class<pragma::SPointConstraintSliderComponent, pragma::BasePointConstraintSliderComponent>("PointConstraintSliderComponent");
	entsMod[defSPointConstraintSlider];

	auto defSPathNode = pragma::LuaCore::create_entity_component_class<pragma::SPathNodeComponent, pragma::BasePointPathNodeComponent>("PathNodeComponent");
	entsMod[defSPathNode];

	// auto defSRenderTarget = pragma::LuaCore::create_entity_component_class<pragma::SRenderTargetComponent,pragma::BasePointRenderTargetComponent>("RenderTargetComponent");
	// entsMod[defSRenderTarget];

	auto defSPointTarget = pragma::LuaCore::create_entity_component_class<pragma::SPointTargetComponent, pragma::BasePointTargetComponent>("PointTargetComponent");
	entsMod[defSPointTarget];

	auto defSProp = pragma::LuaCore::create_entity_component_class<pragma::PropComponent, pragma::BasePropComponent>("PropComponent");
	entsMod[defSProp];

	auto defSPropDynamic = pragma::LuaCore::create_entity_component_class<pragma::SPropDynamicComponent, pragma::BasePropDynamicComponent>("PropDynamicComponent");
	entsMod[defSPropDynamic];

	auto defSPropPhysics = pragma::LuaCore::create_entity_component_class<pragma::SPropPhysicsComponent, pragma::BasePropPhysicsComponent>("PropPhysicsComponent");
	entsMod[defSPropPhysics];

	auto defSTriggerGravity = pragma::LuaCore::create_entity_component_class<pragma::STriggerGravityComponent, pragma::BaseEntityTriggerGravityComponent>("TriggerGravityComponent");
	entsMod[defSTriggerGravity];

	auto defSTriggerHurt = pragma::LuaCore::create_entity_component_class<pragma::STriggerHurtComponent, pragma::BaseTriggerHurtComponent>("TriggerHurtComponent");
	entsMod[defSTriggerHurt];

	auto defSTriggerPush = pragma::LuaCore::create_entity_component_class<pragma::STriggerPushComponent, pragma::BaseTriggerPushComponent>("TriggerPushComponent");
	entsMod[defSTriggerPush];

	auto defSTriggerRemove = pragma::LuaCore::create_entity_component_class<pragma::STriggerRemoveComponent, pragma::BaseTriggerRemoveComponent>("TriggerRemoveComponent");
	entsMod[defSTriggerRemove];

	auto defSTriggerTeleport = pragma::LuaCore::create_entity_component_class<pragma::STriggerTeleportComponent, pragma::BaseTriggerTeleportComponent>("TriggerTeleportComponent");
	entsMod[defSTriggerTeleport];

	auto defSTouch = pragma::LuaCore::create_entity_component_class<pragma::STouchComponent, pragma::BaseTouchComponent>("TouchComponent");
	entsMod[defSTouch];

	auto defSSkybox = pragma::LuaCore::create_entity_component_class<pragma::SSkyboxComponent, pragma::BaseSkyboxComponent>("SkyboxComponent");
	entsMod[defSSkybox];

	auto defSWorld = pragma::LuaCore::create_entity_component_class<pragma::SWorldComponent, pragma::BaseWorldComponent>("WorldComponent");
	entsMod[defSWorld];

	auto defSFlashlight = pragma::LuaCore::create_entity_component_class<pragma::SFlashlightComponent, pragma::BaseFlashlightComponent>("FlashlightComponent");
	entsMod[defSFlashlight];

	auto defSIo = pragma::LuaCore::create_entity_component_class<pragma::SIOComponent, pragma::BaseIOComponent>("IOComponent");
	entsMod[defSIo];

	auto defSModel = pragma::LuaCore::create_entity_component_class<pragma::SModelComponent, pragma::BaseModelComponent>("ModelComponent");
	entsMod[defSModel];

	auto defSAnimated = pragma::LuaCore::create_entity_component_class<pragma::SAnimatedComponent, pragma::BaseAnimatedComponent>("AnimatedComponent");
	entsMod[defSAnimated];

	auto defSTimeScale = pragma::LuaCore::create_entity_component_class<pragma::STimeScaleComponent, pragma::BaseTimeScaleComponent>("TimeScaleComponent");
	entsMod[defSTimeScale];

	auto defSAttachable = pragma::LuaCore::create_entity_component_class<pragma::SAttachmentComponent, pragma::BaseAttachmentComponent>("AttachmentComponent");
	entsMod[defSAttachable];

	auto defSChild = pragma::LuaCore::create_entity_component_class<pragma::SChildComponent, pragma::BaseChildComponent>("ChildComponent");
	entsMod[defSChild];

	auto defSOwnable = pragma::LuaCore::create_entity_component_class<pragma::SOwnableComponent, pragma::BaseOwnableComponent>("OwnableComponent");
	entsMod[defSOwnable];

	auto defSDebugText = pragma::LuaCore::create_entity_component_class<pragma::SDebugTextComponent, pragma::BaseDebugTextComponent>("DebugTextComponent");
	entsMod[defSDebugText];

	auto defSDebugPoint = pragma::LuaCore::create_entity_component_class<pragma::SDebugPointComponent, pragma::BaseDebugPointComponent>("DebugPointComponent");
	entsMod[defSDebugPoint];

	auto defSDebugLine = pragma::LuaCore::create_entity_component_class<pragma::SDebugLineComponent, pragma::BaseDebugLineComponent>("DebugLineComponent");
	entsMod[defSDebugLine];

	auto defSDebugBox = pragma::LuaCore::create_entity_component_class<pragma::SDebugBoxComponent, pragma::BaseDebugBoxComponent>("DebugBoxComponent");
	entsMod[defSDebugBox];

	auto defSDebugSphere = pragma::LuaCore::create_entity_component_class<pragma::SDebugSphereComponent, pragma::BaseDebugSphereComponent>("DebugSphereComponent");
	entsMod[defSDebugSphere];

	auto defSDebugCone = pragma::LuaCore::create_entity_component_class<pragma::SDebugConeComponent, pragma::BaseDebugConeComponent>("DebugConeComponent");
	entsMod[defSDebugCone];

	auto defSDebugCylinder = pragma::LuaCore::create_entity_component_class<pragma::SDebugCylinderComponent, pragma::BaseDebugCylinderComponent>("DebugCylinderComponent");
	entsMod[defSDebugCylinder];

	auto defSDebugPlane = pragma::LuaCore::create_entity_component_class<pragma::SDebugPlaneComponent, pragma::BaseDebugPlaneComponent>("DebugPlaneComponent");
	entsMod[defSDebugPlane];

	auto defSPointAtTarget = pragma::LuaCore::create_entity_component_class<pragma::SPointAtTargetComponent, pragma::BasePointAtTargetComponent>("PointAtTargetComponent");
	entsMod[defSPointAtTarget];

	auto defSInfoLandmark = pragma::LuaCore::create_entity_component_class<pragma::SInfoLandmarkComponent, pragma::BaseInfoLandmarkComponent>("InfoLandmarkComponent");
	entsMod[defSInfoLandmark];

	auto defSGeneric = pragma::LuaCore::create_entity_component_class<pragma::SGenericComponent, pragma::BaseGenericComponent>("GenericComponent");
	entsMod[defSGeneric];

	auto defLiquidControl = pragma::LuaCore::create_entity_component_class<pragma::SLiquidControlComponent, pragma::BaseLiquidControlComponent>("LiquidControlComponent");
	entsMod[defLiquidControl];

	auto defLiquidSurfaceSimulation = pragma::LuaCore::create_entity_component_class<pragma::SLiquidSurfaceSimulationComponent, pragma::BaseLiquidSurfaceSimulationComponent>("LiquidSurfaceSimulationComponent");
	entsMod[defLiquidSurfaceSimulation];

	pragma::SGame::Get()->GetEntityComponentManager().RegisterLuaBindings(l, entsMod);

	// --template-component-register-location
}
