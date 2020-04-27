/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_LENTITY_HANDLES_HPP__
#define __S_LENTITY_HANDLES_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_character_component.hpp"
#include "pragma/entities/components/s_player_component.hpp"
#include "pragma/entities/components/s_ai_component.hpp"
#include "pragma/entities/components/s_flammable_component.hpp"
#include "pragma/entities/components/s_health_component.hpp"
#include "pragma/entities/components/s_name_component.hpp"
#include "pragma/entities/components/s_networked_component.hpp"
#include "pragma/entities/components/s_observable_component.hpp"
#include "pragma/entities/components/s_color_component.hpp"
#include "pragma/entities/components/s_score_component.hpp"
#include "pragma/entities/components/s_physics_component.hpp"
#include "pragma/entities/components/s_radius_component.hpp"
#include "pragma/entities/components/s_render_component.hpp"
#include "pragma/entities/components/s_sound_emitter_component.hpp"
#include "pragma/entities/components/s_toggle_component.hpp"
#include "pragma/entities/components/s_transform_component.hpp"
#include "pragma/entities/components/s_vehicle_component.hpp"
#include "pragma/entities/components/s_weapon_component.hpp"
#include "pragma/entities/components/s_wheel_component.hpp"
#include "pragma/entities/components/s_shooter_component.hpp"
#include "pragma/entities/components/s_model_component.hpp"
#include "pragma/entities/components/s_animated_component.hpp"
#include "pragma/entities/components/s_io_component.hpp"
#include "pragma/entities/components/s_time_scale_component.hpp"
#include "pragma/entities/components/s_attachable_component.hpp"
#include "pragma/entities/components/s_parent_component.hpp"
#include "pragma/entities/components/s_generic_component.hpp"
#include "pragma/entities/components/s_point_at_target_component.hpp"
#include "pragma/entities/environment/audio/s_env_sound_dsp_chorus.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_distortion.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_eaxreverb.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_echo.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_equalizer.h"
#include "pragma/entities/environment/audio/s_env_sound_dsp_flanger.h"
#include "pragma/entities/environment/s_env_camera.h"
#include "pragma/entities/environment/s_env_decal.h"
#include "pragma/entities/environment/effects/s_env_explosion.h"
#include "pragma/entities/environment/effects/s_env_sprite.h"
#include "pragma/entities/environment/s_env_fog_controller.h"
#include "pragma/entities/environment/effects/s_env_fire.h"
#include "pragma/entities/environment/lights/s_env_light.h"
#include "pragma/entities/environment/lights/s_env_light_directional.h"
#include "pragma/entities/environment/lights/s_env_light_point.h"
#include "pragma/entities/environment/lights/s_env_light_spot.h"
#include "pragma/entities/environment/s_env_microphone.h"
#include "pragma/entities/environment/effects/s_env_particle_system.h"
#include "pragma/entities/environment/s_env_quake.h"
#include "pragma/entities/environment/effects/s_env_fire.h"
#include "pragma/entities/environment/lights/s_env_light_spot_vol.h"
#include "pragma/entities/environment/effects/s_env_smoke_trail.h"
#include "pragma/entities/environment/audio/s_env_sound.h"
#include "pragma/entities/environment/audio/s_env_soundscape.h"
#include "pragma/entities/environment/s_env_timescale.h"
#include "pragma/entities/info/s_info_landmark.hpp"
#include "pragma/entities/environment/s_env_wind.hpp"
#include "pragma/entities/filter/s_filter_entity_class.h"
#include "pragma/entities/filter/s_filter_entity_name.h"
#include "pragma/entities/game_player_spawn.h"
#include "pragma/entities/logic/s_logic_relay.h"
#include "pragma/entities/s_bot.h"
#include "pragma/entities/point/constraints/s_point_constraint_ballsocket.h"
#include "pragma/entities/point/constraints/s_point_constraint_conetwist.h"
#include "pragma/entities/point/constraints/s_point_constraint_dof.h"
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
#include "pragma/entities/func/s_func_brush.h"
#include "pragma/entities/func/s_func_kinematic.hpp"
#include "pragma/entities/func/s_func_physics.h"
#include "pragma/entities/func/s_func_portal.h"
#include "pragma/entities/func/s_func_softphysics.hpp"
#include "pragma/entities/func/s_func_water.h"
#include "pragma/entities/func/s_funcbutton.h"
#include "pragma/entities/s_skybox.h"
#include "pragma/entities/s_flashlight.h"
#include "pragma/entities/components/s_ownable_component.hpp"
#include "pragma/entities/components/s_debug_component.hpp"
#include <pragma/lua/l_entity_handles.hpp>
#include <pragma/lua/ldefinitions.h>

DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SAI,BaseAIComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SCharacter,BaseCharacterComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SColor,BaseColorComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SScore,BaseScoreComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SFlammable,BaseFlammableComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SHealth,BaseHealthComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SName,BaseNameComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SNetworked,BaseNetworkedComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SObservable,BaseObservableComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPhysics,BasePhysicsComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPlayer,BasePlayerComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SRadius,BaseRadiusComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SRender,BaseRenderComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SSoundEmitter,BaseSoundEmitterComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SToggle,BaseToggleComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,STransform,BaseTransformComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SVehicle,BaseVehicleComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SWeapon,BaseWeaponComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SWheel,BaseWheelComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SShooter,BaseShooterComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SIO,BaseIOComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SModel,BaseModelComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SAnimated,BaseAnimatedComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SAttachable,BaseAttachableComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SParent,BaseParentComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,STimeScale,BaseTimeScaleComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SGeneric,BaseGenericComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE(DLLSERVER,SSoundDsp);
DEFINE_LUA_COMPONENT_HANDLE(DLLSERVER,SSoundDspChorus);
DEFINE_LUA_COMPONENT_HANDLE(DLLSERVER,SSoundDspDistortion);
DEFINE_LUA_COMPONENT_HANDLE(DLLSERVER,SSoundDspEAXReverb);
DEFINE_LUA_COMPONENT_HANDLE(DLLSERVER,SSoundDspEcho);
DEFINE_LUA_COMPONENT_HANDLE(DLLSERVER,SSoundDspEqualizer);
DEFINE_LUA_COMPONENT_HANDLE(DLLSERVER,SSoundDspFlanger);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SCamera,BaseEnvCameraComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE(DLLSERVER,SDecal);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SExplosion,BaseEnvExplosionComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SFire,BaseEnvFireComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SFogController,BaseEnvFogControllerComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SLight,BaseEnvLightComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SLightDirectional,BaseEnvLightDirectionalComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SLightPoint,BaseEnvLightPointComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SLightSpot,BaseEnvLightSpotComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SLightSpotVol,BaseEnvLightSpotVolComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SMicrophone,BaseEnvMicrophoneComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SParticleSystem,BaseEnvParticleSystemComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SQuake,BaseEnvQuakeComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SSmokeTrail,BaseEnvSmokeTrailComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SSound,BaseEnvSoundComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SSoundScape,BaseEnvSoundScapeComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SSprite,BaseEnvSpriteComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SEnvTimescale,BaseEnvTimescaleComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SInfoLandmark,BaseInfoLandmarkComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SWind,BaseEnvWindComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SFilterClass,BaseFilterClassComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SFilterName,BaseFilterNameComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SBrush,BaseFuncBrushComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SKinematic,BaseFuncKinematicComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SFuncPhysics,BaseFuncPhysicsComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SFuncSoftPhysics,BaseFuncSoftPhysicsComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SFuncPortal,BaseFuncPortalComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SWater,BaseFuncWaterComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SButton,BaseFuncButtonComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE(DLLSERVER,SPlayerSpawn);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SLogicRelay,BaseLogicRelayComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SBot,BaseBotComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPointConstraintBallSocket,BasePointConstraintBallSocketComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPointConstraintConeTwist,BasePointConstraintConeTwistComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPointConstraintDoF,BasePointConstraintDoFComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPointConstraintFixed,BasePointConstraintFixedComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPointConstraintHinge,BasePointConstraintHingeComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPointConstraintSlider,BasePointConstraintSliderComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPathNode,BasePointPathNodeComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SRenderTarget,BasePointRenderTargetComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPointTarget,BasePointTargetComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,Prop,BasePropComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPropDynamic,BasePropDynamicComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPropPhysics,BasePropPhysicsComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,STriggerGravity,BaseEntityTriggerGravityComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,STriggerHurt,BaseTriggerHurtComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,STriggerPush,BaseTriggerPushComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,STriggerRemove,BaseTriggerRemoveComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,STriggerTeleport,BaseTriggerTeleportComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,STouch,BaseTouchComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SSkybox,BaseSkyboxComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SWorld,BaseWorldComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SFlashlight,BaseFlashlightComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SOwnable,BaseOwnableComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SDebugText,BaseDebugTextComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SDebugPoint,BaseDebugPointComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SDebugLine,BaseDebugLineComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SDebugBox,BaseDebugBoxComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SDebugSphere,BaseDebugSphereComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SDebugCone,BaseDebugConeComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SDebugCylinder,BaseDebugCylinderComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SDebugPlane,BaseDebugPlaneComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLSERVER,SPointAtTarget,BasePointAtTargetComponentHandleWrapper);

#endif
