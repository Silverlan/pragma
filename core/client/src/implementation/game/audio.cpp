// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :game;
import :client_state;
import :engine;
import :entities.components;

void pragma::CGame::ClearSoundCache()
{
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
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
static void steam_audio_error_callback(IPLerror err) { Con::CWAR << "[STEAM AUDIO] Error trying to finalize scene: " << err << Con::endl; }
static void steam_audio_finalized_callback()
{
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys == nullptr)
		return;
	soundSys->SetSteamAudioEnabled(true);
}
#endif

static auto cvSteamAudioEnabled = pragma::console::get_client_con_var("cl_steam_audio_enabled");
static auto cvSteamAudioNumRays = pragma::console::get_client_con_var("cl_steam_audio_number_of_rays");
static auto cvSteamAudioNumDiffuseSamples = pragma::console::get_client_con_var("cl_steam_audio_number_of_diffuse_samples");
static auto cvSteamAudioNumBounces = pragma::console::get_client_con_var("cl_steam_audio_number_of_bounces");
static auto cvSteamAudioIrDuration = pragma::console::get_client_con_var("cl_steam_audio_ir_duration");
static auto cvSteamAudioAmbisonicsOrder = pragma::console::get_client_con_var("cl_steam_audio_ambisonics_order");
void pragma::CGame::ReloadSoundCache(bool bReloadBakedCache, SoundCacheFlags cacheFlags, float spacing)
{
	ClearSoundCache();
#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
	if(cvSteamAudioEnabled->GetBool() == false) // See also ClientState::Initialize
		return;
	auto map = GetMapName();
	if(map.empty())
		return;
	auto *soundSys = pragma::get_cengine()->GetSoundSystem();
	if(soundSys != nullptr) {
		auto *iplScene = soundSys->InitializeSteamAudioScene();
		if(iplScene != nullptr) {
			auto &simSettings = iplScene->GetSimulationSettings();
			simSettings.numRays = pragma::math::clamp(cvSteamAudioNumRays->GetInt(), 1'024, 131'072);
			simSettings.numDiffuseSamples = pragma::math::clamp(cvSteamAudioNumDiffuseSamples->GetInt(), 32, 4'096);
			simSettings.numBounces = pragma::math::clamp(cvSteamAudioNumBounces->GetInt(), 1, 32);
			simSettings.irDuration = pragma::math::clamp(cvSteamAudioIrDuration->GetFloat(), 0.5f, 4.f);
			simSettings.ambisonicsOrder = pragma::math::clamp(cvSteamAudioAmbisonicsOrder->GetInt(), 0, 3);

			auto info = ipl::Scene::FinalizeInfo {};
			info.flags = ipl::Scene::InitializeFlags::None;
			if((cacheFlags & pragma::CGame::SoundCacheFlags::BakeConvolution) != pragma::CGame::SoundCacheFlags::None)
				info.flags |= ipl::Scene::InitializeFlags::BakeConvolution;
			if((cacheFlags & pragma::CGame::SoundCacheFlags::BakeParametric) != pragma::CGame::SoundCacheFlags::None)
				info.flags |= ipl::Scene::InitializeFlags::BakeParametric;
			info.defaultSpacing = spacing;

			auto steamCachePath = "maps\\" + map + ".sta";
			if(bReloadBakedCache == false) {
				auto f = pragma::fs::open_file(steamCachePath.c_str(), pragma::fs::FileMode::Read | pragma::fs::FileMode::Binary);
				if(f != nullptr) {
					iplScene->Finalize(
					  f, info, steam_audio_message_callback,
					  [iplScene]() {
						  Con::COUT << "[STEAM AUDIO] Scene has been finalized!" << Con::endl;
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
									iplVerts.push_back(pragma::audio::to_custom_vector<IPLVector3>(pragma::audio::to_audio_position(v)));
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
								iplVerts.push_back(pragma::audio::to_custom_vector<IPLVector3>(pragma::audio::to_audio_position(v)));
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
					fAddEntityMeshes(&static_cast<pragma::ecs::CBaseEntity &>(pWorld->GetEntity()));

				pragma::ecs::EntityIterator entIt {*this};
				entIt.AttachFilter<EntityIteratorFilterClass>("func_brush");
				for(auto *ent : entIt) {
					auto *entBrush = dynamic_cast<CFuncBrush *>(ent);
					if(entBrush == nullptr)
						continue;
					fAddEntityMeshes(entBrush);
				}

				/*auto &sounds = pragma::get_client_state()->GetSounds();
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
				pragma::ecs::EntityIterator entItSnd {*this};
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
						Con::CWAR << "Octree sound probes currently not supported!" << Con::endl;
						break;
					case pragma::CEnvSoundProbeComponent::Placement::UniformFloor:
						iplScene->AddProbeBox(probe.min, probe.max, probe.spacing, probe.heightAboveFloor);
						break;
					}
				}

				auto bSaveProbeBoxes = (cacheFlags & pragma::CGame::SoundCacheFlags::SaveProbeBoxes) != pragma::CGame::SoundCacheFlags::None;
				iplScene->Finalize(
				  info, steam_audio_message_callback,
				  [iplScene, steamCachePath, bSaveProbeBoxes]() {
					  Con::COUT << "[STEAM AUDIO] Scene has been finalized!" << Con::endl;
					  auto f = fs::open_file<fs::VFilePtrReal>(steamCachePath, fs::FileMode::Write | fs::FileMode::Binary);
					  if(f != nullptr) {
						  iplScene->Save(f, bSaveProbeBoxes);
						  Con::COUT << "[STEAM AUDIO] Scene has been saved as '" << steamCachePath << "'!" << Con::endl;
					  }
					  else
						  Con::CWAR << "[STEAM AUDIO] WARNING: Unable to save scene as '" << steamCachePath << "'!" << Con::endl;
					  steam_audio_finalized_callback();
				  },
				  steam_audio_error_callback);
			}
		}
	}
#endif
}
