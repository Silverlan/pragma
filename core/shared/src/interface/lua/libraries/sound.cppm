// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>

// SPDX-License-Identifier: MIT



module;



#include "pragma/networkdefinitions.h"
#include "pragma/lua/core.hpp"
#include <string>




export module pragma.shared:scripting.lua.libraries.sound;



export import :audio.sound;



export namespace Lua {

	namespace sound {

		struct DLLNETWORK SoundPlayInfo {

			float gain = 1.f;

			float pitch = 1.f;

			std::optional<Vector3> origin {};

			pragma::audio::ALCreateFlags flags = pragma::audio::ALCreateFlags::None;

		};



		DLLNETWORK void register_enums(lua_State *l);

		DLLNETWORK void register_library(luabind::module_ &soundMod);



		DLLNETWORK std::shared_ptr<::ALSound> create(lua_State *l, const std::string &snd, pragma::audio::ALSoundType type, pragma::audio::ALCreateFlags flags);

		DLLNETWORK std::shared_ptr<::ALSound> create(lua_State *l, const std::string &snd, pragma::audio::ALSoundType type);

		DLLNETWORK std::shared_ptr<::ALSound> play(lua_State *l, const std::string &sndName, pragma::audio::ALSoundType type, const SoundPlayInfo &playInfo);

		DLLNETWORK std::shared_ptr<::ALSound> play(lua_State *l, const std::string &sndName, pragma::audio::ALSoundType type);

		DLLNETWORK bool is_music_playing(lua_State *l);

		DLLNETWORK float get_duration(lua_State *l, const std::string &snd);

		DLLNETWORK std::vector<std::shared_ptr<::ALSound>> get_all(lua_State *l);

		DLLNETWORK std::vector<std::shared_ptr<::ALSound>> find_by_type(lua_State *l, pragma::audio::ALSoundType type, bool bExactMatch);

		DLLNETWORK std::vector<std::shared_ptr<::ALSound>> find_by_type(lua_State *l, pragma::audio::ALSoundType type);

		DLLNETWORK bool precache(lua_State *l, const std::string &snd, ALChannel mode);

		DLLNETWORK bool precache(lua_State *l, const std::string &snd);

		DLLNETWORK void stop_all(lua_State *l);

		DLLNETWORK void load_scripts(lua_State *l, const std::string &file);

		DLLNETWORK luabind::object read_wav_phonemes(lua_State *l, const std::string &fileName);

	};

};

