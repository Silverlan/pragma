// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :scripting.lua.libraries.print;

bool Lua::lua_value_to_string(lua::State *L, int arg, int *r, std::string *val)
{
	if(IsUserData(L, arg) && !util::is_valid(L, luabind::object {luabind::from_stack(L, arg)})) {
		*val = "NULL";
		return true;
	}
	PushValue(L, arg);
	arg = GetStackTop(L);
	GetGlobal(L, "tostring");
	const char *s;
	size_t l;
	PushValue(L, -1);  /* function to be called */
	PushValue(L, arg); /* value to print */
	Call(L, 1, 1);
	s = lua::to_string(L, -1, &l); /* get result */
	if(s == nullptr) {
		*r = lua::error(L, "'tostring' must return a string to 'print'");
		return false;
	}
	*val = std::string(s, l);
	Pop(L, 1); /* pop result */
	Pop(L, 1); /* pop function */
	Pop(L, 1); /* pop value */
	*r = 0;
	return true;
}

int Lua::console::print(lua::State *L)
{
	/*int argc = Lua::GetStackTop(l);
	for(int i=1;i<=argc;i++)
	{
		if(i > 1) Con::COUT<<"\t";
		const char *arg = lua_gettype(l,i);
		Con::COUT<<arg;
	}
	Con::COUT<<Con::endl;
	return 0;*/
	int n = GetStackTop(L); /* number of arguments */
	int i;
	for(i = 1; i <= n; i++) {
		auto status = -1;
		std::string val;
		if(lua_value_to_string(L, i, &status, &val) == false)
			return status;
		if(i > 1)
			Con::COUT << "\t";
		Con::COUT << val;
	}
	Con::COUT << Con::endl;
	return 0;
}

// TODO: Prevent infinite loops (e.g. printing _G)
int Lua::console::print_table(lua::State *l, std::string tab, int idx)
{
	CheckTable(l, idx);
	PushNil(l);
	while(GetNextPair(l, idx) != 0) {
		if(IsTable(l, -1)) {
			auto status = -1;
			std::string key;
			if(lua_value_to_string(l, -2, &status, &key) == false)
				return status;
			Con::COUT << tab << key << ":" << Con::endl;

			std::string tabSub = tab;
			tabSub += "\t";
			print_table(l, tabSub, GetStackTop(l));
		}
		else {
			auto status = -1;
			std::string key;
			if(lua_value_to_string(l, -2, &status, &key) == false)
				return status;

			status = -1;
			std::string val;
			if(lua_value_to_string(l, -1, &status, &val) == false)
				return status;
			Con::COUT << tab << key << " = " << val << Con::endl;
		}
		Pop(l, 1);
	}
	return 0;
}

int Lua::console::print_table(lua::State *l) { return print_table(l, ""); }

int Lua::console::msg(lua::State *l, int st)
{
	int argc = GetStackTop(l);
	if(argc > 0) {
		auto *state = pragma::Engine::Get()->GetNetworkState(l);
		if(state == nullptr)
			pragma::console::set_console_color(pragma::console::ConsoleColorFlags::White | pragma::console::ConsoleColorFlags::Intensity);
		else if(state->IsServer())
			pragma::console::set_console_color(pragma::console::ConsoleColorFlags::Cyan | pragma::console::ConsoleColorFlags::Intensity);
		else
			pragma::console::set_console_color(pragma::console::ConsoleColorFlags::Magenta | pragma::console::ConsoleColorFlags::Intensity);
	}
	for(int i = st; i <= argc; i++) {
		auto status = -1;
		std::string val;
		if(lua_value_to_string(l, i, &status, &val) == false)
			return status;
		Con::COUT << val;
	}
	return 0;
}

int Lua::debug::print(lua::State *l)
{
	auto flags = pragma::console::ConsoleColorFlags::None;
	if(pragma::Engine::Get()->GetNetworkState(l)->IsClient())
		flags |= pragma::console::ConsoleColorFlags::BackgroundMagenta;
	else
		flags |= pragma::console::ConsoleColorFlags::BackgroundCyan;
	pragma::console::set_console_color(flags | pragma::console::ConsoleColorFlags::BackgroundIntensity | pragma::console::ConsoleColorFlags::Black);
	int n = GetStackTop(l); /* number of arguments */
	int i;
	for(i = 1; i <= n; i++) {
		auto status = -1;
		std::string val;
		if(lua_value_to_string(l, i, &status, &val) == false)
			return status;
		if(i > 1)
			Con::COUT << "\t";
		Con::COUT << val;
	}
	Con::COUT << Con::endl;
	beep(l);
	return 0;
}

