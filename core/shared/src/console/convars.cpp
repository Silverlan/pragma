/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/networkstate/networkstate.h"
#include <pragma/console/convars.h>
#include <pragma/console/conout.h>

ConConf::ConConf(ConVarFlags flags) : m_help(""), m_ID(0), m_type(ConType::Var), m_flags(flags) {}
const std::string &ConConf::GetHelpText() const { return m_help; }
const std::string &ConConf::GetUsageHelp() const { return m_usageHelp; }
ConType ConConf::GetType() const { return m_type; }
ConConf *ConConf::Copy() { return nullptr; }
uint32_t ConConf::GetID() const { return m_ID; }
ConVarFlags ConConf::GetFlags() const { return m_flags; }

void ConConf::Print(const std::string &name)
{
	auto type = GetType();
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	if(type == ConType::Var) {
		ConVar *cvar = static_cast<ConVar *>(this);
		auto flags = cvar->GetFlags();
		if(umath::is_flag_set(flags, ConVarFlags::Hidden) || umath::is_flag_set(flags, ConVarFlags::Password))
			return;
		Con::cout << "\"" << name << "\" = \"" << cvar->GetString() << "\" (Default: " << cvar->GetDefault() << ")" << Con::endl;
		if(flags > ConVarFlags::None) {
			util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
			if((flags & ConVarFlags::Cheat) == ConVarFlags::Cheat)
				Con::cout << " cheat";
			if((flags & ConVarFlags::Singleplayer) == ConVarFlags::Singleplayer)
				Con::cout << " singleplayer";
			if((flags & ConVarFlags::Userinfo) == ConVarFlags::Userinfo)
				Con::cout << " userinfo";
			if((flags & ConVarFlags::Replicated) == ConVarFlags::Replicated)
				Con::cout << " replicated";
			if((flags & ConVarFlags::Archive) == ConVarFlags::Archive)
				Con::cout << " archive";
			if((flags & ConVarFlags::Notify) == ConVarFlags::Notify)
				Con::cout << " notify";
			Con::cout << Con::endl;
			static_assert(umath::to_integral(ConVarFlags::Last) == 256);
		}
	}
	else
		Con::cout << "\"" << name << "\"" << Con::endl;
	util::set_console_color(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity);
	Con::cout << GetHelpText() << Con::endl;
}

//////////////////////////////////

ConVarValue create_convar_value(udm::Type type, const void *value)
{
	if(!value)
		return ConVarValue {nullptr, [](void *) {}};
	return console::visit<false>(type, [type, value](auto tag) -> ConVarValue {
		using T = typename decltype(tag)::type;
		if constexpr(console::is_valid_convar_type_v<T>) {
			return ConVarValue {new T {*static_cast<const T *>(value)}, [](void *val) { delete static_cast<T *>(val); }};
		}
		else
			return create_convar_value();
	});
}
ConVarValue create_convar_value() { return create_convar_value(udm::Type::Invalid, nullptr); }
ConVar::ConVar(udm::Type type, const void *value, ConVarFlags flags, const std::string &help, const std::string &usageHelp) : ConConf {}, m_varType {type}
{
	assert(value != nullptr);
	m_type = ConType::Var;
	m_help = help;
	m_usageHelp = usageHelp;
	m_value = create_convar_value(type, value);
	m_default = create_convar_value(type, value);
	m_flags = flags;
}
void ConVar::SetValue(const std::string &val)
{
	console::visit(m_varType, [this, &val](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<std::string, T>())
			*static_cast<T *>(m_value.get()) = udm::convert<std::string, T>(val);
	});
}
std::string ConVar::GetString() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, std::string>())
			return udm::convert<T, std::string>(*static_cast<T *>(m_value.get()));
		return std::string {};
	});
}
std::string ConVar::GetDefault() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, std::string>())
			return udm::convert<T, std::string>(*static_cast<T *>(m_default.get()));
		return std::string {};
	});
}
int32_t ConVar::GetInt() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, int32_t>())
			return udm::convert<T, int32_t>(*static_cast<T *>(m_value.get()));
		return 0;
	});
}
float ConVar::GetFloat() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, float>())
			return udm::convert<T, float>(*static_cast<T *>(m_value.get()));
		return 0.f;
	});
}
bool ConVar::GetBool() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, bool>())
			return udm::convert<T, bool>(*static_cast<T *>(m_value.get()));
		return false;
	});
}
void ConVar::AddCallback(int function) { m_callbacks.push_back(function); }
ConConf *ConVar::Copy()
{
	auto *cvar = new ConVar {m_varType, m_value.get(), m_flags, m_help, m_usageHelp};
	cvar->m_ID = m_ID;
	return static_cast<ConConf *>(cvar);
}

