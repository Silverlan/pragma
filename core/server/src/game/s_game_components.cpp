/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/game/s_game.h"
#include "pragma/lua/s_lentity_handles.hpp"
#include "pragma/lua/s_lua_component.hpp"
#include "pragma/entities/components/s_ownable_component.hpp"
#include "pragma/entities/components/s_debug_component.hpp"
#include <pragma/lua/lentity_component_lua.hpp>
#include <pragma/game/game_entity_component.hpp>
#include <pragma/entities/entity_component_manager.hpp>
#include <luainterface.hpp>

void SGame::InitializeEntityComponents(pragma::EntityComponentManager &componentManager)
{
	Game::InitializeEntityComponents(componentManager);
	componentManager.RegisterComponentType<pragma::SAIComponent>("ai");
	componentManager.RegisterComponentType<pragma::SCharacterComponent>("character");
	componentManager.RegisterComponentType<pragma::SColorComponent>("color");
	componentManager.RegisterComponentType<pragma::SScoreComponent>("score");
	componentManager.RegisterComponentType<pragma::SFlammableComponent>("flammable");
	componentManager.RegisterComponentType<pragma::SHealthComponent>("health");
	componentManager.RegisterComponentType<pragma::SNameComponent>("name");
	componentManager.RegisterComponentType<pragma::SNetworkedComponent>("networked");
	componentManager.RegisterComponentType<pragma::SObservableComponent>("observable");
	componentManager.RegisterComponentType<pragma::SPhysicsComponent>("physics");
	componentManager.RegisterComponentType<pragma::SPlayerComponent>("player");
	componentManager.RegisterComponentType<pragma::SRadiusComponent>("radius");
	componentManager.RegisterComponentType<pragma::SRenderComponent>("render");
	componentManager.RegisterComponentType<pragma::SSoundEmitterComponent>("sound_emitter");
	componentManager.RegisterComponentType<pragma::SToggleComponent>("toggle");
	componentManager.RegisterComponentType<pragma::STransformComponent>("transform");
	componentManager.RegisterComponentType<pragma::SVehicleComponent>("vehicle");
	componentManager.RegisterComponentType<pragma::SWeaponComponent>("weapon");
	componentManager.RegisterComponentType<pragma::SWheelComponent>("wheel");
	componentManager.RegisterComponentType<pragma::SSoundDspComponent>("sound_dsp");
	componentManager.RegisterComponentType<pragma::SSoundDspChorusComponent>("sound_dsp_chorus");
	componentManager.RegisterComponentType<pragma::SSoundDspDistortionComponent>("sound_dsp_distortion");
	componentManager.RegisterComponentType<pragma::SSoundDspEAXReverbComponent>("sound_dsp_eax_reverb");
	componentManager.RegisterComponentType<pragma::SSoundDspEchoComponent>("sound_dsp_echo");
	componentManager.RegisterComponentType<pragma::SSoundDspEqualizerComponent>("sound_dsp_equalizer");
	componentManager.RegisterComponentType<pragma::SSoundDspFlangerComponent>("sound_dsp_flanger");
	componentManager.RegisterComponentType<pragma::SCameraComponent>("camera");
	componentManager.RegisterComponentType<pragma::SDecalComponent>("decal");
	componentManager.RegisterComponentType<pragma::SExplosionComponent>("explosion");
	componentManager.RegisterComponentType<pragma::SFireComponent>("fire");
	componentManager.RegisterComponentType<pragma::SFogControllerComponent>("fog_controller");
	componentManager.RegisterComponentType<pragma::SLightComponent>("light");
	componentManager.RegisterComponentType<pragma::SLightDirectionalComponent>("light_directional");
	componentManager.RegisterComponentType<pragma::SLightPointComponent>("light_point");
	componentManager.RegisterComponentType<pragma::SLightSpotComponent>("light_spot");
	componentManager.RegisterComponentType<pragma::SLightSpotVolComponent>("light_spot_volume");
	componentManager.RegisterComponentType<pragma::SMicrophoneComponent>("microphone");
	componentManager.RegisterComponentType<pragma::SParticleSystemComponent>("particle_system");
	componentManager.RegisterComponentType<pragma::SQuakeComponent>("quake");
	componentManager.RegisterComponentType<pragma::SSmokeTrailComponent>("smoke_trail");
	componentManager.RegisterComponentType<pragma::SSoundComponent>("sound");
	componentManager.RegisterComponentType<pragma::SSoundScapeComponent>("sound_scape");
	componentManager.RegisterComponentType<pragma::SSpriteComponent>("sprite");
	componentManager.RegisterComponentType<pragma::SEnvTimescaleComponent>("env_time_scale");
	componentManager.RegisterComponentType<pragma::SWindComponent>("wind");
	componentManager.RegisterComponentType<pragma::SFilterClassComponent>("filter_class");
	componentManager.RegisterComponentType<pragma::SFilterNameComponent>("filter_name");
	componentManager.RegisterComponentType<pragma::SBrushComponent>("brush");
	componentManager.RegisterComponentType<pragma::SKinematicComponent>("kinematic");
	componentManager.RegisterComponentType<pragma::SFuncPhysicsComponent>("func_physics");
	componentManager.RegisterComponentType<pragma::SFuncSoftPhysicsComponent>("func_soft_physics");
	componentManager.RegisterComponentType<pragma::SFuncPortalComponent>("func_portal");
	componentManager.RegisterComponentType<pragma::SWaterComponent>("water");
	componentManager.RegisterComponentType<pragma::SButtonComponent>("button");
	componentManager.RegisterComponentType<pragma::SPlayerSpawnComponent>("player_spawn");
	componentManager.RegisterComponentType<pragma::SLogicRelayComponent>("relay");
	componentManager.RegisterComponentType<pragma::SBotComponent>("bot");
	componentManager.RegisterComponentType<pragma::SPointConstraintBallSocketComponent>("constraint_ball_socket");
	componentManager.RegisterComponentType<pragma::SPointConstraintConeTwistComponent>("constraint_cone_twist");
	componentManager.RegisterComponentType<pragma::SPointConstraintDoFComponent>("constraint_dof");
	componentManager.RegisterComponentType<pragma::SPointConstraintFixedComponent>("constraint_fixed");
	componentManager.RegisterComponentType<pragma::SPointConstraintHingeComponent>("constraint_hinge");
	componentManager.RegisterComponentType<pragma::SPointConstraintSliderComponent>("constraint_slider");
	componentManager.RegisterComponentType<pragma::SPathNodeComponent>("path_node");
	componentManager.RegisterComponentType<pragma::SRenderTargetComponent>("render_target");
	componentManager.RegisterComponentType<pragma::SPointTargetComponent>("target");
	componentManager.RegisterComponentType<pragma::PropComponent>("prop");
	componentManager.RegisterComponentType<pragma::SPropDynamicComponent>("prop_dynamic");
	componentManager.RegisterComponentType<pragma::SPropPhysicsComponent>("prop_physics");
	componentManager.RegisterComponentType<pragma::STriggerGravityComponent>("trigger_gravity");
	componentManager.RegisterComponentType<pragma::STriggerHurtComponent>("trigger_hurt");
	componentManager.RegisterComponentType<pragma::STriggerPushComponent>("trigger_push");
	componentManager.RegisterComponentType<pragma::STriggerRemoveComponent>("trigger_remove");
	componentManager.RegisterComponentType<pragma::STriggerTeleportComponent>("trigger_teleport");
	componentManager.RegisterComponentType<pragma::STouchComponent>("touch");
	componentManager.RegisterComponentType<pragma::SSkyboxComponent>("skybox");
	componentManager.RegisterComponentType<pragma::SWorldComponent>("world");
	componentManager.RegisterComponentType<pragma::SFlashlightComponent>("flashlight");
	componentManager.RegisterComponentType<pragma::SShooterComponent>("shooter");
	componentManager.RegisterComponentType<pragma::SModelComponent>("model");
	componentManager.RegisterComponentType<pragma::SAnimatedComponent>("animated");
	componentManager.RegisterComponentType<pragma::SGenericComponent>("entity");
	componentManager.RegisterComponentType<pragma::SIOComponent>("io");
	componentManager.RegisterComponentType<pragma::STimeScaleComponent>("time_scale");
	componentManager.RegisterComponentType<pragma::SInfoLandmarkComponent>("info_landmark");
	componentManager.RegisterComponentType<pragma::SAttachableComponent>("attachable");
	componentManager.RegisterComponentType<pragma::SParentComponent>("parent");
	componentManager.RegisterComponentType<pragma::SOwnableComponent>("ownable");
	componentManager.RegisterComponentType<pragma::SDebugTextComponent>("debug_text");
	componentManager.RegisterComponentType<pragma::SDebugPointComponent>("debug_point");
	componentManager.RegisterComponentType<pragma::SDebugLineComponent>("debug_line");
	componentManager.RegisterComponentType<pragma::SDebugBoxComponent>("debug_box");
	componentManager.RegisterComponentType<pragma::SDebugSphereComponent>("debug_sphere");
	componentManager.RegisterComponentType<pragma::SDebugConeComponent>("debug_cone");
	componentManager.RegisterComponentType<pragma::SDebugCylinderComponent>("debug_cylinder");
	componentManager.RegisterComponentType<pragma::SDebugPlaneComponent>("debug_plane");
	componentManager.RegisterComponentType<pragma::SPointAtTargetComponent>("point_at_target");
}

pragma::BaseEntityComponent *SGame::CreateLuaEntityComponent(BaseEntity &ent,std::string classname)
{
	return Game::CreateLuaEntityComponent<pragma::SLuaBaseEntityComponent>(ent,classname);
}
