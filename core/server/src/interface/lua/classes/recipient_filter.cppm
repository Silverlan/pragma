// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.classes.recipient_filter;

import :entities.components;
export import pragma.lua;

export {
	;
	namespace Lua {
		namespace RecipientFilter {
			DLLSERVER luabind::tableT<pragma::SPlayerComponent> GetRecipients(lua::State *l, pragma::networking::TargetRecipientFilter &rp);
			DLLSERVER void AddRecipient(lua::State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl);
			DLLSERVER void RemoveRecipient(lua::State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl);
			DLLSERVER bool HasRecipient(lua::State *l, pragma::networking::TargetRecipientFilter &rp, pragma::SPlayerComponent &hPl);
		};
	};
};