//////////////////////////////////

ConCommand::ConCommand(const std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &function, ConVarFlags flags, const std::string &help, const std::function<void(const std::string &, std::vector<std::string> &)> &autoCompleteCallback)
    : ConConf(flags), m_function(function), m_functionLua(nullptr), m_autoCompleteCallback {autoCompleteCallback}
{
	m_help = help;
	m_type = ConType::Cmd;
}
ConCommand::ConCommand(const LuaFunction &function, ConVarFlags flags, const std::string &help, const std::function<void(const std::string &, std::vector<std::string> &)> &autoCompleteCallback)
    : ConConf(flags), m_function(nullptr), m_functionLua(function), m_autoCompleteCallback {autoCompleteCallback}
{
	m_help = help;
	m_type = ConType::LuaCmd;
}
const std::function<void(const std::string &, std::vector<std::string> &)> &ConCommand::GetAutoCompleteCallback() const { return m_autoCompleteCallback; }
void ConCommand::SetAutoCompleteCallback(const std::function<void(const std::string &, std::vector<std::string> &)> &callback) { m_autoCompleteCallback = callback; }
void ConCommand::GetFunction(LuaFunction &function) const { function = m_functionLua; }
void ConCommand::GetFunction(std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &function) const { function = m_function; }
void ConCommand::SetFunction(const std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &function)
{
	m_type = ConType::Cmd;
	m_function = function;
}
ConConf *ConCommand::Copy()
{
	ConCommand *cmd;
	if(GetType() == ConType::Cmd)
		cmd = new ConCommand(m_function, m_flags, m_help, m_autoCompleteCallback);
	else
		cmd = new ConCommand(m_functionLua, m_flags, m_help, m_autoCompleteCallback);
	cmd->m_ID = m_ID;
	return static_cast<ConConf *>(cmd);
}

//////////////////////////////////

CvarCallback::CvarCallback() : m_bLua(false), m_function(nullptr), m_functionLua(nullptr) {}
CvarCallback::CvarCallback(LuaFunction fc) : CvarCallback()
{
	m_bLua = true;
	m_functionLua = fc;
}
CvarCallback::CvarCallback(const std::shared_ptr<CvarCallbackFunction> &fc) : CvarCallback() { m_function = fc; }
CvarCallback::CvarCallback(const CvarCallback &cv) : CvarCallback()
{
	m_bLua = cv.m_bLua;
	if(m_bLua)
		m_functionLua = cv.m_functionLua;
	else
		m_function = cv.m_function;
}
bool CvarCallback::IsLuaFunction() const { return m_bLua; }
LuaFunction *CvarCallback::GetLuaFunction() { return (m_bLua == true) ? &m_functionLua : nullptr; }
CvarCallbackFunction *CvarCallback::GetFunction() { return m_function.get(); }

//////////////////////////////////

