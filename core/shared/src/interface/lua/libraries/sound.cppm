// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"
#include "pragma/lua/luaapi.h"

export module pragma.shared:scripting.lua.libraries.sound;

export import :audio.sound;

#define LUA_LIB_SOUND_SHARED                                                                                                                                                                                                                                                                     \
	{"play", Lua::sound::play}, {"get_duration", Lua::sound::get_duration}, {"get_all", Lua::sound::get_all}, {"is_music_playing", Lua::sound::is_music_playing}, {"find_by_type", Lua::sound::find_by_type}, {"precache", Lua::sound::precache}, {"stop_all", Lua::sound::stop_all},            \
	  {"load_scripts", Lua::sound::load_scripts},                                                                                                                                                                                                                                                \
	{                                                                                                                                                                                                                                                                                            \
		"read_wav_phonemes", Lua::sound::read_wav_phonemes                                                                                                                                                                                                                                       \
	}

export namespace Lua {
	namespace sound {
		DLLNETWORK void register_enums(lua_State *l);

		DLLNETWORK int create(lua_State *l, const std::function<std::shared_ptr<ALSound>(NetworkState *, const std::string &, ALSoundType, ALCreateFlags)> &f);
		DLLNETWORK int create(lua_State *l);
		DLLNETWORK int play(lua_State *l);
		DLLNETWORK int is_music_playing(lua_State *l);
		DLLNETWORK int get_duration(lua_State *l);
		DLLNETWORK int get_all(lua_State *l);
		DLLNETWORK int find_by_type(lua_State *l);
		DLLNETWORK int precache(lua_State *l);
		DLLNETWORK int stop_all(lua_State *l);
		DLLNETWORK int load_scripts(lua_State *l);
		DLLNETWORK int read_wav_phonemes(lua_State *l);
	};
};
