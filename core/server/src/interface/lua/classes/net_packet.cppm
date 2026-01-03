// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.classes.net_packet;

export import pragma.shared;
export import pragma.util;

export namespace Lua {
	namespace NetPacket {
		namespace Server {
			DLLSERVER void register_class(luabind::class_<::NetPacket> &classDef);
			DLLSERVER void WriteALSound(lua::State *l, ::NetPacket &packet, std::shared_ptr<pragma::audio::ALSound> snd);
			DLLSERVER void WriteUniqueEntity(lua::State *l, ::NetPacket &packet, pragma::ecs::BaseEntity *hEnt);
			DLLSERVER void WriteUniqueEntity(lua::State *l, ::NetPacket &packet);
		};
	};
};