CvarCallbackFunction::CvarCallbackFunction() : m_type(Type::Invalid) {}
CvarCallbackFunction::CvarCallbackFunction(const std::function<void(NetworkState *, ConVar *, int, int)> &fc) : m_type(Type::Int), m_functionInt(fc) {}
CvarCallbackFunction::CvarCallbackFunction(const std::function<void(NetworkState *, ConVar *, std::string, std::string)> &fc) : m_type(Type::String), m_functionString(fc) {}
CvarCallbackFunction::CvarCallbackFunction(const std::function<void(NetworkState *, ConVar *, float, float)> &fc) : m_type(Type::Float), m_functionFloat(fc) {}
CvarCallbackFunction::CvarCallbackFunction(const std::function<void(NetworkState *, ConVar *, bool, bool)> &fc) : m_type(Type::Bool), m_functionBool(fc) {}
CvarCallbackFunction::CvarCallbackFunction(const CvarCallbackFunction &b)
{
	m_type = b.m_type;
	switch(m_type) {
	case Type::Int:
		m_functionInt = b.m_functionInt;
		break;
	case Type::String:
		m_functionString = b.m_functionString;
		break;
	case Type::Float:
		m_functionFloat = b.m_functionFloat;
		break;
	default:
		m_functionBool = b.m_functionBool;
		break;
	}
}
void CvarCallbackFunction::Call(NetworkState *state, ConVar *cvar, const std::string &prev)
{
	switch(m_type) {
	case Type::Int:
		m_functionInt(state, cvar, atoi(prev.c_str()), cvar->GetInt());
		break;
	case Type::String:
		m_functionString(state, cvar, prev, cvar->GetString());
		break;
	case Type::Float:
		m_functionFloat(state, cvar, static_cast<float>(atof(prev.c_str())), cvar->GetFloat());
		break;
	case Type::Bool:
		m_functionBool(state, cvar, prev != "0", cvar->GetBool());
		break;
	}
}
bool CvarCallbackFunction::IsValid() { return (m_type != Type::Invalid) ? true : false; }
void CvarCallbackFunction::SetFunction(const std::function<void(NetworkState *, ConVar *, int, int)> &fc)
{
	m_type = Type::Int;
	m_functionInt = fc;
}
void CvarCallbackFunction::SetFunction(const std::function<void(NetworkState *, ConVar *, std::string, std::string)> &fc)
{
	m_type = Type::String;
	m_functionString = fc;
}
void CvarCallbackFunction::SetFunction(const std::function<void(NetworkState *, ConVar *, float, float)> &fc)
{
	m_type = Type::Float;
	m_functionFloat = fc;
}
void CvarCallbackFunction::SetFunction(const std::function<void(NetworkState *, ConVar *, bool, bool)> &fc)
{
	m_type = Type::Bool;
	m_functionBool = fc;
}

//////////////////////////////////

static void initialize_convar_map(ConVarMap *&r)
{
	if(r != nullptr)
		return;
	// Linux sets the object to NULL when using unique_ptr in some cases,
	// which causes it to be initialized multiple times, so we'll use a regular pointer instead.
	// Ugly, but it works.
	//r = std::unique_ptr<ConVarMap>(new ConVarMap{});
	r = new ConVarMap {};
}

#define cvar_newglobal(suffix, glname)                                                                                                                                                                                                                                                           \
	DLLNETWORK ConVarMap *g_ConVars##suffix = nullptr;                                                                                                                                                                                                                                           \
	bool console_system::glname::register_convar(const std::string &cvar, const std::string &value, ConVarFlags flags, const std::string &help)                                                                                                                                                  \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->RegisterConVar<std::string>(cvar, value, flags, help);                                                                                                                                                                                                                \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool console_system::glname::register_convar_callback(const std::string &scvar, int)                                                                                                                                                                                                         \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->PreRegisterConVarCallback(scvar);                                                                                                                                                                                                                                     \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool console_system::glname::register_convar_callback(const std::string &scvar, void (*function)(NetworkState *, ConVar *, int, int))                                                                                                                                                        \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->RegisterConVarCallback(scvar, std::function<void(NetworkState *, ConVar *, int, int)>(function));                                                                                                                                                                     \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool console_system::glname::register_convar_callback(const std::string &scvar, void (*function)(NetworkState *, ConVar *, std::string, std::string))                                                                                                                                        \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->RegisterConVarCallback(scvar, std::function<void(NetworkState *, ConVar *, std::string, std::string)>(function));                                                                                                                                                     \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool console_system::glname::register_convar_callback(const std::string &scvar, void (*function)(NetworkState *, ConVar *, float, float))                                                                                                                                                    \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->RegisterConVarCallback(scvar, std::function<void(NetworkState *, ConVar *, float, float)>(function));                                                                                                                                                                 \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool console_system::glname::register_convar_callback(const std::string &scvar, void (*function)(NetworkState *, ConVar *, bool, bool))                                                                                                                                                      \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->RegisterConVarCallback(scvar, std::function<void(NetworkState *, ConVar *, bool, bool)>(function));                                                                                                                                                                   \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	static bool register_concommand_##glname(const std::string &cvar, const std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &function, ConVarFlags flags, const std::string &help)                                                         \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		if(function == nullptr)                                                                                                                                                                                                                                                                  \
			g_ConVars##suffix->PreRegisterConCommand(cvar, flags, help);                                                                                                                                                                                                                         \
		else                                                                                                                                                                                                                                                                                     \
			g_ConVars##suffix->RegisterConCommand(cvar, function, flags, help);                                                                                                                                                                                                                  \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool console_system::glname::register_concommand(const std::string &cvar, void (*function)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), ConVarFlags flags, const std::string &help)                                                                    \
	{                                                                                                                                                                                                                                                                                            \
		return ::register_concommand_##glname(cvar, function, flags, help);                                                                                                                                                                                                                      \
	}                                                                                                                                                                                                                                                                                            \
	bool console_system::glname::register_concommand(const std::string &cvar, void (*function)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), ConVarFlags flags, const std::string &help)                                                                           \
	{                                                                                                                                                                                                                                                                                            \
		return ::register_concommand_##glname(cvar, std::bind(function, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), flags, help);                                                                                                                                      \
	}                                                                                                                                                                                                                                                                                            \
	bool console_system::glname::register_concommand(const std::string &cvar, void (*function)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), const std::string &help)                                                                                       \
	{                                                                                                                                                                                                                                                                                            \
		return register_concommand(cvar, function, ConVarFlags::None, help);                                                                                                                                                                                                                     \
	}                                                                                                                                                                                                                                                                                            \
	bool console_system::glname::register_concommand(const std::string &cvar, void (*function)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), const std::string &help)                                                                                              \
	{                                                                                                                                                                                                                                                                                            \
		return register_concommand(cvar, function, ConVarFlags::None, help);                                                                                                                                                                                                                     \
	}                                                                                                                                                                                                                                                                                            \
	ConVarMap *console_system::glname::get_convar_map()                                                                                                                                                                                                                                          \
	{                                                                                                                                                                                                                                                                                            \
		return g_ConVars##suffix;                                                                                                                                                                                                                                                                \
	}

