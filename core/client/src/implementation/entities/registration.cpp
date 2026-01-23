// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.registration;

#undef GetClassName

using namespace client_entities;

void ClientEntityRegistry::RegisterEntity(const std::string &localName, std::type_index type, Factory creator)
{
	m_factories[type] = std::move(creator);
	m_classNameToTypeIndex.insert(std::make_pair(localName, type));
}

std::optional<std::string_view> ClientEntityRegistry::GetClassName(std::type_index type) const
{
	auto it = m_typeIndexToClassName.find(type);
	if(it == m_typeIndexToClassName.end())
		return {};
	return it->second;
}

void ClientEntityRegistry::GetRegisteredClassNames(std::vector<std::string> &outNames) const
{
	outNames.reserve(outNames.size() + m_classNameToTypeIndex.size());
	for(auto &[className, typeIndex] : m_classNameToTypeIndex)
		outNames.push_back(className);
}

Factory ClientEntityRegistry::FindFactory(const std::string &localName) const
{
	auto itType = m_classNameToTypeIndex.find(localName);
	if(itType == m_classNameToTypeIndex.end())
		return nullptr;
	return FindFactory(itType->second);
}

Factory ClientEntityRegistry::FindFactory(std::type_index type) const
{
	auto itFactory = m_factories.find(type);
	if(itFactory == m_factories.end())
		return nullptr;
	auto &factory = itFactory->second;
	return factory;
}

uint32_t ClientEntityRegistry::RegisterNetworkedEntity(const NetworkedFactory &factory)
{
	auto ID = m_nextNetworkFactoryID++;
	m_networkedFactories.insert(std::make_pair(ID, factory));
	return ID;
}

const NetworkedFactory *ClientEntityRegistry::GetNetworkedFactory(uint32_t ID) const
{
	auto it = m_networkedFactories.find(ID);
	if(it == m_networkedFactories.end())
		return nullptr;
	return &it->second;
}

