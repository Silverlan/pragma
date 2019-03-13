#ifndef __LENTITY_HANDLES_HPP__
#define __LENTITY_HANDLES_HPP__

#include "pragma/networkdefinitions.h"
#include "pragma/lua/ldefinitions.h"
#include "pragma/entities/components/base_actor_component.hpp"
#include "pragma/entities/components/base_ai_component.hpp"
#include "pragma/entities/components/base_character_component.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/entities/components/base_wheel_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_attachable_component.hpp"
#include "pragma/entities/components/base_parent_component.hpp"
#include "pragma/entities/components/base_io_component.hpp"
#include "pragma/entities/components/base_time_scale_component.hpp"
#include "pragma/entities/components/base_generic_component.hpp"
#include "pragma/entities/components/velocity_component.hpp"
#include "pragma/entities/components/ik_component.hpp"
#include "pragma/entities/components/logic_component.hpp"
#include "pragma/entities/components/map_component.hpp"
#include "pragma/entities/components/damageable_component.hpp"
#include "pragma/entities/components/submergible_component.hpp"
#include "pragma/entities/components/usable_component.hpp"
#include "pragma/entities/components/global_component.hpp"
#include "pragma/entities/components/base_flammable_component.hpp"
#include "pragma/entities/components/base_flex_component.hpp"
#include "pragma/entities/components/base_health_component.hpp"
#include "pragma/entities/components/base_networked_component.hpp"
#include "pragma/entities/components/base_observable_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/base_render_component.hpp"
#include "pragma/entities/components/base_softbody_component.hpp"
#include "pragma/entities/components/base_sound_emitter_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_wheel_component.hpp"
#include "pragma/entities/components/base_color_component.hpp"
#include "pragma/entities/components/base_radius_component.hpp"
#include "pragma/entities/components/base_name_component.hpp"
#include "pragma/entities/components/base_shooter_component.hpp"
#include "pragma/entities/components/base_ownable_component.hpp"
#include "pragma/entities/components/base_point_at_target_component.hpp"
#include "pragma/entities/components/basegravity.h"
#include "pragma/entities/func/basefuncwater.h"
#include "pragma/entities/info/info_landmark.hpp"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include "pragma/entities/environment/lights/env_light_point.h"
#include "pragma/entities/environment/lights/env_light_spot.h"
#include "pragma/entities/environment/effects/env_particle_system.h"
#include "pragma/entities/environment/audio/env_sound_dsp.h"
#include "pragma/entities/environment/audio/env_sound_dsp_chorus.h"
#include "pragma/entities/environment/audio/env_sound_dsp_distortion.h"
#include "pragma/entities/environment/audio/env_sound_dsp_eaxreverb.h"
#include "pragma/entities/environment/audio/env_sound_dsp_echo.h"
#include "pragma/entities/environment/audio/env_sound_dsp_equalizer.h"
#include "pragma/entities/environment/audio/env_sound_dsp_flanger.h"
#include "pragma/entities/environment/audio/env_sound_dsp_reverb.h"
#include "pragma/entities/environment/env_camera.h"
#include "pragma/entities/environment/env_decal.h"
#include "pragma/entities/environment/effects/env_explosion.h"
#include "pragma/entities/environment/effects/env_fire.h"
#include "pragma/entities/environment/env_fog_controller.h"
#include "pragma/entities/environment/lights/env_light.h"
#include "pragma/entities/environment/lights/env_light_directional.h"
#include "pragma/entities/environment/lights/env_light_point.h"
#include "pragma/entities/environment/lights/env_light_spot.h"
#include "pragma/entities/environment/lights/env_light_spot_vol.h"
#include "pragma/entities/environment/env_microphone_base.h"
#include "pragma/entities/environment/effects/env_particle_system.h"
#include "pragma/entities/environment/env_quake.h"
#include "pragma/entities/environment/effects/env_smoke_trail.h"
#include "pragma/entities/environment/audio/env_sound.h"
#include "pragma/entities/environment/audio/env_soundscape.h"
#include "pragma/entities/environment/effects/env_sprite.h"
#include "pragma/entities/environment/env_timescale.h"
#include "pragma/entities/environment/env_weather.h"
#include "pragma/entities/environment/env_wind.hpp"
#include "pragma/entities/func/basefuncbrush.h"
#include "pragma/entities/func/basefuncbutton.h"
#include "pragma/entities/func/basefunckinematic.hpp"
#include "pragma/entities/func/basefuncphysics.h"
#include "pragma/entities/func/basefuncportal.h"
#include "pragma/entities/func/basefuncsoftphysics.hpp"
#include "pragma/entities/func/basefuncsurfacematerial.hpp"
#include "pragma/entities/func/basefuncwater.h"
#include "pragma/entities/logic/logic_relay.h"
#include "pragma/entities/point/constraints/point_constraint_ballsocket.h"
#include "pragma/entities/point/constraints/point_constraint_conetwist.h"
#include "pragma/entities/point/constraints/point_constraint_dof.h"
#include "pragma/entities/point/constraints/point_constraint_fixed.h"
#include "pragma/entities/point/constraints/point_constraint_hinge.h"
#include "pragma/entities/point/constraints/point_constraint_slider.h"
#include "pragma/entities/point/point_rendertarget.h"
#include "pragma/entities/point/point_target.h"
#include "pragma/entities/prop/prop_base.h"
#include "pragma/entities/prop/prop_dynamic.hpp"
#include "pragma/entities/prop/prop_physics.hpp"
#include "pragma/entities/trigger/base_trigger_hurt.hpp"
#include "pragma/entities/trigger/base_trigger_push.hpp"
#include "pragma/entities/trigger/base_trigger_remove.h"
#include "pragma/entities/trigger/base_trigger_teleport.hpp"
#include "pragma/entities/trigger/base_trigger_touch.hpp"
#include "pragma/entities/components/basetriggergravity.hpp"
#include "pragma/entities/baseskybox.h"
#include "pragma/entities/baseworld.h"
#include "pragma/entities/basebot.h"
#include "pragma/entities/baseflashlight.h"
#include "pragma/entities/components/basepointpathnode.h"
#include "pragma/entities/basefilterentity.h"
#include "pragma/entities/components/base_debug_component.hpp"
#include <sharedutils/util_weak_handle.hpp>

