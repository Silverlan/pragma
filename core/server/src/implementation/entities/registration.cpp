// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.registration;

import :entities;
import :game;
import :server_state;

#undef GetClassName

using namespace server_entities;

void ServerEntityRegistry::RegisterEntity(const std::string &localName, std::type_index type, Factory creator)
{
	m_factories[type] = std::move(creator);
	m_classNameToTypeIndex.insert(std::make_pair(localName, type));
}

std::optional<std::string_view> ServerEntityRegistry::GetClassName(std::type_index type) const
{
	auto it = m_typeIndexToClassName.find(type);
	if(it == m_typeIndexToClassName.end())
		return {};
	return it->second;
}

void ServerEntityRegistry::GetRegisteredClassNames(std::vector<std::string> &outNames) const
{
	outNames.reserve(outNames.size() + m_classNameToTypeIndex.size());
	for(auto &[className, typeIndex] : m_classNameToTypeIndex)
		outNames.push_back(className);
}

Factory ServerEntityRegistry::FindFactory(const std::string &localName) const
{
	auto itType = m_classNameToTypeIndex.find(localName);
	if(itType == m_classNameToTypeIndex.end())
		return nullptr;
	return FindFactory(itType->second);
}

Factory ServerEntityRegistry::FindFactory(std::type_index type) const
{
	auto itFactory = m_factories.find(type);
	if(itFactory == m_factories.end())
		return nullptr;
	auto &factory = itFactory->second;
	return factory;
}

uint32_t ServerEntityRegistry::RegisterNetworkedEntity(std::type_index type)
{
	auto it = m_networkFactoryIDs.find(type);
	if(it != m_networkFactoryIDs.end())
		return it->second;
	uint32_t id = m_nextNetworkFactoryID++;
	m_networkFactoryIDs.emplace(type, id);
	return id;
}

std::optional<uint32_t> ServerEntityRegistry::GetNetworkFactoryID(std::type_index type) const
{
	auto it = m_networkFactoryIDs.find(type);
	if(it == m_networkFactoryIDs.end())
		return std::nullopt;
	return it->second;
}

