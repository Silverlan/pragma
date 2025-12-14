// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.classes.convar;

pragma::console::ConVar *Lua::console::CreateConVar(lua::State *l, const std::string &cmd, ::udm::Type type, udm_type def, pragma::console::ConVarFlags flags, const std::string &help)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	if(state == nullptr)
		return 0;
	auto cvar = ::udm::visit(type, [&def, flags, &help](auto tag) {
		using T = typename decltype(tag)::type;
		constexpr auto type = ::udm::type_to_enum<T>();
		if constexpr(type == ::udm::Type::Element || ::udm::is_array_type(type))
			return std::shared_ptr<pragma::console::ConVar> {nullptr};
		else {
			auto v = luabind::object_cast<T>(def);
			return pragma::console::ConVar::Create<T>(v, flags, help);
		}
	});
	if(!cvar)
		return 0;
	return state->RegisterConVar(cmd, cvar);
}

void Lua::console::CreateConCommand(lua::State *l, const std::string &name, const func<void, pragma::BasePlayerComponent, float, variadic<std::string>> &function, pragma::console::ConVarFlags flags, const std::string &help)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	state->CreateConCommand(name, function, flags, help);
}
void Lua::console::CreateConCommand(lua::State *l, const std::string &name, const func<void, pragma::BasePlayerComponent, float, variadic<std::string>> &function, pragma::console::ConVarFlags flags)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	state->CreateConCommand(name, function, flags);
}
void Lua::console::CreateConCommand(lua::State *l, const std::string &name, const func<void, pragma::BasePlayerComponent, float, variadic<std::string>> &function, const std::string &help)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	state->CreateConCommand(name, function, pragma::console::ConVarFlags::None, help);
}

pragma::console::ConVar *Lua::console::GetConVar(lua::State *l, const std::string &name)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	if(state == nullptr)
		return nullptr;
	auto *cv = state->GetConVar(name);
	if(cv == nullptr)
		return nullptr;
	if(cv->GetType() != pragma::console::ConType::Var)
		return nullptr;
	return static_cast<pragma::console::ConVar *>(cv);
}

static pragma::console::ConVar *get_con_var(lua::State *l, const std::string &conVar)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	if(state == nullptr)
		return nullptr;
	auto *cv = state->GetConVar(conVar);
	if(cv == nullptr || cv->GetType() != pragma::console::ConType::Var)
		return nullptr;
	return static_cast<pragma::console::ConVar *>(cv);
}

int32_t Lua::console::GetConVarInt(lua::State *l, const std::string &conVar)
{
	auto *cv = get_con_var(l, conVar);
	return cv ? cv->GetInt() : 0;
}

float Lua::console::GetConVarFloat(lua::State *l, const std::string &conVar)
{
	auto *cv = get_con_var(l, conVar);
	return cv ? cv->GetFloat() : 0.f;
}

std::string Lua::console::GetConVarString(lua::State *l, const std::string &conVar)
{
	auto *cv = get_con_var(l, conVar);
	return cv ? cv->GetString() : "";
}

bool Lua::console::GetConVarBool(lua::State *l, const std::string &conVar)
{
	auto *cv = get_con_var(l, conVar);
	return cv ? cv->GetBool() : false;
}

pragma::console::ConVarFlags Lua::console::GetConVarFlags(lua::State *l, const std::string &conVar)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	if(state == nullptr)
		return pragma::console::ConVarFlags::None;
	auto *cv = state->GetConVar(conVar);
	if(cv == nullptr)
		return pragma::console::ConVarFlags::None;
	return cv->GetFlags();
}

int Lua::console::Run(lua::State *l)
{
	int argc = GetStackTop(l);
	std::string cmd = CheckString(l, 1);
	int i = 2;
	while(argc >= i) {
		std::string arg = CheckString(l, i);
		cmd += " \"" + arg + "\"";
		i++;
	}
	//Engine::Get()->ConsoleInput(cmd.c_str());
	pragma::Engine::Get()->ProcessConsoleInput(cmd);
	return 0;
}

int Lua::console::AddChangeCallback(lua::State *l)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	auto *game = state->GetGameState();
	if(game == nullptr)
		return 0;
	auto cvar = CheckString(l, 1);
	CheckFunction(l, 2);
	auto fc = luabind::object(luabind::from_stack(l, 2));
	auto cb = game->AddConVarCallback(cvar, fc);
	if(!cb.IsValid())
		return 0;
	Push(l, cb);
	return 1;
}

void Lua::console::register_override(lua::State *l, const std::string &src, const std::string &dst)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	state->SetConsoleCommandOverride(src, dst);
}
void Lua::console::clear_override(lua::State *l, const std::string &src)
{
	auto *state = pragma::Engine::Get()->GetNetworkState(l);
	state->ClearConsoleCommandOverride(src);
}
int Lua::console::parse_command_arguments(lua::State *l)
{
	int32_t tArgs = 1;
	CheckTable(l, tArgs);
	std::vector<std::string> args {};
	auto numArgs = GetObjectLength(l, tArgs);
	args.reserve(numArgs);
	for(auto i = decltype(numArgs) {0u}; i < numArgs; ++i) {
		PushInt(l, i + 1);       /* 1 */
		GetTableValue(l, tArgs); /* 1 */
		args.push_back(CheckString(l, -1));

		Pop(l, 1); /* 0 */
	}

	std::unordered_map<std::string, pragma::console::CommandOption> commandOptions {};
	pragma::console::parse_command_options(args, commandOptions);

	auto t = CreateTable(l);
	for(auto &pair : commandOptions) {
		PushString(l, pair.first);  /* 1 */
		auto tCmd = CreateTable(l); /* 2 */
		auto &cmd = pair.second;
		int32_t argIdx = 1;
		for(auto &str : cmd.parameters) {
			PushInt(l, argIdx++);   /* 3 */
			PushString(l, str);     /* 4 */
			SetTableValue(l, tCmd); /* 2 */
		}
		SetTableValue(l, t); /* 0 */
	}
	return 1;
}