cvar_newglobal(Sv, server);
cvar_newglobal(Cl, client);
cvar_newglobal(En, engine);

ConVarMap::ConVarMap() : m_conVarID(1) {}

std::shared_ptr<ConVar> ConVarMap::RegisterConVar(const std::string &scmd, udm::Type type, const void *value, ConVarFlags flags, const std::string &help, const std::optional<std::string> &optUsageHelp,
  std::function<void(const std::string &, std::vector<std::string> &)> autoCompleteFunction)
{
	auto lscmd = scmd;
	ustring::to_lower(lscmd);
	if(m_conVars.find(lscmd) != m_conVars.end())
		return nullptr;
	std::string usageHelp;
	if(optUsageHelp.has_value())
		usageHelp = *optUsageHelp;
	else if(type == udm::Type::Boolean)
		usageHelp = "1/0";
	auto cv = console::visit(type, [value, flags, &help, &usageHelp](auto tag) {
		using T = typename decltype(tag)::type;
		return ConVar::Create<T>(*static_cast<const T *>(value), flags, help, usageHelp);
	});
	cv->m_ID = m_conVarID;
	m_conVars.insert(std::map<std::string, std::shared_ptr<ConConf>>::value_type(lscmd, cv));
	m_conVarIDs.insert(std::unordered_map<std::string, unsigned int>::value_type(lscmd, m_conVarID));
	m_conVarIdentifiers.insert(std::unordered_map<unsigned int, std::string>::value_type(m_conVarID, lscmd));
	m_conVarID++;
	return cv;
}

std::shared_ptr<ConVar> ConVarMap::RegisterConVar(const ConVarCreateInfo &createInfo) { return RegisterConVar(createInfo.name, createInfo.type, createInfo.defaultValue.get(), createInfo.flags, createInfo.helpText, createInfo.usageHelp); }

template<class T>
static void register_convar_callback(const std::string &scvar, const std::function<void(NetworkState *, ConVar *, T, T)> &function, std::unordered_map<std::string, std::vector<std::shared_ptr<CvarCallbackFunction>>> &callbacks)
{
	auto lscvar = scvar;
	ustring::to_lower(lscvar);
	auto it = callbacks.find(lscvar);
	if(it == callbacks.end()) {
		callbacks.insert(std::unordered_map<std::string, std::vector<std::shared_ptr<CvarCallbackFunction>>>::value_type(lscvar, std::vector<std::shared_ptr<CvarCallbackFunction>> {}));
		it = callbacks.find(lscvar);
	}
	else {
		auto &callbacks = it->second;
		for(auto &callback : callbacks) {
			if(!callback->IsValid()) {
				callback->SetFunction(function);
				return;
			}
		}
	}
	it->second.push_back(std::make_shared<CvarCallbackFunction>(function));
}

void ConVarMap::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, ConVar *, int, int)> &function) { register_convar_callback<int>(scvar, function, m_conVarCallbacks); }

