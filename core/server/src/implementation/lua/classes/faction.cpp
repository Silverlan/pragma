// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :scripting.lua.classes.faction;

void Lua::Faction::register_class(lua::State *l, luabind::module_ &mod)
{
	auto classDef = luabind::class_<::Faction>("Faction");
	classDef.def("AddClass", &::Faction::AddClass);
	classDef.def("GetClasses", &::Faction::GetClasses);
	classDef.def("SetDisposition", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, uint32_t, bool, int32_t)>(&SetDisposition));
	classDef.def("SetDisposition", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, uint32_t, bool)>(&SetDisposition));
	classDef.def("SetDisposition", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, uint32_t)>(&SetDisposition));
	classDef.def("SetEnemyFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, bool, int32_t)>(&SetEnemyFaction));
	classDef.def("SetEnemyFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, bool)>(&SetEnemyFaction));
	classDef.def("SetEnemyFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &)>(&SetEnemyFaction));
	classDef.def("SetAlliedFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, bool, int32_t)>(&SetAlliedFaction));
	classDef.def("SetAlliedFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, bool)>(&SetAlliedFaction));
	classDef.def("SetAlliedFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &)>(&SetAlliedFaction));
	classDef.def("SetNeutralFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, bool, int32_t)>(&SetNeutralFaction));
	classDef.def("SetNeutralFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, bool)>(&SetNeutralFaction));
	classDef.def("SetNeutralFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &)>(&SetNeutralFaction));
	classDef.def("SetFearsomeFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, bool, int32_t)>(&SetFearsomeFaction));
	classDef.def("SetFearsomeFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &, bool)>(&SetFearsomeFaction));
	classDef.def("SetFearsomeFaction", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &)>(&SetFearsomeFaction));
	classDef.def("GetDisposition", static_cast<void (*)(lua::State *, ::Faction &, ::Faction &)>(&GetDisposition));
	classDef.def("GetDisposition", static_cast<void (*)(lua::State *, ::Faction &, const std::string &)>(&GetDisposition));
	classDef.def("GetDisposition", static_cast<void (*)(lua::State *, ::Faction &, pragma::ecs::BaseEntity &)>(&GetDisposition));
	classDef.def("HasClass", &::Faction::HasClass);
	classDef.def("SetDefaultDisposition", &::Faction::SetDefaultDisposition);
	classDef.def("GetDefaultDisposition", &::Faction::GetDefaultDisposition);
	classDef.def("GetName", &::Faction::GetName);
	mod[classDef];
}

void Lua::Faction::SetDisposition(lua::State *, ::Faction &faction, ::Faction &factionTgt, uint32_t disposition, bool revert, int32_t priority) { faction.SetDisposition(factionTgt, static_cast<DISPOSITION>(disposition), revert, priority); }
void Lua::Faction::SetDisposition(lua::State *l, ::Faction &faction, ::Faction &factionTgt, uint32_t disposition, bool revert) { SetDisposition(l, faction, factionTgt, disposition, revert, 0); }
void Lua::Faction::SetDisposition(lua::State *l, ::Faction &faction, ::Faction &factionTgt, uint32_t disposition) { SetDisposition(l, faction, factionTgt, disposition, false, 0); }
void Lua::Faction::SetEnemyFaction(lua::State *, ::Faction &faction, ::Faction &factionTgt, bool revert, int32_t priority) { faction.SetEnemyFaction(factionTgt, revert, priority); }
void Lua::Faction::SetEnemyFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert) { SetEnemyFaction(l, faction, factionTgt, revert, 0); }
void Lua::Faction::SetEnemyFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt) { SetEnemyFaction(l, faction, factionTgt, false, 0); }
void Lua::Faction::SetAlliedFaction(lua::State *, ::Faction &faction, ::Faction &factionTgt, bool revert, int32_t priority) { faction.SetAlliedFaction(factionTgt, revert, priority); }
void Lua::Faction::SetAlliedFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert) { SetAlliedFaction(l, faction, factionTgt, revert, 0); }
void Lua::Faction::SetAlliedFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt) { SetAlliedFaction(l, faction, factionTgt, false, 0); }
void Lua::Faction::SetNeutralFaction(lua::State *, ::Faction &faction, ::Faction &factionTgt, bool revert, int32_t priority) { faction.SetNeutralFaction(factionTgt, revert, priority); }
void Lua::Faction::SetNeutralFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert) { SetNeutralFaction(l, faction, factionTgt, revert, 0); }
void Lua::Faction::SetNeutralFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt) { SetNeutralFaction(l, faction, factionTgt, false, 0); }
void Lua::Faction::SetFearsomeFaction(lua::State *, ::Faction &faction, ::Faction &factionTgt, bool revert, int32_t priority) { faction.SetFearsomeFaction(factionTgt, revert, priority); }
void Lua::Faction::SetFearsomeFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt, bool revert) { SetFearsomeFaction(l, faction, factionTgt, revert, 0); }
void Lua::Faction::SetFearsomeFaction(lua::State *l, ::Faction &faction, ::Faction &factionTgt) { SetFearsomeFaction(l, faction, factionTgt, false, 0); }
void Lua::Faction::GetDisposition(lua::State *l, ::Faction &faction, ::Faction &factionTgt)
{
	int32_t priority = 0;
	auto disp = faction.GetDisposition(factionTgt, &priority);
	PushInt(l, pragma::math::to_integral(disp));
	PushInt(l, priority);
}
void Lua::Faction::GetDisposition(lua::State *l, ::Faction &faction, const std::string &className)
{
	int32_t priority = 0;
	auto disp = faction.GetDisposition(className, &priority);
	PushInt(l, pragma::math::to_integral(disp));
	PushInt(l, priority);
}
void Lua::Faction::GetDisposition(lua::State *l, ::Faction &faction, pragma::ecs::BaseEntity &ent)
{
	int32_t priority = 0;
	auto disp = faction.GetDisposition(&ent, &priority);
	PushInt(l, pragma::math::to_integral(disp));
	PushInt(l, priority);
}
