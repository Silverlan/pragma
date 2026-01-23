// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.libraries.sound;

import :server_state;

std::shared_ptr<pragma::audio::ALSound> Lua::sound::Server::create(lua::State *l, const std::string &snd, pragma::audio::ALSoundType type, pragma::audio::ALCreateFlags flags)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto pAl = state->CreateSound(snd, type, flags);
	if(pAl == nullptr)
		return nullptr;
	pAl->SetType(type);
	return pAl;
}
