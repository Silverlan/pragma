// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:scripting.lua.classes.faction;

import :ai;
export import pragma.lua;

export namespace Lua {
	namespace Faction {
		DLLSERVER void register_class(lua::State *l, luabind::module_ &mod);
		DLLSERVER void SetDisposition(lua::State *l, ::Faction &faction, ::Faction &factionTgt, uint32_t disposition, bool revert, int32_t priority);
		DLLSERVER void SetDisposition(lua::State *l, ::Faction &faction, ::Faction &factionTgt, uint32_t disposition, bool revert);
		DLLSERVER void SetDisposition(lua::State *l, ::Faction &faction, ::Faction &factionTgt, uint32_t disposition);
		DLLSERVER void SetEnemyFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert, int32_t priority);
		DLLSERVER void SetEnemyFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert);
		DLLSERVER void SetEnemyFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt);
		DLLSERVER void SetAlliedFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert, int32_t priority);
		DLLSERVER void SetAlliedFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert);
		DLLSERVER void SetAlliedFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt);
		DLLSERVER void SetNeutralFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert, int32_t priority);
		DLLSERVER void SetNeutralFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert);
		DLLSERVER void SetNeutralFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt);
		DLLSERVER void SetFearsomeFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert, int32_t priority);
		DLLSERVER void SetFearsomeFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert);
		DLLSERVER void SetFearsomeFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt);
		DLLSERVER void GetDisposition(lua::State *l, ::Faction &faction, ::Faction &factionTgt);
		DLLSERVER void GetDisposition(lua::State *l, ::Faction &faction, const std::string &className);
		DLLSERVER void GetDisposition(lua::State *l, ::Faction &faction, pragma::ecs::BaseEntity &ent);
	};
};
