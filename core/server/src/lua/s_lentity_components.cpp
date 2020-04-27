/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

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

namespace Lua
{
	namespace SoundEmitter
	{
		static void CreateSound(lua_State *l,SSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,bool bTransmit);
		static void EmitSound(lua_State *l,SSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit);
	};
};
void SGame::RegisterLuaEntityComponent(luabind::class_<BaseEntityComponentHandleWrapper> &def)
{
	Game::RegisterLuaEntityComponent(def);
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,uint32_t,NetPacket&,pragma::networking::TargetRecipientFilter&)>(
		[](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t protocol,uint32_t eventId,NetPacket &packet,pragma::networking::TargetRecipientFilter &rf) {
			pragma::Lua::check_component(l,hComponent);
			static_cast<SBaseEntity&>(hComponent->GetEntity()).SendNetEvent(eventId,packet,static_cast<pragma::networking::Protocol>(protocol),rf);
	}));
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,uint32_t,NetPacket&)>(
		[](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t protocol,uint32_t eventId,NetPacket &packet) {
			pragma::Lua::check_component(l,hComponent);
			static_cast<SBaseEntity&>(hComponent->GetEntity()).SendNetEvent(eventId,packet,static_cast<pragma::networking::Protocol>(protocol));
	}));
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,uint32_t)>(
		[](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t protocol,uint32_t eventId) {
			pragma::Lua::check_component(l,hComponent);
			static_cast<SBaseEntity&>(hComponent->GetEntity()).SendNetEvent(eventId,static_cast<pragma::networking::Protocol>(protocol));
	}));
	def.def("SendNetEvent",static_cast<void(*)(lua_State*,BaseEntityComponentHandle&,uint32_t,NetPacket&)>(
		[](lua_State *l,BaseEntityComponentHandle &hComponent,uint32_t eventId,NetPacket &packet) {
			pragma::Lua::check_component(l,hComponent);
			static_cast<SBaseEntity&>(hComponent->GetEntity()).SendNetEvent(eventId,packet);
	}));
}
void SGame::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	Game::RegisterLuaEntityComponents(entsMod);

	auto *l = GetLuaState();
	Lua::register_sv_ai_component(l,entsMod);
	Lua::register_sv_character_component(l,entsMod);
	Lua::register_sv_player_component(l,entsMod);
	Lua::register_sv_vehicle_component(l,entsMod);
	Lua::register_sv_weapon_component(l,entsMod);

	auto defSColor = luabind::class_<SColorHandle,BaseEntityComponentHandle>("ColorComponent");
	Lua::register_base_color_component_methods<luabind::class_<SColorHandle,BaseEntityComponentHandle>,SColorHandle>(l,defSColor);
	entsMod[defSColor];

	auto defSScore = luabind::class_<SScoreHandle,BaseEntityComponentHandle>("ScoreComponent");
	Lua::register_base_score_component_methods<luabind::class_<SScoreHandle,BaseEntityComponentHandle>,SScoreHandle>(l,defSScore);
	entsMod[defSScore];

	auto defSFlammable = luabind::class_<SFlammableHandle,BaseEntityComponentHandle>("FlammableComponent");
	Lua::register_base_flammable_component_methods<luabind::class_<SFlammableHandle,BaseEntityComponentHandle>,SFlammableHandle>(l,defSFlammable);
	entsMod[defSFlammable];

	auto defSHealth = luabind::class_<SHealthHandle,BaseEntityComponentHandle>("HealthComponent");
	Lua::register_base_health_component_methods<luabind::class_<SHealthHandle,BaseEntityComponentHandle>,SHealthHandle>(l,defSHealth);
	entsMod[defSHealth];

	auto defSName = luabind::class_<SNameHandle,BaseEntityComponentHandle>("NameComponent");
	Lua::register_base_name_component_methods<luabind::class_<SNameHandle,BaseEntityComponentHandle>,SNameHandle>(l,defSName);
	entsMod[defSName];

	auto defSNetworked = luabind::class_<SNetworkedHandle,BaseEntityComponentHandle>("NetworkedComponent");
	Lua::register_base_networked_component_methods<luabind::class_<SNetworkedHandle,BaseEntityComponentHandle>,SNetworkedHandle>(l,defSNetworked);
	entsMod[defSNetworked];

	auto defSObservable = luabind::class_<SObservableHandle,BaseEntityComponentHandle>("ObservableComponent");
	Lua::register_base_observable_component_methods<luabind::class_<SObservableHandle,BaseEntityComponentHandle>,SObservableHandle>(l,defSObservable);
	entsMod[defSObservable];

	auto defSShooter = luabind::class_<SShooterHandle,BaseEntityComponentHandle>("ShooterComponent");
	Lua::register_base_shooter_component_methods<luabind::class_<SShooterHandle,BaseEntityComponentHandle>,SShooterHandle>(l,defSShooter);
	entsMod[defSShooter];

	auto defSPhysics = luabind::class_<SPhysicsHandle,BaseEntityComponentHandle>("PhysicsComponent");
	Lua::register_base_physics_component_methods<luabind::class_<SPhysicsHandle,BaseEntityComponentHandle>,SPhysicsHandle>(l,defSPhysics);
	entsMod[defSPhysics];

	auto defSRadius = luabind::class_<SRadiusHandle,BaseEntityComponentHandle>("RadiusComponent");
	Lua::register_base_radius_component_methods<luabind::class_<SRadiusHandle,BaseEntityComponentHandle>,SRadiusHandle>(l,defSRadius);
	entsMod[defSRadius];

	auto defSRender = luabind::class_<SRenderHandle,BaseEntityComponentHandle>("RenderComponent");
	Lua::register_base_render_component_methods<luabind::class_<SRenderHandle,BaseEntityComponentHandle>,SRenderHandle>(l,defSRender);
	entsMod[defSRender];

	auto defSSoundEmitter = luabind::class_<SSoundEmitterHandle,BaseEntityComponentHandle>("SoundEmitterComponent");
	Lua::register_base_sound_emitter_component_methods<luabind::class_<SSoundEmitterHandle,BaseEntityComponentHandle>,SSoundEmitterHandle>(l,defSSoundEmitter);
	defSSoundEmitter.def("CreateSound",static_cast<void(*)(lua_State*,SSoundEmitterHandle&,std::string,uint32_t,bool)>(&Lua::SoundEmitter::CreateSound));
	defSSoundEmitter.def("EmitSound",static_cast<void(*)(lua_State*,SSoundEmitterHandle&,std::string,uint32_t,float,float,bool)>(&Lua::SoundEmitter::EmitSound));
	entsMod[defSSoundEmitter];

	auto defSToggle = luabind::class_<SToggleHandle,BaseEntityComponentHandle>("ToggleComponent");
	Lua::register_base_toggle_component_methods<luabind::class_<SToggleHandle,BaseEntityComponentHandle>,SToggleHandle>(l,defSToggle);
	entsMod[defSToggle];

	auto defSTransform = luabind::class_<STransformHandle,BaseEntityComponentHandle>("TransformComponent");
	Lua::register_base_transform_component_methods<luabind::class_<STransformHandle,BaseEntityComponentHandle>,STransformHandle>(l,defSTransform);
	entsMod[defSTransform];

	auto defSWheel = luabind::class_<SWheelHandle,BaseEntityComponentHandle>("WheelComponent");
	Lua::register_base_wheel_component_methods<luabind::class_<SWheelHandle,BaseEntityComponentHandle>,SWheelHandle>(l,defSWheel);
	entsMod[defSWheel];

	auto defSSoundDsp = luabind::class_<SSoundDspHandle,BaseEntityComponentHandle>("SoundDspComponent");
	Lua::register_base_env_sound_dsp_component_methods<luabind::class_<SSoundDspHandle,BaseEntityComponentHandle>,SSoundDspHandle>(l,defSSoundDsp);
	entsMod[defSSoundDsp];

	auto defSSoundDspChorus = luabind::class_<SSoundDspChorusHandle,BaseEntityComponentHandle>("SoundDspChorusComponent");
	entsMod[defSSoundDspChorus];

	auto defSSoundDspDistortion = luabind::class_<SSoundDspDistortionHandle,BaseEntityComponentHandle>("SoundDspDistortionComponent");
	entsMod[defSSoundDspDistortion];

	auto defSSoundDspEAXReverb = luabind::class_<SSoundDspEAXReverbHandle,BaseEntityComponentHandle>("SoundDspEAXReverbComponent");
	entsMod[defSSoundDspEAXReverb];

	auto defSSoundDspEcho = luabind::class_<SSoundDspEchoHandle,BaseEntityComponentHandle>("SoundDspEchoComponent");
	entsMod[defSSoundDspEcho];

	auto defSSoundDspEqualizer = luabind::class_<SSoundDspEqualizerHandle,BaseEntityComponentHandle>("SoundDspEqualizerComponent");
	entsMod[defSSoundDspEqualizer];

	auto defSSoundDspFlanger = luabind::class_<SSoundDspFlangerHandle,BaseEntityComponentHandle>("SoundDspFlangerComponent");
	entsMod[defSSoundDspFlanger];

	auto defSCamera = luabind::class_<SCameraHandle,BaseEntityComponentHandle>("CameraComponent");
	Lua::register_base_env_camera_component_methods<luabind::class_<SCameraHandle,BaseEntityComponentHandle>,SCameraHandle>(l,defSCamera);
	entsMod[defSCamera];

	auto defSDecal = luabind::class_<SDecalHandle,BaseEntityComponentHandle>("DecalComponent");
	Lua::register_base_decal_component_methods<luabind::class_<SDecalHandle,BaseEntityComponentHandle>,SDecalHandle>(l,defSDecal);
	entsMod[defSDecal];

	auto defSExplosion = luabind::class_<SExplosionHandle,BaseEntityComponentHandle>("ExplosionComponent");
	Lua::register_base_env_explosion_component_methods<luabind::class_<SExplosionHandle,BaseEntityComponentHandle>,SExplosionHandle>(l,defSExplosion);
	entsMod[defSExplosion];

	auto defSFire = luabind::class_<SFireHandle,BaseEntityComponentHandle>("FireComponent");
	Lua::register_base_env_fire_component_methods<luabind::class_<SFireHandle,BaseEntityComponentHandle>,SFireHandle>(l,defSFire);
	entsMod[defSFire];

	auto defSFogController = luabind::class_<SFogControllerHandle,BaseEntityComponentHandle>("FogControllerComponent");
	Lua::register_base_env_fog_controller_component_methods<luabind::class_<SFogControllerHandle,BaseEntityComponentHandle>,SFogControllerHandle>(l,defSFogController);
	entsMod[defSFogController];

	auto defSLight = luabind::class_<SLightHandle,BaseEntityComponentHandle>("LightComponent");
	Lua::register_base_env_light_component_methods<luabind::class_<SLightHandle,BaseEntityComponentHandle>,SLightHandle>(l,defSLight);
	entsMod[defSLight];

	auto defSLightDirectional = luabind::class_<SLightDirectionalHandle,BaseEntityComponentHandle>("LightDirectionalComponent");
	Lua::register_base_env_light_directional_component_methods<luabind::class_<SLightDirectionalHandle,BaseEntityComponentHandle>,SLightDirectionalHandle>(l,defSLightDirectional);
	entsMod[defSLightDirectional];

	auto defSLightPoint = luabind::class_<SLightPointHandle,BaseEntityComponentHandle>("LightPointComponent");
	Lua::register_base_env_light_point_component_methods<luabind::class_<SLightPointHandle,BaseEntityComponentHandle>,SLightPointHandle>(l,defSLightPoint);
	entsMod[defSLightPoint];

	auto defSLightSpot = luabind::class_<SLightSpotHandle,BaseEntityComponentHandle>("LightSpotComponent");
	Lua::register_base_env_light_spot_component_methods<luabind::class_<SLightSpotHandle,BaseEntityComponentHandle>,SLightSpotHandle>(l,defSLightSpot);
	entsMod[defSLightSpot];

	auto defSLightSpotVol = luabind::class_<SLightSpotVolHandle,BaseEntityComponentHandle>("LightSpotVolComponent");
	Lua::register_base_env_light_spot_vol_component_methods<luabind::class_<SLightSpotVolHandle,BaseEntityComponentHandle>,SLightSpotVolHandle>(l,defSLightSpotVol);
	entsMod[defSLightSpotVol];

	auto defSMicrophone = luabind::class_<SMicrophoneHandle,BaseEntityComponentHandle>("MicrophoneComponent");
	Lua::register_base_env_microphone_component_methods<luabind::class_<SMicrophoneHandle,BaseEntityComponentHandle>,SMicrophoneHandle>(l,defSMicrophone);
	entsMod[defSMicrophone];

	auto defSParticleSystem = luabind::class_<SParticleSystemHandle,BaseEntityComponentHandle>("ParticleSystemComponent");
	Lua::register_base_env_particle_system_component_methods<luabind::class_<SParticleSystemHandle,BaseEntityComponentHandle>,SParticleSystemHandle>(l,defSParticleSystem);
	entsMod[defSParticleSystem];

	auto defSQuake = luabind::class_<SQuakeHandle,BaseEntityComponentHandle>("QuakeComponent");
	Lua::register_base_env_quake_component_methods<luabind::class_<SQuakeHandle,BaseEntityComponentHandle>,SQuakeHandle>(l,defSQuake);
	entsMod[defSQuake];

	auto defSSmokeTrail = luabind::class_<SSmokeTrailHandle,BaseEntityComponentHandle>("SmokeTrailComponent");
	Lua::register_base_env_smoke_trail_component_methods<luabind::class_<SSmokeTrailHandle,BaseEntityComponentHandle>,SSmokeTrailHandle>(l,defSSmokeTrail);
	entsMod[defSSmokeTrail];

	auto defSSound = luabind::class_<SSoundHandle,BaseEntityComponentHandle>("SoundComponent");
	Lua::register_base_env_sound_component_methods<luabind::class_<SSoundHandle,BaseEntityComponentHandle>,SSoundHandle>(l,defSSound);
	entsMod[defSSound];

	auto defSSoundScape = luabind::class_<SSoundScapeHandle,BaseEntityComponentHandle>("SoundScapeComponent");
	Lua::register_base_env_soundscape_component_methods<luabind::class_<SSoundScapeHandle,BaseEntityComponentHandle>,SSoundScapeHandle>(l,defSSoundScape);
	entsMod[defSSoundScape];

	auto defSSprite = luabind::class_<SSpriteHandle,BaseEntityComponentHandle>("SpriteComponent");
	Lua::register_base_env_sprite_component_methods<luabind::class_<SSpriteHandle,BaseEntityComponentHandle>,SSpriteHandle>(l,defSSprite);
	entsMod[defSSprite];

	auto defSTimescale = luabind::class_<SEnvTimescaleHandle,BaseEntityComponentHandle>("EnvTimeScaleComponent");
	Lua::register_base_env_timescale_component_methods<luabind::class_<SEnvTimescaleHandle,BaseEntityComponentHandle>,SEnvTimescaleHandle>(l,defSTimescale);
	entsMod[defSTimescale];

	auto defSWind = luabind::class_<SWindHandle,BaseEntityComponentHandle>("WindComponent");
	Lua::register_base_env_wind_component_methods<luabind::class_<SWindHandle,BaseEntityComponentHandle>,SWindHandle>(l,defSWind);
	entsMod[defSWind];

	auto defSFilterClass = luabind::class_<SFilterClassHandle,BaseEntityComponentHandle>("FilterClassComponent");
	Lua::register_base_env_filter_class_component_methods<luabind::class_<SFilterClassHandle,BaseEntityComponentHandle>,SFilterClassHandle>(l,defSFilterClass);
	entsMod[defSFilterClass];

	auto defSFilterName = luabind::class_<SFilterNameHandle,BaseEntityComponentHandle>("FilterNameComponent");
	Lua::register_base_env_filter_name_component_methods<luabind::class_<SFilterNameHandle,BaseEntityComponentHandle>,SFilterNameHandle>(l,defSFilterName);
	entsMod[defSFilterName];

	auto defSBrush = luabind::class_<SBrushHandle,BaseEntityComponentHandle>("BrushComponent");
	Lua::register_base_func_brush_component_methods<luabind::class_<SBrushHandle,BaseEntityComponentHandle>,SBrushHandle>(l,defSBrush);
	entsMod[defSBrush];

	auto defSKinematic = luabind::class_<SKinematicHandle,BaseEntityComponentHandle>("KinematicComponent");
	Lua::register_base_func_kinematic_component_methods<luabind::class_<SKinematicHandle,BaseEntityComponentHandle>,SKinematicHandle>(l,defSKinematic);
	entsMod[defSKinematic];

	auto defSFuncPhysics = luabind::class_<SFuncPhysicsHandle,BaseEntityComponentHandle>("FuncPhysicsComponent");
	Lua::register_base_func_physics_component_methods<luabind::class_<SFuncPhysicsHandle,BaseEntityComponentHandle>,SFuncPhysicsHandle>(l,defSFuncPhysics);
	entsMod[defSFuncPhysics];

	auto defSFuncSoftPhysics = luabind::class_<SFuncSoftPhysicsHandle,BaseEntityComponentHandle>("FuncSoftPhysicsComponent");
	Lua::register_base_func_soft_physics_component_methods<luabind::class_<SFuncSoftPhysicsHandle,BaseEntityComponentHandle>,SFuncSoftPhysicsHandle>(l,defSFuncSoftPhysics);
	entsMod[defSFuncSoftPhysics];

	auto defSFuncPortal = luabind::class_<SFuncPortalHandle,BaseEntityComponentHandle>("FuncPortalComponent");
	Lua::register_base_func_portal_component_methods<luabind::class_<SFuncPortalHandle,BaseEntityComponentHandle>,SFuncPortalHandle>(l,defSFuncPortal);
	entsMod[defSFuncPortal];

	auto defSWater = luabind::class_<SWaterHandle,BaseEntityComponentHandle>("WaterComponent");
	Lua::register_base_func_water_component_methods<luabind::class_<SWaterHandle,BaseEntityComponentHandle>,SWaterHandle>(l,defSWater);
	entsMod[defSWater];

	auto defSButton = luabind::class_<SButtonHandle,BaseEntityComponentHandle>("ButtonComponent");
	Lua::register_base_func_button_component_methods<luabind::class_<SButtonHandle,BaseEntityComponentHandle>,SButtonHandle>(l,defSButton);
	entsMod[defSButton];

	auto defSPlayerSpawn = luabind::class_<SPlayerSpawnHandle,BaseEntityComponentHandle>("PlayerSpawnComponent");
	entsMod[defSPlayerSpawn];

	auto defSLogicRelay = luabind::class_<SLogicRelayHandle,BaseEntityComponentHandle>("LogicRelayComponent");
	Lua::register_base_logic_relay_component_methods<luabind::class_<SLogicRelayHandle,BaseEntityComponentHandle>,SLogicRelayHandle>(l,defSLogicRelay);
	entsMod[defSLogicRelay];

	auto defSBot = luabind::class_<SBotHandle,BaseEntityComponentHandle>("BotComponent");
	Lua::register_base_bot_component_methods<luabind::class_<SBotHandle,BaseEntityComponentHandle>,SBotHandle>(l,defSBot);
	entsMod[defSBot];

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

	auto defSGeneric = luabind::class_<SGenericHandle,BaseEntityComponentHandle>("EntityComponent");
	//Lua::register_base_generic_component_methods<luabind::class_<SGenericHandle,BaseEntityComponentHandle>,SGenericHandle>(l,defSGeneric);
	entsMod[defSGeneric];
}

void Lua::SoundEmitter::CreateSound(lua_State *l,SSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,bool bTransmit)
{
	pragma::Lua::check_component(l,hEnt);
	auto snd = hEnt->CreateSound(sndname,static_cast<ALSoundType>(soundType),bTransmit);
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}
void Lua::SoundEmitter::EmitSound(lua_State *l,SSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit)
{
	pragma::Lua::check_component(l,hEnt);
	auto snd = hEnt->EmitSound(sndname,static_cast<ALSoundType>(soundType),gain,pitch,bTransmit);
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}
