// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"



export module pragma.server.scripting.lua.classes.recipient_filter;

import pragma.server.entities.components;
export import luabind;

export {;
	namespace Lua {
		namespace RecipientFilter {
			DLLSERVER luabind::tableT<pragma::SPlayerComponent> GetRecipients(lua_State *l, pragma::networking::TargetRecipientFilter &rp);
			DLLSERVER void AddRecipient(lua_State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl);
			DLLSERVER void RemoveRecipient(lua_State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl);
			DLLSERVER bool HasRecipient(lua_State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl);
		};
	};
};
