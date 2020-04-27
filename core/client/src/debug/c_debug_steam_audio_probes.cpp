/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/debug/c_debugoverlay.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/console/c_cvar_global_functions.h"
#include <alsoundsystem.hpp>
#include <steam_audio/alsound_steam_audio.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

#if ALSYS_STEAM_AUDIO_SUPPORT_ENABLED == 1
static std::unique_ptr<DebugGameGUI> dbgSoundProbeBoxes = nullptr;
static void debug_steam_audio_probe_boxes(NetworkState *state,ConVar*,bool,bool val)
{
	if(val == false)
	{
		dbgSoundProbeBoxes = nullptr;
		return;
	}
	auto *sndSys = c_engine->GetSoundSystem();
	if(sndSys == nullptr)
		return;
	auto *iplScene = sndSys->GetSteamAudioScene();
	if(iplScene == nullptr)
		return;
	if(iplScene->IsComplete() == false)
	{
		Con::cwar<<"WARNING: Steam audio is still initializing. Cannot display probe boxes at this time!"<<Con::endl;
		return;
	}
	std::vector<ipl::Scene::ProbeSphere> spheres {};
	iplScene->GetProbeSpheres(spheres);
	auto outlineColor = Color::Aqua;
	outlineColor.a = 64;

	dbgSoundProbeBoxes = std::make_unique<DebugGameGUI>(nullptr);
	std::vector<std::shared_ptr<DebugRenderer::BaseObject>> dbgSpheres;
	dbgSpheres.reserve(spheres.size());
	Con::cout<<"Number of probe spheres: "<<spheres.size()<<Con::endl;
	for(auto &sphere : spheres)
	{
		Con::cout<<"Sphere: ("<<sphere.origin.x<<","<<sphere.origin.y<<","<<sphere.origin.z<<") ("<<sphere.radius<<")"<<Con::endl;
		auto o = DebugRenderer::DrawSphere(sphere.origin,sphere.radius,Color::AliceBlue,outlineColor);
		dbgSpheres.push_back(o);
	}
	dbgSoundProbeBoxes->CallOnRemove([dbgSpheres]() mutable {
		dbgSpheres = {};
	});
}
REGISTER_CONVAR_CALLBACK_CL(debug_steam_audio_probe_boxes,debug_steam_audio_probe_boxes);

void Console::commands::debug_steam_audio_dump_scene(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	auto *sndSys = c_engine->GetSoundSystem();
	if(sndSys == nullptr)
	{
		Con::cwar<<"WARNING: Unable to dump steam audio scene: No sound system found!"<<Con::endl;
		return;
	}
	auto *iplScene = sndSys->GetSteamAudioScene();
	if(iplScene == nullptr)
	{
		Con::cwar<<"WARNING: Unable to dump steam audio scene: Steam audio has not been initialized!"<<Con::endl;
		return;
	}
	std::string mapName = "unknown";
	auto *client = c_engine->GetClientState();
	auto *game = (client != nullptr) ? static_cast<CGame*>(client->GetGameState()) : nullptr;
	if(game != nullptr)
		mapName = game->GetMapName();

	std::string path = "debug/maps/";
	FileManager::CreatePath(path.c_str());
	path += mapName +"_steam_audio_scene.obj";
	auto r = iplScene->DumpScene(path);
	if(r == false)
	{
		Con::cwar<<"WARNING: Unable to dump steam audio scene: Has steam audio scene been finalized?"<<Con::endl;
		return;
	}
	Con::cout<<"Steam audio scene successfully dumped as '"<<path<<"'!"<<Con::endl;
}

#else
void Console::commands::debug_steam_audio_dump_scene(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	Con::cwar<<"WARNING: Steam audio is disabled! Scene cannot be dumped."<<Con::endl;
}
#endif
