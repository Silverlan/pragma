// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.client:scripting.lua.bindings.audio;
export import :audio;
export import se_script;

export namespace pragma::scripting::lua_core::bindings {
	void register_audio(lua::State *l);
};

namespace Lua {
	namespace ALSound {
		namespace Client {
			DLLCLIENT void register_class(luabind::class_<pragma::audio::ALSound> &classDef);
			DLLCLIENT void register_buffer(luabind::class_<pragma::audio::ISoundBuffer> &classDef);
		};
	};
};

namespace Lua {
	namespace ALSound {
		namespace Client {
			static void GetWorldPosition(lua::State *l, pragma::audio::ALSound &snd);

			//static void GetChannelConfigName(lua::State *l,::ALSound &snd);
			//static void GetSampleTypeName(lua::State *l,::ALSound &snd);
			static void IsMono(lua::State *l, pragma::audio::ALSound &snd);
			static void IsStereo(lua::State *l, pragma::audio::ALSound &snd);
		};
	};
	namespace ALBuffer {
		static opt<luabind::tableT<void>> GetPhonemeData(lua::State *l, pragma::audio::ISoundBuffer &);
	};
};
