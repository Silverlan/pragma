/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_LENTITY_HANDLES_HPP__
#define __C_LENTITY_HANDLES_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_character_component.hpp"
#include "pragma/entities/components/c_player_component.hpp"
#include "pragma/entities/components/c_ai_component.hpp"
#include "pragma/entities/components/c_flammable_component.hpp"
#include "pragma/entities/components/c_health_component.hpp"
#include "pragma/entities/components/c_name_component.hpp"
#include "pragma/entities/components/c_networked_component.hpp"
#include "pragma/entities/components/c_observable_component.hpp"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_score_component.hpp"
#include "pragma/entities/components/c_physics_component.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/entities/components/c_sound_emitter_component.hpp"
#include "pragma/entities/components/c_toggle_component.hpp"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/components/c_vehicle_component.hpp"
#include "pragma/entities/components/c_weapon_component.hpp"
#include "pragma/entities/components/c_shooter_component.hpp"
#include "pragma/entities/components/c_model_component.hpp"
#include "pragma/entities/components/c_animated_component.hpp"
#include "pragma/entities/components/c_io_component.hpp"
#include "pragma/entities/components/c_attachable_component.hpp"
#include "pragma/entities/components/c_parent_component.hpp"
#include "pragma/entities/components/c_time_scale_component.hpp"
#include "pragma/entities/components/c_generic_component.hpp"
#include "pragma/entities/components/c_point_at_target_component.hpp"
#include "pragma/entities/components/c_scene_component.hpp"
#include "pragma/entities/components/c_gamemode_component.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include "pragma/entities/environment/lights/c_env_shadow_csm.hpp"
#include "pragma/entities/c_filter_entity_class.h"
#include "pragma/entities/c_filter_entity_name.h"
#include "pragma/entities/components/c_bsp_leaf_component.hpp"
#include "pragma/entities/c_wheel.hpp"
#include "pragma/entities/environment/audio/c_env_sound_dsp_chorus.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_distortion.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_eaxreverb.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_echo.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_equalizer.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_flanger.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/c_env_decal.h"
#include "pragma/entities/environment/effects/c_env_explosion.h"
#include "pragma/entities/environment/c_env_decal.h"
#include "pragma/entities/environment/c_env_fog_controller.h"
#include "pragma/entities/environment/effects/c_env_fire.h"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/c_env_microphone.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/environment/c_env_quake.h"
#include "pragma/entities/environment/effects/c_env_fire.h"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/entities/environment/effects/c_env_smoke_trail.h"
#include "pragma/entities/environment/audio/c_env_sound.h"
#include "pragma/entities/environment/audio/c_env_soundscape.h"
#include "pragma/entities/environment/c_env_timescale.h"
#include "pragma/entities/environment/c_env_wind.hpp"
#include "pragma/entities/c_bot.h"
#include "pragma/entities/point/constraints/c_point_constraint_ballsocket.h"
#include "pragma/entities/point/constraints/c_point_constraint_conetwist.h"
#include "pragma/entities/point/constraints/c_point_constraint_dof.h"
#include "pragma/entities/point/constraints/c_point_constraint_fixed.h"
#include "pragma/entities/point/constraints/c_point_constraint_hinge.h"
#include "pragma/entities/point/constraints/c_point_constraint_slider.h"
#include "pragma/entities/point/c_point_rendertarget.h"
#include "pragma/entities/point/c_point_target.h"
#include "pragma/entities/prop/c_prop_base.hpp"
#include "pragma/entities/prop/c_prop_dynamic.hpp"
#include "pragma/entities/prop/c_prop_physics.h"
#include "pragma/entities/trigger/c_trigger_touch.h"
#include "pragma/entities/func/c_func_brush.h"
#include "pragma/entities/func/c_func_kinematic.hpp"
#include "pragma/entities/func/c_func_physics.h"
#include "pragma/entities/func/c_func_portal.h"
#include "pragma/entities/func/c_func_softphysics.hpp"
#include "pragma/entities/func/c_func_water.h"
#include "pragma/entities/c_water_surface.hpp"
#include "pragma/entities/func/c_funcbutton.h"
#include "pragma/entities/c_skybox.h"
#include "pragma/entities/c_flashlight.h"
#include "pragma/entities/environment/c_env_weather.h"
#include "pragma/entities/environment/audio/c_env_sound_probe.hpp"
#include "pragma/entities/c_listener.h"
#include "pragma/entities/c_viewbody.h"
#include "pragma/entities/c_viewmodel.h"
#include "pragma/entities/components/renderers/c_rasterization_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_raytracing_renderer_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_component.hpp"
#include "pragma/entities/components/c_flex_component.hpp"
#include "pragma/entities/components/c_softbody_component.hpp"
#include "pragma/entities/components/c_vertex_animated_component.hpp"
#include "pragma/entities/components/c_eye_component.hpp"
#include "pragma/entities/components/c_raytracing_component.hpp"
#include "pragma/entities/components/c_ownable_component.hpp"
#include "pragma/entities/components/c_debug_component.hpp"
#include "pragma/entities/components/c_bsp_component.hpp"
#include "pragma/entities/components/c_light_map_component.hpp"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/entities/environment/c_env_reflection_probe.hpp"
#include "pragma/entities/environment/c_sky_camera.hpp"
#include "pragma/entities/util/c_util_pbr_converter.hpp"
#include "pragma/entities/game/c_game_shadow_manager.hpp"
#include "pragma/entities/game/c_game_occlusion_culler.hpp"
#include "pragma/entities/environment/lights/c_env_shadow.hpp"
#include <pragma/lua/l_entity_handles.hpp>
#include <pragma/lua/ldefinitions.h>

DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CAI,BaseAIComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CCharacter,BaseCharacterComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CColor,BaseColorComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CScore,BaseScoreComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CFlammable,BaseFlammableComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CHealth,BaseHealthComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CName,BaseNameComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CNetworked,BaseNetworkedComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CObservable,BaseObservableComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPhysics,BasePhysicsComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPlayer,BasePlayerComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CRadius,BaseRadiusComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CRender,BaseRenderComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CSoundEmitter,BaseSoundEmitterComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CToggle,BaseToggleComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CTransform,BaseTransformComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CVehicle,BaseVehicleComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CWeapon,BaseWeaponComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CWheel,BaseWheelComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CShooter,BaseShooterComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CIO,BaseIOComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CModel,BaseModelComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CAnimated,BaseAnimatedComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CAttachable,BaseAttachableComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CParent,BaseParentComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CTimeScale,BaseTimeScaleComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CGeneric,BaseGenericComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CSoundDsp);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CSoundDspChorus);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CSoundDspDistortion);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CSoundDspEAXReverb);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CSoundDspEcho);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CSoundDspEqualizer);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CSoundDspFlanger);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CCamera,BaseEnvCameraComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CDecal);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CExplosion,BaseEnvExplosionComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CFire,BaseEnvFireComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CFogController,BaseEnvFogControllerComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CLight,BaseEnvLightComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CLightDirectional,BaseEnvLightDirectionalComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CLightPoint,BaseEnvLightPointComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CLightSpot,BaseEnvLightSpotComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CLightSpotVol,BaseEnvLightSpotVolComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CMicrophone,BaseEnvMicrophoneComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CParticleSystem,BaseEnvParticleSystemComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CQuake,BaseEnvQuakeComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CSmokeTrail,BaseEnvSmokeTrailComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CSound,BaseEnvSoundComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CSoundScape,BaseEnvSoundScapeComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CSprite,BaseEnvSpriteComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CEnvTimescale,BaseEnvTimescaleComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CWind,BaseEnvWindComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CBrush,BaseFuncBrushComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CKinematic,BaseFuncKinematicComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CFuncPhysics,BaseFuncPhysicsComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CFuncSoftPhysics,BaseFuncSoftPhysicsComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CFuncPortal,BaseFuncPortalComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CWater,BaseFuncWaterComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CButton,BaseFuncButtonComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CBot,BaseBotComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPointConstraintBallSocket,BasePointConstraintBallSocketComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPointConstraintConeTwist,BasePointConstraintConeTwistComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPointConstraintDoF,BasePointConstraintDoFComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPointConstraintFixed,BasePointConstraintFixedComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPointConstraintHinge,BasePointConstraintHingeComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPointConstraintSlider,BasePointConstraintSliderComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CRenderTarget,BasePointRenderTargetComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPointTarget,BasePointTargetComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CProp,BasePropComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPropDynamic,BasePropDynamicComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPropPhysics,BasePropPhysicsComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CTouch,BaseTouchComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CSkybox,BaseSkyboxComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CWorld,BaseWorldComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CFlashlight,BaseFlashlightComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,COwnable,BaseOwnableComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CDebugText,BaseDebugTextComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CDebugPoint,BaseDebugPointComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CDebugLine,BaseDebugLineComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CDebugBox,BaseDebugBoxComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CDebugSphere,BaseDebugSphereComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CDebugCone,BaseDebugConeComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CDebugCylinder,BaseDebugCylinderComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CDebugPlane,BaseDebugPlaneComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CFilterClass,BaseFilterClassComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CFilterName,BaseFilterNameComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CPointAtTarget,BasePointAtTargetComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLCLIENT,CGamemode,BaseGamemodeComponentHandleWrapper);

DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CEnvSoundProbe);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CWeather);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CWaterSurface);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CListener);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CViewBody);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CViewModel);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CFlex);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CSoftBody);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CVertexAnimated);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CRaytracing);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CLightMap);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CLightMapReceiver);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CBSP);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CBSPLeaf);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CReflectionProbe);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CPBRConverter);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CShadowManager);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CShadow);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CShadowCSM);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,COcclusionCuller);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CEye);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CSkyCamera);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CScene);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CRenderer);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CRasterizationRenderer);
DEFINE_LUA_COMPONENT_HANDLE(DLLCLIENT,CRaytracingRenderer);

#endif
