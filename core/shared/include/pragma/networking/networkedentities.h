// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#define LINK_NETWORKED_PHYSICS_ENTITIES                                                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(PointConstraintBallSocket, CPointConstraintBallSocket);                                                                                                                                                                                                                \
	LINK_NETWORKED_ENTITY(PointConstraintConeTwist, CPointConstraintConeTwist);                                                                                                                                                                                                                  \
	LINK_NETWORKED_ENTITY(PointConstraintSlider, CPointConstraintSlider);                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(PointConstraintHinge, CPointConstraintHinge);                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(PointConstraintDoF, CPointConstraintDoF);                                                                                                                                                                                                                              \
	LINK_NETWORKED_ENTITY(PointConstraintFixed, CPointConstraintFixed);                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(FuncSoftPhysics, CFuncSoftPhysics);

#define LINK_NETWORKED_ENTITIES                                                                                                                                                                                                                                                                  \
	LINK_NETWORKED_ENTITY(World, CWorld);                                                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(PropPhysics, CPropPhysics);                                                                                                                                                                                                                                            \
	LINK_NETWORKED_ENTITY(EnvParticleSystem, CEnvParticleSystem);                                                                                                                                                                                                                                \
	LINK_NETWORKED_ENTITY(EnvSound, CEnvSound);                                                                                                                                                                                                                                                  \
	LINK_NETWORKED_ENTITY(EnvLightSpot, CEnvLightSpot);                                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(EnvLightDirectional, CEnvLightDirectional);                                                                                                                                                                                                                            \
	LINK_NETWORKED_ENTITY(EnvLightPoint, CEnvLightPoint);                                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(Skybox, CSkybox);                                                                                                                                                                                                                                                      \
	LINK_NETWORKED_ENTITY(Flashlight, CFlashlight);                                                                                                                                                                                                                                              \
	LINK_NETWORKED_ENTITY(EnvCamera, CEnvCamera);                                                                                                                                                                                                                                                \
	LINK_NETWORKED_ENTITY(FuncButton, CFuncButton);                                                                                                                                                                                                                                              \
	LINK_NETWORKED_ENTITY(EnvExplosion, CEnvExplosion);                                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(EnvQuake, CEnvQuake);                                                                                                                                                                                                                                                  \
	LINK_NETWORKED_ENTITY(EnvSoundDsp, CEnvSoundDsp);                                                                                                                                                                                                                                            \
	LINK_NETWORKED_ENTITY(EnvSoundDspEAXReverb, CEnvSoundDspEAXReverb);                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(EnvSoundDspChorus, CEnvSoundDspChorus);                                                                                                                                                                                                                                \
	LINK_NETWORKED_ENTITY(EnvSoundDspDistortion, CEnvSoundDspDistortion);                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(EnvSoundDspEcho, CEnvSoundDspEcho);                                                                                                                                                                                                                                    \
	LINK_NETWORKED_ENTITY(EnvSoundDspFlanger, CEnvSoundDspFlanger);                                                                                                                                                                                                                              \
	LINK_NETWORKED_ENTITY(EnvTimescale, CEnvTimescale);                                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(EnvFogController, CEnvFogController);                                                                                                                                                                                                                                  \
	LINK_NETWORKED_ENTITY(EnvLightSpotVol, CEnvLightSpotVol);                                                                                                                                                                                                                                    \
	LINK_NETWORKED_ENTITY(PointRenderTarget, CPointRenderTarget);                                                                                                                                                                                                                                \
	LINK_NETWORKED_ENTITY(FuncPortal, CFuncPortal);                                                                                                                                                                                                                                              \
	LINK_NETWORKED_ENTITY(FuncWater, CFuncWater);                                                                                                                                                                                                                                                \
	LINK_NETWORKED_ENTITY(FuncPhysics, CFuncPhysics);                                                                                                                                                                                                                                            \
	LINK_NETWORKED_ENTITY(EnvSoundScape, CEnvSoundScape);                                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(FuncBrush, CFuncBrush);                                                                                                                                                                                                                                                \
	LINK_NETWORKED_ENTITY(EnvMicrophone, CEnvMicrophone);                                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(FuncKinematic, CFuncKinematic);                                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(Bot, CBot);                                                                                                                                                                                                                                                            \
	LINK_NETWORKED_ENTITY(PointTarget, CPointTarget);                                                                                                                                                                                                                                            \
	LINK_NETWORKED_ENTITY(SWheel, CWheel);                                                                                                                                                                                                                                                       \
	LINK_NETWORKED_ENTITY(PropDynamic, CPropDynamic);                                                                                                                                                                                                                                            \
	LINK_NETWORKED_ENTITY(EnvFire, CEnvFire);                                                                                                                                                                                                                                                    \
	LINK_NETWORKED_ENTITY(EnvSmokeTrail, CEnvSmokeTrail);                                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(EnvSprite, CEnvSprite);                                                                                                                                                                                                                                                \
	LINK_NETWORKED_ENTITY(EnvDecal, CEnvDecal);                                                                                                                                                                                                                                                  \
	LINK_NETWORKED_ENTITY(Player, CPlayer);                                                                                                                                                                                                                                                      \
	LINK_NETWORKED_ENTITY(EntDebugText, CEntDebugText);                                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(EntDebugPoint, CEntDebugPoint);                                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(EntDebugLine, CEntDebugLine);                                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(EntDebugBox, CEntDebugBox);                                                                                                                                                                                                                                            \
	LINK_NETWORKED_ENTITY(EntDebugSphere, CEntDebugSphere);                                                                                                                                                                                                                                      \
	LINK_NETWORKED_ENTITY(EntDebugCone, CEntDebugCone);                                                                                                                                                                                                                                          \
	LINK_NETWORKED_ENTITY(EntDebugCylinder, CEntDebugCylinder);                                                                                                                                                                                                                                  \
	LINK_NETWORKED_ENTITY(EntDebugPlane, CEntDebugPlane);                                                                                                                                                                                                                                        \
	LINK_NETWORKED_ENTITY(SGameEntity, CGameEntity);

