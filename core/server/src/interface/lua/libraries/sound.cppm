// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.libraries.sound;

export import pragma.shared;

export namespace Lua {
	namespace sound {
		namespace Server {
			std::shared_ptr<pragma::audio::ALSound> create(lua::State *l, const std::string &snd, pragma::audio::ALSoundType type, pragma::audio::ALCreateFlags flags);
		};
	};
};
