/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/console/c_cvar.h"
#include "pragma/entities/environment/audio/c_env_sound.h"
#include "pragma/entities/environment/audio/c_env_sound_probe.hpp"
#include "pragma/entities/func/c_func_brush.h"
#include <pragma/entities/components/base_physics_component.hpp>
#include <pragma/entities/components/base_transform_component.hpp>
#include <pragma/physics/environment.hpp>
#include <pragma/physics/shape.hpp>
#include <pragma/physics/collisionmesh.h>
#include <alsoundsystem.hpp>
#include <steam_audio/alsound_steam_audio.hpp>
#include <alsound_coordinate_system.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;

void CGame::ClearSoundCache()
{
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys != nullptr)
		soundSys->ClearSteamAudioScene();
#endif
}

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
static void steam_audio_message_callback(ipl::Scene::LoadStage stage, float progress)
{
	switch(stage) {
	case ipl::Scene::LoadStage::LoadFromDisk:
		std::cout << "[STEAM AUDIO] Loading steam audio data... " << (progress * 100.f) << "%" << std::endl;
		break;
	case ipl::Scene::LoadStage::FinalizingScene:
		std::cout << "[STEAM AUDIO] Finalizing scene... " << (progress * 100.f) << "%" << std::endl;
		break;
	case ipl::Scene::LoadStage::GeneratingProbes:
		std::cout << "[STEAM AUDIO] Generating probes... " << (progress * 100.f) << "%" << std::endl;
		break;
	case ipl::Scene::LoadStage::BakingReverb:
		std::cout << "[STEAM AUDIO] Baking reverb... " << (progress * 100.f) << "%" << std::endl;
		break;
	case ipl::Scene::LoadStage::BakingPropagation:
		std::cout << "[STEAM AUDIO] Baking propagation... " << (progress * 100.f) << "%" << std::endl;
		break;
	};
}
static void steam_audio_error_callback(IPLerror err) { Con::cwar << "[STEAM AUDIO] Error trying to finalize scene: " << err << Con::endl; }
static void steam_audio_finalized_callback()
{
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	soundSys->SetSteamAudioEnabled(true);
}
#endif