void ConVarMap::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, ConVar *, std::string, std::string)> &function) { register_convar_callback<std::string>(scvar, function, m_conVarCallbacks); }

void ConVarMap::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, ConVar *, float, float)> &function) { register_convar_callback<float>(scvar, function, m_conVarCallbacks); }

void ConVarMap::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, ConVar *, bool, bool)> &function) { register_convar_callback<bool>(scvar, function, m_conVarCallbacks); }

std::shared_ptr<ConCommand> ConVarMap::PreRegisterConCommand(const std::string &scmd, ConVarFlags flags, const std::string &help)
{
	auto lscmd = scmd;
	ustring::to_lower(lscmd);
	if(m_conVars.find(lscmd) != m_conVars.end())
		return nullptr;
	auto cmd = std::make_shared<ConCommand>(static_cast<void (*)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)>(nullptr), flags, help);
	cmd->m_ID = m_conVarID;
	m_conVars.insert(decltype(m_conVars)::value_type(lscmd, cmd));
	m_conVarIDs.insert(decltype(m_conVarIDs)::value_type(lscmd, m_conVarID));
	m_conVarIdentifiers.insert(decltype(m_conVarIdentifiers)::value_type(m_conVarID, lscmd));
	m_conVarID++;
	return cmd;
}

void ConVarMap::PreRegisterConVarCallback(const std::string &scvar)
{
	auto lscvar = scvar;
	ustring::to_lower(lscvar);
	auto it = m_conVarCallbacks.find(lscvar);
	if(it == m_conVarCallbacks.end()) {
		m_conVarCallbacks.insert(decltype(m_conVarCallbacks)::value_type(lscvar, std::vector<std::shared_ptr<CvarCallbackFunction>> {}));
		it = m_conVarCallbacks.find(lscvar);
	}
	it->second.push_back(std::make_shared<CvarCallbackFunction>());
}

std::shared_ptr<ConCommand> ConVarMap::RegisterConCommand(const std::string &scmd, const std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &fc, ConVarFlags flags, const std::string &help,
  const std::function<void(const std::string &, std::vector<std::string> &)> &autoCompleteCallback)
{
	auto lscmd = scmd;
	ustring::to_lower(lscmd);
	auto it = m_conVars.find(lscmd);
	if(it != m_conVars.end()) {
		if(it->second->GetType() == ConType::Cmd) // C++ defined ConCommand
		{
			auto *cmd = static_cast<ConCommand *>(it->second.get());
			cmd->SetFunction(fc);
		}
		return nullptr;
	}
	auto cmd = std::make_shared<ConCommand>(fc, flags, help, autoCompleteCallback);
	cmd->m_ID = m_conVarID;
	m_conVars.insert(decltype(m_conVars)::value_type(lscmd, cmd));
	m_conVarIDs.insert(decltype(m_conVarIDs)::value_type(lscmd, m_conVarID));
	m_conVarIdentifiers.insert(decltype(m_conVarIdentifiers)::value_type(m_conVarID, lscmd));
	m_conVarID++;
	return cmd;
}

std::shared_ptr<ConCommand> ConVarMap::RegisterConCommand(const ConCommandCreateInfo &createInfo) { return RegisterConCommand(createInfo.name, createInfo.callbackFunction, createInfo.flags, createInfo.helpText, createInfo.autoComplete); }

std::shared_ptr<ConConf> ConVarMap::GetConVar(const std::string &scmd)
{
	auto lscmd = scmd;
	ustring::to_lower(lscmd);
	auto it = m_conVars.find(lscmd);
	if(it == m_conVars.end())
		return nullptr;
	return it->second;
}

std::unordered_map<std::string, std::vector<std::shared_ptr<CvarCallbackFunction>>> &ConVarMap::GetConVarCallbacks() { return m_conVarCallbacks; }

std::map<std::string, std::shared_ptr<ConConf>> &ConVarMap::GetConVars() { return m_conVars; }

unsigned int ConVarMap::GetConVarID(const std::string &scmd)
{
	auto lscmd = scmd;
	ustring::to_lower(lscmd);
	auto it = m_conVarIDs.find(lscmd);
	if(it == m_conVarIDs.end())
		return 0;
	return it->second;
}

bool ConVarMap::GetConVarIdentifier(unsigned int ID, std::string **str)
{
	auto it = m_conVarIdentifiers.find(ID);
	if(it == m_conVarIdentifiers.end())
		return false;
	*str = &it->second;
	return true;
}
