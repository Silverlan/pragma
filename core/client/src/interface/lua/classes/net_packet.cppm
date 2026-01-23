// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:scripting.lua.classes.net_packet;

export import pragma.lua;
export import pragma.util;

export namespace Lua {
	namespace NetPacket {
		namespace Client {
			DLLCLIENT void register_class(luabind::class_<::NetPacket> &classDef);
			DLLCLIENT void ReadUniqueEntity(lua::State *l, ::NetPacket &packet, luabind::object o);
		};
	};
};