#ifdef DLLSERVER_EX
#include "pragma/entities/s_entityfactories.h"
#include "pragma/entities/player.h"
#include <pragma/entities/world.h>
#include "pragma/entities/s_skybox.h"
#include "pragma/entities/s_flashlight.h"
#include "pragma/entities/s_npc_dragonworm.h"
#include "pragma/entities/s_bot.h"
#include "pragma/entities/s_ent_debug.hpp"

import pragma.server.entities.components;

LINK_NETWORKED_ENTITIES;
LINK_NETWORKED_PHYSICS_ENTITIES;
#elif DLLCLIENT_EX
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/c_world.h"
#include "pragma/entities/environment/effects/c_env_particle_system.h"
#include "pragma/entities/environment/audio/c_env_sound.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/environment/effects/c_env_explosion.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_eaxreverb.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_chorus.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_distortion.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_echo.h"
#include "pragma/entities/environment/audio/c_env_sound_dsp_flanger.h"
#include "pragma/entities/environment/effects/c_env_sprite.h"
#include "pragma/entities/environment/lights/c_env_light_spot_vol.h"
#include "pragma/entities/components/liquid/c_liquid_component.hpp"
#include "pragma/entities/trigger/c_trigger_touch.h"
#include "pragma/entities/environment/audio/c_env_soundscape.h"
#include "pragma/entities/c_wheel.hpp"
#include "pragma/entities/environment/effects/c_env_fire.h"
#include "pragma/entities/components/c_game_component.hpp"
#include "pragma/entities/c_ent_debug.hpp"
#include "pragma/entities/environment/effects/c_env_smoke_trail.h"

import pragma.client.entities.components;

LINK_NETWORKED_ENTITIES;
LINK_NETWORKED_PHYSICS_ENTITIES;
#endif