static auto cvSteamAudioEnabled = GetClientConVar("cl_steam_audio_enabled");
static auto cvSteamAudioNumRays = GetClientConVar("cl_steam_audio_number_of_rays");
static auto cvSteamAudioNumDiffuseSamples = GetClientConVar("cl_steam_audio_number_of_diffuse_samples");
static auto cvSteamAudioNumBounces = GetClientConVar("cl_steam_audio_number_of_bounces");
static auto cvSteamAudioIrDuration = GetClientConVar("cl_steam_audio_ir_duration");
static auto cvSteamAudioAmbisonicsOrder = GetClientConVar("cl_steam_audio_ambisonics_order");
void CGame::ReloadSoundCache(bool bReloadBakedCache, SoundCacheFlags cacheFlags, float spacing)
{
	ClearSoundCache();
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	if(cvSteamAudioEnabled->GetBool() == false) // See also ClientState::Initialize
		return;
	auto map = GetMapName();
	if(map.empty())
		return;
	auto *soundSys = c_engine->GetSoundSystem();
	if(soundSys != nullptr) {
		auto *iplScene = soundSys->InitializeSteamAudioScene();
		if(iplScene != nullptr) {
			auto &simSettings = iplScene->GetSimulationSettings();
			simSettings.numRays = umath::clamp(cvSteamAudioNumRays->GetInt(), 1'024, 131'072);
			simSettings.numDiffuseSamples = umath::clamp(cvSteamAudioNumDiffuseSamples->GetInt(), 32, 4'096);
			simSettings.numBounces = umath::clamp(cvSteamAudioNumBounces->GetInt(), 1, 32);
			simSettings.irDuration = umath::clamp(cvSteamAudioIrDuration->GetFloat(), 0.5f, 4.f);
			simSettings.ambisonicsOrder = umath::clamp(cvSteamAudioAmbisonicsOrder->GetInt(), 0, 3);

			auto info = ipl::Scene::FinalizeInfo {};
			info.flags = ipl::Scene::InitializeFlags::None;
			if((cacheFlags & CGame::SoundCacheFlags::BakeConvolution) != CGame::SoundCacheFlags::None)
				info.flags |= ipl::Scene::InitializeFlags::BakeConvolution;
			if((cacheFlags & CGame::SoundCacheFlags::BakeParametric) != CGame::SoundCacheFlags::None)
				info.flags |= ipl::Scene::InitializeFlags::BakeParametric;
			info.defaultSpacing = spacing;

			auto steamCachePath = "maps\\" + map + ".sta";
			if(bReloadBakedCache == false) {
				auto f = FileManager::OpenFile(steamCachePath.c_str(), "rb");
				if(f != nullptr) {
					iplScene->Finalize(
					  f, info, steam_audio_message_callback,
					  [iplScene]() {
						  Con::cout << "[STEAM AUDIO] Scene has been finalized!" << Con::endl;
						  steam_audio_finalized_callback();
					  },
					  steam_audio_error_callback);
				}
				else
					bReloadBakedCache = true;
			}
			if(bReloadBakedCache == true) {
				std::unordered_map<uint32_t, uint32_t> surfMatToIplMat {};
				auto fAddEntityMeshes = [this, iplScene, &surfMatToIplMat](CBaseEntity *ent) {
					auto &mdl = ent->GetModel();
					if(mdl == nullptr)
						return;
					auto pPhysComponent = ent->GetPhysicsComponent();
					auto pTrComponent = ent->GetTransformComponent();
					auto *phys = pPhysComponent != nullptr ? pPhysComponent->GetPhysicsObject() : nullptr;
					if(phys == nullptr || pTrComponent == nullptr)
						return;
					auto &pos = pTrComponent->GetPosition();
					auto &rot = pTrComponent->GetRotation();
					auto &hColObjs = phys->GetCollisionObjects();
					for(auto &hColObj : hColObjs) {
						if(hColObj.IsValid() == false)
							continue;
						auto shape = hColObj->GetCollisionShape();
						if(shape == nullptr)
							continue;
						auto bConvex = shape->IsConvex();
						auto bTriangle = shape->IsTriangleShape();
						if(bConvex == false && bTriangle == false)
							continue;
						auto surfMatId = hColObj->GetSurfaceMaterial();
						auto it = surfMatToIplMat.find(surfMatId);
						if(it == surfMatToIplMat.end()) {
							auto *surfMat = GetSurfaceMaterial(surfMatId);

							IPLMaterial mat {};
							if(surfMat != nullptr) {
								mat.lowFreqAbsorption = surfMat->GetAudioLowFrequencyAbsorption();
								mat.midFreqAbsorption = surfMat->GetAudioMidFrequencyAbsorption();
								mat.highFreqAbsorption = surfMat->GetAudioHighFrequencyAbsorption();
								mat.scattering = surfMat->GetAudioScattering();
								mat.lowFreqTransmission = surfMat->GetAudioLowFrequencyTransmission();
								mat.midFreqTransmission = surfMat->GetAudioMidFrequencyTransmission();
								mat.highFreqTransmission = surfMat->GetAudioHighFrequencyTransmission();
							}
							else
								mat = {0.10f, 0.20f, 0.30f, 0.05f, 0.100f, 0.050f, 0.030f}; // These should correspond to the values specified in "surfacematerial.h"
							auto matId = iplScene->AddMaterial(mat);
							it = surfMatToIplMat.insert(std::make_pair(surfMatId, matId)).first;
						}

						if(bConvex == true) {
							auto &convexShape = *shape->GetConvexShape();
							auto *colMesh = convexShape.GetCollisionMesh();
							if(colMesh != nullptr) {
								auto mesh = iplScene->CreateStaticMesh();

								auto &iplVerts = mesh->GetVertices();
								auto &iplTris = mesh->GetTriangles();
								auto &iplMatIndices = mesh->GetMaterialIndices();

								auto &verts = colMesh->GetVertices();
								auto &indices = colMesh->GetTriangles();

								iplVerts.reserve(verts.size());
								for(auto v : verts) {
									uvec::local_to_world(pos, rot, v);
									iplVerts.push_back(al::to_custom_vector<IPLVector3>(al::to_audio_position(v)));
								}

								iplTris.reserve(indices.size() / 3);
								for(auto i = decltype(indices.size()) {0}; i < indices.size(); i += 3) {
									iplTris.push_back({});
									auto &tri = iplTris.back();
									tri.indices[0] = indices.at(i);
									tri.indices[1] = indices.at(i + 1);
									tri.indices[2] = indices.at(i + 2);
								}
								iplMatIndices.resize(indices.size() / 3, it->second);
							}
						}
						else {
							auto mesh = iplScene->CreateStaticMesh();

							auto &iplVerts = mesh->GetVertices();
							auto &iplTris = mesh->GetTriangles();
							auto &iplMatIndices = mesh->GetMaterialIndices();

							auto &triShape = *shape->GetTriangleShape();
							auto verts = triShape.GetVertices();
							auto &triangles = triShape.GetTriangles();
							auto &materialIndices = triShape.GetSurfaceMaterials();

							iplVerts.reserve(verts.size());
							auto vertIdx = 0u;
							for(auto &v : verts) {
								uvec::local_to_world(pos, rot, v);
								iplVerts.push_back(al::to_custom_vector<IPLVector3>(al::to_audio_position(v)));
							}
							iplTris.reserve(triangles.size() / 3);
							for(auto i = decltype(triangles.size()) {0}; i < triangles.size(); i += 3) {
								iplTris.push_back({});
								auto &tri = iplTris.back();
								tri.indices[0] = triangles.at(i);
								tri.indices[1] = triangles.at(i + 1);
								tri.indices[2] = triangles.at(i + 2);
							}
							iplMatIndices = materialIndices;
						}
					}
				};
				auto *pWorld = GetWorld();
				if(pWorld != nullptr)
					fAddEntityMeshes(&static_cast<CBaseEntity &>(pWorld->GetEntity()));

				EntityIterator entIt {*this};
				entIt.AttachFilter<EntityIteratorFilterClass>("func_brush");
				for(auto *ent : entIt) {
					auto *entBrush = dynamic_cast<CFuncBrush *>(ent);
					if(entBrush == nullptr)
						continue;
					fAddEntityMeshes(entBrush);
				}

				/*auto &sounds = client->GetSounds();
				for(auto &sndRef : sounds)
				{
					auto *snd = static_cast<CALSound*>(&sndRef.get());
					if(snd == nullptr || snd->IsRelative() == true)
						continue;
					auto &identifier = snd->GetIdentifier();
					if(identifier.empty())
						continue;
					iplScene->RegisterPropagationSoundSource(identifier,snd->GetPosition(),snd->GetMaxDistance());
				}*/

				// Entity sounds may not have been created yet; Add them indirectly
				EntityIterator entItSnd {*this};
				entItSnd.AttachFilter<EntityIteratorFilterClass>("env_sound");
				for(auto *ent : entItSnd) {
					auto pTrComponent = ent->GetTransformComponent();

					auto *pSoundComponent = (ent != nullptr) ? static_cast<pragma::CSoundComponent *>(ent->FindComponent("sound").get()) : nullptr;
					if(pTrComponent == nullptr || pSoundComponent == nullptr)
						continue;
					auto pAttComponent = ent->GetComponent<pragma::CAttachableComponent>();
					if(pAttComponent.valid() && pAttComponent->GetParent() != nullptr)
						continue;
					auto name = pSoundComponent->GetSteamAudioIdentifier();
					//if(name.empty())
					//	name = "world_sound" +std::to_string(entSnd->GetMapIndex()); // Has to correspond to identifier in c_alsound.cpp
					iplScene->RegisterPropagationSoundSource(name, pTrComponent->GetPosition(), pSoundComponent->GetMaxDistance());
				}

				auto &probes = pragma::CEnvSoundProbeComponent::GetProbes();
				for(auto &probe : probes) {
					switch(probe.placement) {
					case pragma::CEnvSoundProbeComponent::Placement::Centroid:
						iplScene->AddProbeSphere(probe.min, probe.spacing);
						break;
					case pragma::CEnvSoundProbeComponent::Placement::Octree:
						Con::cwar << "Octree sound probes currently not supported!" << Con::endl;
						break;
					case pragma::CEnvSoundProbeComponent::Placement::UniformFloor:
						iplScene->AddProbeBox(probe.min, probe.max, probe.spacing, probe.heightAboveFloor);
						break;
					}
				}

				auto bSaveProbeBoxes = (cacheFlags & CGame::SoundCacheFlags::SaveProbeBoxes) != CGame::SoundCacheFlags::None;
				iplScene->Finalize(
				  info, steam_audio_message_callback,
				  [iplScene, steamCachePath, bSaveProbeBoxes]() {
					  Con::cout << "[STEAM AUDIO] Scene has been finalized!" << Con::endl;
					  auto f = FileManager::OpenFile<VFilePtrReal>(steamCachePath.c_str(), "wb");
					  if(f != nullptr) {
						  iplScene->Save(f, bSaveProbeBoxes);
						  Con::cout << "[STEAM AUDIO] Scene has been saved as '" << steamCachePath << "'!" << Con::endl;
					  }
					  else
						  Con::cwar << "[STEAM AUDIO] WARNING: Unable to save scene as '" << steamCachePath << "'!" << Con::endl;
					  steam_audio_finalized_callback();
				  },
				  steam_audio_error_callback);
			}
		}
	}
#endif
}