struct DLLNETWORK BaseEntityComponentHandleWrapper
{
	BaseEntityComponentHandleWrapper(const util::WeakHandle<pragma::BaseEntityComponent> &wh)
		: handle(wh)
	{}
	BaseEntityComponentHandleWrapper()=default;
	util::WeakHandle<pragma::BaseEntityComponent> handle = {};
	bool expired() const {return handle.expired();}

	pragma::BaseEntityComponent *operator->() {return get();}
	const pragma::BaseEntityComponent *get() const {return handle.get();}
	pragma::BaseEntityComponent *get() {return handle.get();}
};
template<class T,class TBase=BaseEntityComponentHandleWrapper>
	struct TBaseEntityComponentHandleWrapper
		: public TBase
{
	using TBase::TBase;
	const T *operator->() const {return get();}
	T *operator->() {return get();}

	const T &operator*() const {return *get();}
	T &operator*() {return *get();}

	const T *get() const {return static_cast<const T*>(handle.get());}
	T *get() {return static_cast<T*>(handle.get());}
};

using BaseEntityComponentHandle = BaseEntityComponentHandleWrapper;
lua_registercheck(BaseEntityComponent,BaseEntityComponentHandle);

#define DEFINE_LUA_COMPONENT_HANDLE_BASE(dllio,name,base) \
	struct dllio name##ComponentHandleWrapper \
		: public TBaseEntityComponentHandleWrapper<pragma::name##Component,base> \
	{ \
		using TBaseEntityComponentHandleWrapper<pragma::name##Component,base>::TBaseEntityComponentHandleWrapper; \
	}; \
	using name##Handle = name##ComponentHandleWrapper; \
	lua_registercheck(name,name##Handle);

#define DEFINE_LUA_COMPONENT_HANDLE(dllio,name) \
	DEFINE_LUA_COMPONENT_HANDLE_BASE(dllio,name,BaseEntityComponentHandleWrapper)

DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseActor);
DEFINE_LUA_COMPONENT_HANDLE_BASE(DLLNETWORK,BaseCharacter,BaseActorComponentHandleWrapper);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseAI);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePlayer);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseVehicle);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseWheel);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseWeapon);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseModel);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseAnimated);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseIO);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseAttachable);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseParent);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseTimeScale);

DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,Velocity);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,IK);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,Logic);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,Map);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,Usable);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,Submergible);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,Damageable);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,GlobalName);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFlammable);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFlex);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseHealth);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseName);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseNetworked);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseObservable);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePhysics);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseRender);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseSoftBody);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseSoundEmitter);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseTransform);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseColor);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseRadius);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseGeneric);

DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvSoundDsp);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvCamera);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvExplosion);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvFire);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvFogController);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvLight);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvLightDirectional);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvLightPoint);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvLightSpot);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvLightSpotVol);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvMicrophone);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvParticleSystem);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvQuake);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvSmokeTrail);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvSound);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvSoundScape);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvSprite);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvTimescale);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseInfoLandmark);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvWeather);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEnvWind);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFilterName);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFilterClass);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFuncBrush);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFuncButton);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFuncKinematic);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFuncPhysics);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFuncPortal);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFuncSoftPhysics);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFuncSurfaceMaterial);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFuncWater);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseLogicRelay);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseBot);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointPathNode);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointConstraintBallSocket);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointConstraintConeTwist);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointConstraintDoF);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointConstraintFixed);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointConstraintHinge);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointConstraintSlider);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointRenderTarget);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointTarget);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseProp);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePropDynamic);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePropPhysics);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseTriggerHurt);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseTriggerPush);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseTriggerRemove);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseTriggerTeleport);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseTouch);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseEntityTriggerGravity);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseFlashlight);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,Gravity);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseSkybox);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseToggle);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseWorld);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseShooter);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseOwnable);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseDebugText);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseDebugPoint);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseDebugLine);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseDebugBox);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseDebugSphere);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseDebugCone);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseDebugCylinder);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BaseDebugPlane);
DEFINE_LUA_COMPONENT_HANDLE(DLLNETWORK,BasePointAtTarget);

struct lua_State;
namespace Lua
{
	DLLNETWORK bool CheckComponentHandle(lua_State *l,const BaseEntityComponentHandle &handle);
};

#endif
