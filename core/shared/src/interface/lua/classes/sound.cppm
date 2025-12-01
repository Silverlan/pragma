// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.classes.sound;

export import :audio.sound;

export {
	namespace Lua {
		namespace ALSound {
			DLLNETWORK void register_class(luabind::class_<::ALSound> &classDef);
		};
	};
	std::ostream &operator<<(std::ostream &out, const ::ALSound &snd);
};