void server_entities::register_entities()
{
	register_entity<Bot>("bot");
	register_entity<EntDebugBox>("debug_box");
	register_entity<EntDebugCone>("debug_cone");
	register_entity<EntDebugCylinder>("debug_cylinder");
	register_entity<EntDebugLine>("debug_line");
	register_entity<EntDebugText>("debug_text");
	register_entity<EntDebugPlane>("debug_plane");
	register_entity<EntDebugPoint>("debug_point");
	register_entity<EntDebugSphere>("debug_sphere");
	register_entity<SBaseEntity>("entity");
	register_entity<EnvCamera>("env_camera");
	register_entity<EnvDecal>("env_decal");
	register_entity<EnvExplosion>("env_explosion");
	register_entity<EnvFire>("env_fire");
	register_entity<EnvFogController>("env_fog_controller");
	register_entity<EnvLightDirectional>("env_light_environment");
	register_entity<EnvLightPoint>("env_light_point");
	register_entity<EnvLightSpot>("env_light_spot");
	register_entity<EnvLightSpotVol>("env_light_spot_vol");
	register_entity<EnvMicrophone>("env_microphone");
	register_entity<EnvParticleSystem>("env_particle_system");
	register_entity<EnvQuake>("env_quake");
	register_entity<EnvSmokeTrail>("env_smoke_trail");
	register_entity<EnvSprite>("env_sprite");
	register_entity<EnvSound>("env_sound");
	register_entity<EnvSoundDsp>("env_sound_dsp");
	register_entity<EnvSoundDspChorus>("env_sound_dsp_chorus");
	register_entity<EnvSoundDspDistortion>("env_sound_dsp_distortion");
	register_entity<EnvSoundDspEAXReverb>("env_sound_dsp_eaxreverb");
	register_entity<EnvSoundDspEcho>("env_sound_dsp_echo");
	register_entity<EnvSoundDspEqualizer>("env_sound_dsp_equalizer");
	register_entity<EnvSoundDspFlanger>("env_sound_dsp_flanger");
	register_entity<EnvSoundScape>("env_soundscape");
	register_entity<EnvTimescale>("env_timescale");
	register_entity<EnvWind>("env_wind");
	register_entity<FilterEntityClass>("filter_entity_class");
	register_entity<FilterEntityName>("filter_entity_name");
	register_entity<Flashlight>("flashlight");
	register_entity<FuncKinematic>("func_kinematic");
	register_entity<FuncPhysics>("func_physics");
	// register_entity<FuncPortal>("func_portal");
	register_entity<FuncSoftPhysics>("func_physics_softbody");
	register_entity<FuncButton>("func_button");
	register_entity<FuncBrush>("func_brush");
	register_entity<FuncWater>("func_water");
	register_entity<SGameEntity>("game");
	register_entity<GamePlayerSpawn>("game_player_spawn");
	register_entity<SGamemode>("gamemode");
	register_entity<InfoLandmark>("info_landmark");
	register_entity<LogicRelay>("logic_relay");
	register_entity<Player>("player");
	register_entity<PointConstraintBallSocket>("point_constraint_ballsocket");
	register_entity<PointConstraintConeTwist>("point_constraint_conetwist");
	register_entity<PointConstraintDoF>("point_constraint_dof");
	register_entity<PointConstraintFixed>("point_constraint_fixed");
	register_entity<PointConstraintHinge>("point_constraint_hinge");
	register_entity<PointConstraintSlider>("point_constraint_slider");
	register_entity<PointPathNode>("point_path_node");
	// register_entity<PointRenderTarget>("point_rendertarget");
	register_entity<PropDynamic>("prop_dynamic");
	register_entity<PropPhysics>("prop_physics");
	register_entity<PointTarget>("point_target");
	register_entity<Skybox>("skybox");
	register_entity<TriggerGravity>("trigger_gravity");
	register_entity<TriggerHurt>("trigger_hurt");
	register_entity<TriggerPush>("trigger_push");
	register_entity<TriggerRemove>("trigger_remove");
	register_entity<TriggerTeleport>("trigger_teleport");
	register_entity<TriggerTouch>("trigger_touch");
	register_entity<SWheel>("vhc_wheel");
	register_entity<World>("world");

	// Register networked entities
	// Note: These have to match the clientside registrations in the exact same order!
	register_networked_entity<World>();
	register_networked_entity<PropPhysics>();
	register_networked_entity<EnvParticleSystem>();
	register_networked_entity<EnvSound>();
	register_networked_entity<EnvLightSpot>();
	register_networked_entity<EnvLightDirectional>();
	register_networked_entity<EnvLightPoint>();
	register_networked_entity<Skybox>();
	register_networked_entity<Flashlight>();
	register_networked_entity<EnvCamera>();
	register_networked_entity<FuncButton>();
	register_networked_entity<EnvExplosion>();
	register_networked_entity<EnvQuake>();
	register_networked_entity<EnvSoundDsp>();
	register_networked_entity<EnvSoundDspEAXReverb>();
	register_networked_entity<EnvSoundDspChorus>();
	register_networked_entity<EnvSoundDspDistortion>();
	register_networked_entity<EnvSoundDspEcho>();
	register_networked_entity<EnvSoundDspFlanger>();
	register_networked_entity<EnvTimescale>();
	register_networked_entity<EnvFogController>();
	register_networked_entity<EnvLightSpotVol>();
	register_networked_entity<PointRenderTarget>();
	register_networked_entity<FuncPortal>();
	register_networked_entity<FuncWater>();
	register_networked_entity<FuncPhysics>();
	register_networked_entity<EnvSoundScape>();
	register_networked_entity<FuncBrush>();
	register_networked_entity<EnvMicrophone>();
	register_networked_entity<FuncKinematic>();
	register_networked_entity<Bot>();
	register_networked_entity<PointTarget>();
	register_networked_entity<SWheel>();
	register_networked_entity<PropDynamic>();
	register_networked_entity<EnvFire>();
	register_networked_entity<EnvSmokeTrail>();
	register_networked_entity<EnvSprite>();
	register_networked_entity<EnvDecal>();
	register_networked_entity<Player>();
	register_networked_entity<EntDebugText>();
	register_networked_entity<EntDebugPoint>();
	register_networked_entity<EntDebugLine>();
	register_networked_entity<EntDebugBox>();
	register_networked_entity<EntDebugSphere>();
	register_networked_entity<EntDebugCone>();
	register_networked_entity<EntDebugCylinder>();
	register_networked_entity<EntDebugPlane>();
	register_networked_entity<SGameEntity>();

	register_networked_entity<PointConstraintBallSocket>();
	register_networked_entity<PointConstraintConeTwist>();
	register_networked_entity<PointConstraintSlider>();
	register_networked_entity<PointConstraintHinge>();
	register_networked_entity<PointConstraintDoF>();
	register_networked_entity<PointConstraintFixed>();
	register_networked_entity<FuncSoftPhysics>();
}