int Lua::console::msg(lua::State *l) { return msg(l, 1); }

int Lua::console::msgn(lua::State *l)
{
	msg(l);
	Con::COUT << Con::endl;
	return 0;
}

int Lua::console::msgc(lua::State *l)
{
	if(Lua::IsType<::Color>(l, 1)) {
		auto &col = Lua::Check<::Color>(l, 1);

		auto argc = GetStackTop(l);
		std::stringstream ss {};
		for(int i = 2; i <= argc; i++) {
			auto status = -1;
			std::string val;
			if(lua_value_to_string(l, i, &status, &val) == false)
				return status;
			ss << val;
		}

		Con::print(ss.str(), col);
		return 0;
	}
	int flags = Lua::CheckInt<int>(l, 1);
	Con::attr(flags);
	msg(l, 2);
	Con::COUT << Con::endl;
	return 0;
}

int Lua::console::msgw(lua::State *l)
{
	int argc = GetStackTop(l);
	if(argc == 0)
		return 0;
	Con::CWAR << "";
	for(int i = 1; i <= argc; i++) {
		auto status = -1;
		std::string val;
		if(lua_value_to_string(l, i, &status, &val) == false)
			return status;
		Con::CWAR << val;
	}
	Con::CWAR << Con::endl;
	return 0;
}

int Lua::console::msge(lua::State *l)
{
	int argc = GetStackTop(l);
	if(argc == 0)
		return 0;
	Con::CERR << "";
	for(int i = 1; i <= argc; i++) {
		auto status = -1;
		std::string val;
		if(lua_value_to_string(l, i, &status, &val) == false)
			return status;
		Con::CERR << val;
	}
	Con::CERR << Con::endl;
	return 0;
}

static int log(lua::State *l, spdlog::level::level_enum lv)
{
	int n = Lua::GetStackTop(l); /* number of arguments */
	int i = 1;
	std::stringstream ss;
	if(Lua::IsSet(l, 1) && Lua::IsTable(l, 1)) {
		++i;

		for(luabind::iterator i {luabind::object {luabind::from_stack(l, 1)}}, e; i != e; ++i)
			ss << luabind::object_cast<std::string>(*i);
		switch(lv) {
		case spdlog::level::level_enum::warn:
			ss << Con::COLOR_WARNING;
			break;
		case spdlog::level::level_enum::err:
			ss << Con::COLOR_ERROR;
			break;
		case spdlog::level::level_enum::critical:
			ss << Con::COLOR_CRITICAL;
			break;
		}
	}
	auto istart = i;
	for(; i <= n; i++) {
		auto status = -1;
		std::string val;
		if(Lua::lua_value_to_string(l, i, &status, &val) == false)
			return status;
		if(i > istart)
			ss << "\t";
		ss << val;
	}
	switch(lv) {
	case spdlog::level::level_enum::warn:
	case spdlog::level::level_enum::err:
	case spdlog::level::level_enum::critical:
		ss << Con::COLOR_RESET;
		break;
	}
	spdlog::log(lv, ss.str());
	return 0;
}

int Lua::log::info(lua::State *l) { return ::log(l, spdlog::level::info); }
int Lua::log::warn(lua::State *l) { return ::log(l, spdlog::level::warn); }
int Lua::log::error(lua::State *l) { return ::log(l, spdlog::level::err); }
int Lua::log::critical(lua::State *l) { return ::log(l, spdlog::level::critical); }
int Lua::log::debug(lua::State *l) { return ::log(l, spdlog::level::debug); }
int Lua::log::register_logger(lua::State *l)
{
	std::string name = CheckString(l, 1);
	auto &logger = pragma::register_logger(name);
	Lua::Push<spdlog::logger *>(l, &logger);
	return 1;
}
int Lua::log::color(lua::State *l)
{
	auto level = static_cast<pragma::util::LogSeverity>(CheckInt(l, 1));
	std::string c {};
	switch(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(level))) {
	case spdlog::level::level_enum::warn:
		c = Con::COLOR_WARNING;
		break;
	case spdlog::level::level_enum::err:
		c = Con::COLOR_ERROR;
		break;
	case spdlog::level::level_enum::critical:
		c = Con::COLOR_CRITICAL;
		break;
	}
	PushString(l, c);
	return 1;
}