void client_entities::register_entities()
{
	register_entity<CBot>("bot");
	register_entity<CWaterSurface>("c_water_surface");
	register_entity<CEntDebugText>("debug_text");
	register_entity<CEntDebugPoint>("debug_point");
	register_entity<CEntDebugLine>("debug_line");
	register_entity<CEntDebugBox>("debug_box");
	register_entity<CEntDebugSphere>("debug_sphere");
	register_entity<CEntDebugCone>("debug_cone");
	register_entity<CEntDebugCylinder>("debug_cylinder");
	register_entity<CEntDebugPlane>("debug_plane");
	register_entity<pragma::ecs::CBaseEntity>("entity");
	register_entity<CEnvCamera>("env_camera");
	register_entity<CEnvDecal>("env_decal");
	register_entity<CEnvExplosion>("env_explosion");
	register_entity<CEnvFire>("env_fire");
	register_entity<CEnvFogController>("env_fog_controller");
	register_entity<CEnvLightDirectional>("env_light_environment");
	register_entity<CEnvLightPoint>("env_light_point");
	register_entity<CEnvLightSpot>("env_light_spot");
	register_entity<CEnvLightSpotVol>("env_light_spot_vol");
	register_entity<CEnvMicrophone>("env_microphone");
	register_entity<pragma::ecs::CEnvParticleSystem>("env_particle_system");
	register_entity<CEnvQuake>("env_quake");
	register_entity<CEnvReflectionProbe>("env_reflection_probe");
	register_entity<CEnvSmokeTrail>("env_smoke_trail");
	register_entity<CEnvSound>("env_sound");
	register_entity<CEnvSoundDsp>("env_sound_dsp");
	register_entity<CEnvSoundDspChorus>("env_sound_dsp_chorus");
	register_entity<CEnvSoundDspDistortion>("env_sound_dsp_distortion");
	register_entity<CEnvSoundDspEAXReverb>("env_sound_dsp_eaxreverb");
	register_entity<CEnvSoundDspEcho>("env_sound_dsp_echo");
	register_entity<CEnvSoundDspEqualizer>("env_sound_dsp_equalizer");
	register_entity<CEnvSoundDspFlanger>("env_sound_dsp_flanger");
	register_entity<CEnvSoundProbe>("env_sound_probe");
	register_entity<CEnvSoundScape>("env_soundscape");
	register_entity<CEnvSprite>("env_sprite");
	register_entity<CEnvTimescale>("env_timescale");
	register_entity<CEnvWeather>("env_weather");
	register_entity<CEnvWind>("env_wind");
	register_entity<CFlashlight>("flashlight");
	register_entity<CFilterEntityClass>("filter_entity_class");
	register_entity<CFilterEntityName>("filter_entity_name");
	register_entity<CFuncBrush>("func_brush");
	register_entity<CFuncButton>("func_button");
	register_entity<CFuncKinematic>("func_kinematic");
	register_entity<CFuncPhysics>("func_physics");
	register_entity<CFuncSoftPhysics>("func_physics_softbody");
	register_entity<CFuncPortal>("func_portal");
	register_entity<CFuncWater>("func_water");
	register_entity<CGameEntity>("game");
	register_entity<COcclusionCuller>("game_occlusion_culler");
	register_entity<CShadowManager>("game_shadow_manager");
	register_entity<CGamemode>("gamemode");
	register_entity<CListener>("listener");
	register_entity<CPlayer>("player");
	register_entity<CPointConstraintBallSocket>("point_constraint_ballsocket");
	register_entity<CPointConstraintConeTwist>("point_constraint_conetwist");
	register_entity<CPointConstraintDoF>("point_constraint_dof");
	register_entity<CPointConstraintFixed>("point_constraint_fixed");
	register_entity<CPointConstraintHinge>("point_constraint_hinge");
	register_entity<CPointConstraintSlider>("point_constraint_slider");
	register_entity<CPointRenderTarget>("point_rendertarget");
	register_entity<CPointTarget>("point_target");
	register_entity<CPropDynamic>("prop_dynamic");
	register_entity<CPropPhysics>("prop_physics");
	register_entity<CRasterizationRenderer>("rasterization_renderer");
	register_entity<CRaytracingRenderer>("raytracing_renderer");
	register_entity<CScene>("scene");
	register_entity<CSkyCamera>("sky_camera");
	register_entity<CSkybox>("skybox");
	register_entity<CTriggerTouch>("trigger_touch");
	register_entity<CUtilPBRConverter>("util_pbr_converter");
	register_entity<CWheel>("vhc_wheel");
	register_entity<CViewBody>("viewbody");
	register_entity<CViewModel>("viewmodel");
	register_entity<CWorld>("world");

	// Register networked entities
	// Note: These have to match the serverside registrations in the exact same order!
	register_networked_entity<CWorld>();
	register_networked_entity<CPropPhysics>();
	register_networked_entity<pragma::ecs::CEnvParticleSystem>();
	register_networked_entity<CEnvSound>();
	register_networked_entity<CEnvLightSpot>();
	register_networked_entity<CEnvLightDirectional>();
	register_networked_entity<CEnvLightPoint>();
	register_networked_entity<CSkybox>();
	register_networked_entity<CFlashlight>();
	register_networked_entity<CEnvCamera>();
	register_networked_entity<CFuncButton>();
	register_networked_entity<CEnvExplosion>();
	register_networked_entity<CEnvQuake>();
	register_networked_entity<CEnvSoundDsp>();
	register_networked_entity<CEnvSoundDspEAXReverb>();
	register_networked_entity<CEnvSoundDspChorus>();
	register_networked_entity<CEnvSoundDspDistortion>();
	register_networked_entity<CEnvSoundDspEcho>();
	register_networked_entity<CEnvSoundDspFlanger>();
	register_networked_entity<CEnvTimescale>();
	register_networked_entity<CEnvFogController>();
	register_networked_entity<CEnvLightSpotVol>();
	register_networked_entity<CPointRenderTarget>();
	register_networked_entity<CFuncPortal>();
	register_networked_entity<CFuncWater>();
	register_networked_entity<CFuncPhysics>();
	register_networked_entity<CEnvSoundScape>();
	register_networked_entity<CFuncBrush>();
	register_networked_entity<CEnvMicrophone>();
	register_networked_entity<CFuncKinematic>();
	register_networked_entity<CBot>();
	register_networked_entity<CPointTarget>();
	register_networked_entity<CWheel>();
	register_networked_entity<CPropDynamic>();
	register_networked_entity<CEnvFire>();
	register_networked_entity<CEnvSmokeTrail>();
	register_networked_entity<CEnvSprite>();
	register_networked_entity<CEnvDecal>();
	register_networked_entity<CPlayer>();
	register_networked_entity<CEntDebugText>();
	register_networked_entity<CEntDebugPoint>();
	register_networked_entity<CEntDebugLine>();
	register_networked_entity<CEntDebugBox>();
	register_networked_entity<CEntDebugSphere>();
	register_networked_entity<CEntDebugCone>();
	register_networked_entity<CEntDebugCylinder>();
	register_networked_entity<CEntDebugPlane>();
	register_networked_entity<CGameEntity>();

	register_networked_entity<CPointConstraintBallSocket>();
	register_networked_entity<CPointConstraintConeTwist>();
	register_networked_entity<CPointConstraintSlider>();
	register_networked_entity<CPointConstraintHinge>();
	register_networked_entity<CPointConstraintDoF>();
	register_networked_entity<CPointConstraintFixed>();
	register_networked_entity<CFuncSoftPhysics>();
}
