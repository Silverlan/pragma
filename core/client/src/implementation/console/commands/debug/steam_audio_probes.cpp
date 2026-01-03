// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :console.commands;

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
static std::unique_ptr<DebugGameGUI> dbgSoundProbeBoxes = nullptr;
static void debug_steam_audio_probe_boxes(pragma::NetworkState *state, ConVar *, bool, bool val)
{
	if(val == false) {
		dbgSoundProbeBoxes = nullptr;
		return;
	}
	auto *sndSys = pragma::get_cengine()->GetSoundSystem();
	if(sndSys == nullptr)
		return;
	auto *iplScene = sndSys->GetSteamAudioScene();
	if(iplScene == nullptr)
		return;
	if(iplScene->IsComplete() == false) {
		Con::CWAR << "Steam audio is still initializing. Cannot display probe boxes at this time!" << Con::endl;
		return;
	}
	std::vector<ipl::Scene::ProbeSphere> spheres {};
	iplScene->GetProbeSpheres(spheres);
	auto outlineColor = colors::Aqua;
	outlineColor.a = 64;

	dbgSoundProbeBoxes = std::make_unique<DebugGameGUI>(nullptr);
	std::vector<std::shared_ptr<DebugRenderer::BaseObject>> dbgSpheres;
	dbgSpheres.reserve(spheres.size());
	Con::COUT << "Number of probe spheres: " << spheres.size() << Con::endl;
	for(auto &sphere : spheres) {
		Con::COUT << "Sphere: (" << sphere.origin.x << "," << sphere.origin.y << "," << sphere.origin.z << ") (" << sphere.radius << ")" << Con::endl;
		auto o = DebugRenderer::DrawSphere(sphere.origin, sphere.radius, Color::AliceBlue, outlineColor);
		dbgSpheres.push_back(o);
	}
	dbgSoundProbeBoxes->CallOnRemove([dbgSpheres]() mutable { dbgSpheres = {}; });
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<bool>("debug_steam_audio_probe_boxes", &debug_steam_audio_probe_boxes);
}

static void debug_steam_audio_dump_scene(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv)
{
	auto *sndSys = pragma::get_cengine()->GetSoundSystem();
	if(sndSys == nullptr) {
		Con::CWAR << "Unable to dump steam audio scene: No sound system found!" << Con::endl;
		return;
	}
	auto *iplScene = sndSys->GetSteamAudioScene();
	if(iplScene == nullptr) {
		Con::CWAR << "Unable to dump steam audio scene: Steam audio has not been initialized!" << Con::endl;
		return;
	}
	std::string mapName = "unknown";
	auto *client = pragma::get_cengine()->GetClientState();
	auto *game = (pragma::get_client_state() != nullptr) ? static_cast<pragma::CGame *>(pragma::get_client_state()->GetGameState()) : nullptr;
	if(game != nullptr)
		mapName = game->GetMapName();

	std::string path = "debug/maps/";
	fs::create_path(path);
	path += mapName + "_steam_audio_scene.obj";
	auto r = iplScene->DumpScene(path);
	if(r == false) {
		Con::CWAR << "Unable to dump steam audio scene: Has steam audio scene been finalized?" << Con::endl;
		return;
	}
	Con::COUT << "Steam audio scene successfully dumped as '" << path << "'!" << Con::endl;
}

#else
static void debug_steam_audio_dump_scene(pragma::NetworkState *state, pragma::BasePlayerComponent *pl, std::vector<std::string> &argv) { Con::CWAR << "Steam audio is disabled! Scene cannot be dumped." << Con::endl; }
#endif

namespace {
	auto UVN = pragma::console::client::register_command("debug_steam_audio_dump_scene", &debug_steam_audio_dump_scene, pragma::console::ConVarFlags::None, "Saves the steam audio scene as OBJ file.");
}
