// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include <cassert>

module pragma.shared;

import :console.convar;

pragma::console::ConConf::ConConf(ConVarFlags flags) : m_help(""), m_ID(0), m_type(ConType::Var), m_flags(flags) {}
const std::string &pragma::console::ConConf::GetHelpText() const { return m_help; }
const std::string &pragma::console::ConConf::GetUsageHelp() const { return m_usageHelp; }
pragma::console::ConType pragma::console::ConConf::GetType() const { return m_type; }
pragma::console::ConConf *pragma::console::ConConf::Copy() { return nullptr; }
uint32_t pragma::console::ConConf::GetID() const { return m_ID; }
void pragma::console::ConConf::SetID(uint32_t ID) { m_ID = ID; }
pragma::console::ConVarFlags pragma::console::ConConf::GetFlags() const { return m_flags; }

void pragma::console::ConConf::Print(const std::string &name)
{
	auto type = GetType();
	set_console_color(ConsoleColorFlags::White | ConsoleColorFlags::Intensity);
	if(type == ConType::Var) {
		ConVar *cvar = static_cast<ConVar *>(this);
		auto flags = cvar->GetFlags();
		if(math::is_flag_set(flags, ConVarFlags::Hidden) || math::is_flag_set(flags, ConVarFlags::Password))
			return;
		Con::COUT << "\"" << name << "\" = \"" << cvar->GetString() << "\" (Type: " << magic_enum::enum_name(cvar->GetVarType()) << ") (Default: " << cvar->GetDefault() << ")" << Con::endl;
		if(flags > ConVarFlags::None) {
			set_console_color(ConsoleColorFlags::White | ConsoleColorFlags::Intensity);
			if((flags & ConVarFlags::Cheat) == ConVarFlags::Cheat)
				Con::COUT << " cheat";
			if((flags & ConVarFlags::Singleplayer) == ConVarFlags::Singleplayer)
				Con::COUT << " singleplayer";
			if((flags & ConVarFlags::Userinfo) == ConVarFlags::Userinfo)
				Con::COUT << " userinfo";
			if((flags & ConVarFlags::Replicated) == ConVarFlags::Replicated)
				Con::COUT << " replicated";
			if((flags & ConVarFlags::Archive) == ConVarFlags::Archive)
				Con::COUT << " archive";
			if((flags & ConVarFlags::Notify) == ConVarFlags::Notify)
				Con::COUT << " notify";
			Con::COUT << Con::endl;
			static_assert(math::to_integral(ConVarFlags::Last) == 256);
		}
	}
	else
		Con::COUT << "\"" << name << "\"" << Con::endl;
	set_console_color(ConsoleColorFlags::White | ConsoleColorFlags::Intensity);
	Con::COUT << GetHelpText() << Con::endl;
}

//////////////////////////////////

pragma::console::ConVarValue pragma::console::create_convar_value(udm::Type type, const void *value)
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
pragma::console::ConVarValue pragma::console::create_convar_value() { return create_convar_value(udm::Type::Invalid, nullptr); }
pragma::console::ConVar::ConVar(udm::Type type, const void *value, ConVarFlags flags, const std::string &help, const std::string &usageHelp) : ConConf {}, m_varType {type}
{
	assert(value != nullptr);
	m_type = ConType::Var;
	m_help = help;
	m_usageHelp = usageHelp;
	m_value = create_convar_value(type, value);
	m_default = create_convar_value(type, value);
	m_flags = flags;
}
void pragma::console::ConVar::SetValue(const std::string &val)
{
	console::visit(m_varType, [this, &val](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<std::string, T>())
			*static_cast<T *>(m_value.get()) = udm::convert<std::string, T>(val);
	});
}
std::string pragma::console::ConVar::GetString() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, std::string>())
			return udm::convert<T, std::string>(*static_cast<T *>(m_value.get()));
		return std::string {};
	});
}
std::string pragma::console::ConVar::GetDefault() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, std::string>())
			return udm::convert<T, std::string>(*static_cast<T *>(m_default.get()));
		return std::string {};
	});
}
int32_t pragma::console::ConVar::GetInt() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, int32_t>())
			return udm::convert<T, int32_t>(*static_cast<T *>(m_value.get()));
		return 0;
	});
}
float pragma::console::ConVar::GetFloat() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, float>())
			return udm::convert<T, float>(*static_cast<T *>(m_value.get()));
		return 0.f;
	});
}
bool pragma::console::ConVar::GetBool() const
{
	return console::visit(m_varType, [this](auto tag) {
		using T = typename decltype(tag)::type;
		if constexpr(udm::is_convertible<T, bool>())
			return udm::convert<T, bool>(*static_cast<T *>(m_value.get()));
		return false;
	});
}
void pragma::console::ConVar::AddCallback(int function) { m_callbacks.push_back(function); }
pragma::console::ConConf *pragma::console::ConVar::Copy()
{
	auto *cvar = new ConVar {m_varType, m_value.get(), m_flags, m_help, m_usageHelp};
	cvar->m_ID = m_ID;
	return static_cast<ConConf *>(cvar);
}

//////////////////////////////////

pragma::console::ConCommand::ConCommand(const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &function, ConVarFlags flags, const std::string &help,
  const std::function<void(const std::string &, std::vector<std::string> &, bool)> &autoCompleteCallback)
    : ConConf(flags), m_function(function), m_functionLua(nullptr), m_autoCompleteCallback {autoCompleteCallback}
{
	m_help = help;
	m_type = ConType::Cmd;
}
pragma::console::ConCommand::ConCommand(const LuaFunction &function, ConVarFlags flags, const std::string &help, const std::function<void(const std::string &, std::vector<std::string> &, bool)> &autoCompleteCallback)
    : ConConf(flags), m_function(nullptr), m_functionLua(function), m_autoCompleteCallback {autoCompleteCallback}
{
	m_help = help;
	m_type = ConType::LuaCmd;
}
const std::function<void(const std::string &, std::vector<std::string> &, bool)> &pragma::console::ConCommand::GetAutoCompleteCallback() const { return m_autoCompleteCallback; }
void pragma::console::ConCommand::SetAutoCompleteCallback(const std::function<void(const std::string &, std::vector<std::string> &, bool)> &callback) { m_autoCompleteCallback = callback; }
void pragma::console::ConCommand::GetFunction(LuaFunction &function) const { function = m_functionLua; }
void pragma::console::ConCommand::GetFunction(std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &function) const { function = m_function; }
void pragma::console::ConCommand::SetFunction(const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &function)
{
	m_type = ConType::Cmd;
	m_function = function;
}
pragma::console::ConConf *pragma::console::ConCommand::Copy()
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

pragma::console::CvarCallback::CvarCallback() {}
pragma::console::CvarCallback::CvarCallback(LuaFunction fc) : CvarCallback {}
{
	m_isLuaCallback = true;
	auto f = [fc](NetworkState *nw, const ConVar &cvar, const void *poldVal, const void *pnewVal) mutable {
		auto *game = nw ? nw->GetGameState() : nullptr;
		if(!game)
			return;
		udm::visit(cvar.GetVarType(), [game, &fc, poldVal, pnewVal](auto tag) {
			using T = typename decltype(tag)::type;
			if constexpr(console::is_valid_convar_type_v<T>) {
				auto prevVal = *static_cast<const T *>(poldVal);
				auto &newVal = *static_cast<const T *>(pnewVal);
				game->ProtectedLuaCall(
				  [&prevVal, &newVal, &fc](lua::State *l) {
					  fc.GetLuaObject().push(l);
					  Lua::Push(l, prevVal);
					  Lua::Push(l, newVal);
					  return Lua::StatusCode::Ok;
				  },
				  0);
			}
		});
	};
	SetFunction(f);
}
pragma::console::CvarCallback::CvarCallback(const std::function<void(NetworkState *, const ConVar &, const void *, const void *)> &f) : CvarCallback() { SetFunction(f); }
bool pragma::console::CvarCallback::IsLuaFunction() const { return m_isLuaCallback; }
CallbackHandle &pragma::console::CvarCallback::GetFunction() { return m_callbackHandle; }
void pragma::console::CvarCallback::SetFunction(const std::function<void(NetworkState *, const ConVar &, const void *, const void *)> &f) { m_callbackHandle = FunctionCallback<void, NetworkState *, const ConVar &, const void *, const void *>::Create(f); }

//////////////////////////////////

static void initialize_convar_map(pragma::console::ConVarMap *&r)
{
	if(r != nullptr)
		return;
	// Linux sets the object to NULL when using unique_ptr in some cases,
	// which causes it to be initialized multiple times, so we'll use a regular pointer instead.
	// Ugly, but it works.
	//r = std::unique_ptr<ConVarMap>(new ConVarMap{});
	r = new pragma::console::ConVarMap {};
}

#define cvar_newglobal(suffix, glname)                                                                                                                                                                                                                                                           \
	DLLNETWORK pragma::console::ConVarMap *g_ConVars##suffix = nullptr;                                                                                                                                                                                                                                           \
	bool pragma::console::glname::register_convar(const std::string &cvar, udm::Type type, const std::string &value, pragma::console::ConVarFlags flags, const std::string &help)                                                                                                                 \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		udm::visit(type, [&cvar, type, &value, flags, &help](auto tag) {                                                                                                                                                                                                                         \
			using T = typename decltype(tag)::type;                                                                                                                                                                                                                                              \
			if constexpr(console::is_valid_convar_type_v<T> && udm::is_convertible<std::string, T>()) {                                                                                                                                                                                          \
				g_ConVars##suffix->RegisterConVar<T>(cvar, udm::convert<std::string, T>(value), flags, help);                                                                                                                                                                                    \
			}                                                                                                                                                                                                                                                                                    \
		});                                                                                                                                                                                                                                                                                      \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool pragma::console::glname::register_convar_callback(const std::string &scvar, int)                                                                                                                                                                                                         \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->PreRegisterConVarCallback(scvar);                                                                                                                                                                                                                                     \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool pragma::console::glname::register_convar_callback(const std::string &scvar, void (*function)(pragma::NetworkState *, const pragma::console::ConVar &, int, int))                                                                                                                                                  \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->RegisterConVarCallback(scvar, std::function<void(pragma::NetworkState *, const pragma::console::ConVar &, int, int)>(function));                                                                                                                                                               \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool pragma::console::glname::register_convar_callback(const std::string &scvar, void (*function)(pragma::NetworkState *, const pragma::console::ConVar &, std::string, std::string))                                                                                                                                  \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->RegisterConVarCallback(scvar, std::function<void(pragma::NetworkState *, const pragma::console::ConVar &, std::string, std::string)>(function));                                                                                                                                               \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool pragma::console::glname::register_convar_callback(const std::string &scvar, void (*function)(pragma::NetworkState *, const pragma::console::ConVar &, float, float))                                                                                                                                              \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->RegisterConVarCallback(scvar, std::function<void(pragma::NetworkState *, const pragma::console::ConVar &, float, float)>(function));                                                                                                                                                           \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool pragma::console::glname::register_convar_callback(const std::string &scvar, void (*function)(pragma::NetworkState *, const pragma::console::ConVar &, bool, bool))                                                                                                                                                \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		g_ConVars##suffix->RegisterConVarCallback(scvar, std::function<void(pragma::NetworkState *, const pragma::console::ConVar &, bool, bool)>(function));                                                                                                                                                             \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	static bool register_concommand_##glname(const std::string &cvar, const std::function<void(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &function, pragma::console::ConVarFlags flags, const std::string &help)                                        \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		if(function == nullptr)                                                                                                                                                                                                                                                                  \
			g_ConVars##suffix->PreRegisterConCommand(cvar, flags, help);                                                                                                                                                                                                                         \
		else                                                                                                                                                                                                                                                                                     \
			g_ConVars##suffix->RegisterConCommand(cvar, function, flags, help);                                                                                                                                                                                                                  \
		return true;                                                                                                                                                                                                                                                                             \
	}                                                                                                                                                                                                                                                                                            \
	bool pragma::console::glname::register_concommand(const std::string &cvar, void (*function)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), pragma::console::ConVarFlags flags, const std::string &help)                                                   \
	{                                                                                                                                                                                                                                                                                            \
		return ::register_concommand_##glname(cvar, function, flags, help);                                                                                                                                                                                                                      \
	}                                                                                                                                                                                                                                                                                            \
	bool pragma::console::glname::register_concommand(const std::string &cvar, void (*function)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), pragma::console::ConVarFlags flags, const std::string &help)                                                          \
	{                                                                                                                                                                                                                                                                                            \
		return ::register_concommand_##glname(cvar, std::bind(function, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), flags, help);                                                                                                                                      \
	}                                                                                                                                                                                                                                                                                            \
	bool pragma::console::glname::register_concommand(const std::string &cvar, void (*function)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), const std::string &help)                                                                                       \
	{                                                                                                                                                                                                                                                                                            \
		return register_concommand(cvar, function, pragma::console::ConVarFlags::None, help);                                                                                                                                                                                                    \
	}                                                                                                                                                                                                                                                                                            \
	bool pragma::console::glname::register_concommand(const std::string &cvar, void (*function)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), const std::string &help) { return register_concommand(cvar, function, pragma::console::ConVarFlags::None, help); }    \
	pragma::console::ConVarMap *pragma::console::glname::get_convar_map()                                                                                                                                                                                                                                          \
	{                                                                                                                                                                                                                                                                                            \
		initialize_convar_map(g_ConVars##suffix);                                                                                                                                                                                                                                                \
		return g_ConVars##suffix;                                                                                                                                                                                                                                                                \
	}

cvar_newglobal(Sv, server);
cvar_newglobal(Cl, client);
cvar_newglobal(En, engine);

pragma::console::ConVarMap::ConVarMap() : m_conVarID(1) {}

std::shared_ptr<pragma::console::ConVar> pragma::console::ConVarMap::RegisterConVar(const std::string &scmd, udm::Type type, const void *value, ConVarFlags flags, const std::string &help, const std::optional<std::string> &optUsageHelp,
  std::function<void(const std::string &, std::vector<std::string> &, bool)> autoCompleteFunction)
{
	auto lscmd = scmd;
	string::to_lower(lscmd);
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

std::shared_ptr<pragma::console::ConVar> pragma::console::ConVarMap::RegisterConVar(const ConVarCreateInfo &createInfo) { return RegisterConVar(createInfo.name, createInfo.type, createInfo.defaultValue.get(), createInfo.flags, createInfo.helpText, createInfo.usageHelp); }

template<class T>
static CallbackHandle register_convar_callback(const std::string &scvar, const std::function<void(pragma::NetworkState *, const pragma::console::ConVar &, T, T)> &function, std::unordered_map<std::string, std::vector<pragma::console::CvarCallback>> &callbacks)
{
	auto f = [function](pragma::NetworkState *nw, const pragma::console::ConVar &cvar, const void *poldVal, const void *pnewVal) {
		udm::visit(cvar.GetVarType(), [&function, nw, &cvar, poldVal, pnewVal](auto tag) {
			using TCv = typename decltype(tag)::type;
			if constexpr(udm::is_convertible<TCv, T>()) {
				auto prevVal = udm::convert<TCv, T>(*static_cast<const TCv *>(poldVal));
				auto newVal = udm::convert<TCv, T>(*static_cast<const TCv *>(pnewVal));
				function(nw, cvar, prevVal, newVal);
			}
		});
	};

	auto lscvar = scvar;
	pragma::string::to_lower(lscvar);
	auto it = callbacks.find(lscvar);
	if(it == callbacks.end()) {
		callbacks.insert(std::unordered_map<std::string, std::vector<pragma::console::CvarCallback>>::value_type(lscvar, std::vector<pragma::console::CvarCallback> {}));
		it = callbacks.find(lscvar);
	}
	else {
		auto &callbacks = it->second;
		for(auto &callback : callbacks) {
			auto &func = callback.GetFunction();
			if(!func.IsValid()) {
				callback.SetFunction(f);
				return callback.GetFunction();
			}
		}
	}
	it->second.push_back(pragma::console::CvarCallback {f});
	return it->second.back().GetFunction();
}

CallbackHandle pragma::console::ConVarMap::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, int, int)> &function) { return register_convar_callback<int>(scvar, function, m_conVarCallbacks); }

CallbackHandle pragma::console::ConVarMap::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, std::string, std::string)> &function) { return register_convar_callback<std::string>(scvar, function, m_conVarCallbacks); }

CallbackHandle pragma::console::ConVarMap::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, float, float)> &function) { return register_convar_callback<float>(scvar, function, m_conVarCallbacks); }

CallbackHandle pragma::console::ConVarMap::RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, bool, bool)> &function) { return register_convar_callback<bool>(scvar, function, m_conVarCallbacks); }

std::shared_ptr<pragma::console::ConCommand> pragma::console::ConVarMap::PreRegisterConCommand(const std::string &scmd, ConVarFlags flags, const std::string &help)
{
	auto lscmd = scmd;
	string::to_lower(lscmd);
	if(m_conVars.find(lscmd) != m_conVars.end())
		return nullptr;
	auto cmd = pragma::util::make_shared<ConCommand>(static_cast<void (*)(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)>(nullptr), flags, help);
	cmd->m_ID = m_conVarID;
	m_conVars.insert(decltype(m_conVars)::value_type(lscmd, cmd));
	m_conVarIDs.insert(decltype(m_conVarIDs)::value_type(lscmd, m_conVarID));
	m_conVarIdentifiers.insert(decltype(m_conVarIdentifiers)::value_type(m_conVarID, lscmd));
	m_conVarID++;
	return cmd;
}

void pragma::console::ConVarMap::PreRegisterConVarCallback(const std::string &scvar)
{
	auto lscvar = scvar;
	string::to_lower(lscvar);
	auto it = m_conVarCallbacks.find(lscvar);
	if(it == m_conVarCallbacks.end()) {
		m_conVarCallbacks.insert(decltype(m_conVarCallbacks)::value_type(lscvar, std::vector<CvarCallback> {}));
		it = m_conVarCallbacks.find(lscvar);
	}
	it->second.push_back(CvarCallback {});
}

std::shared_ptr<pragma::console::ConCommand> pragma::console::ConVarMap::RegisterConCommand(const std::string &scmd, const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &fc, ConVarFlags flags, const std::string &help,
  const std::function<void(const std::string &, std::vector<std::string> &, bool)> &autoCompleteCallback)
{
	auto lscmd = scmd;
	string::to_lower(lscmd);
	auto it = m_conVars.find(lscmd);
	if(it != m_conVars.end()) {
		if(it->second->GetType() == ConType::Cmd) // C++ defined ConCommand
		{
			auto *cmd = static_cast<ConCommand *>(it->second.get());
			cmd->SetFunction(fc);
		}
		return nullptr;
	}
	auto cmd = pragma::util::make_shared<ConCommand>(fc, flags, help, autoCompleteCallback);
	cmd->m_ID = m_conVarID;
	m_conVars.insert(decltype(m_conVars)::value_type(lscmd, cmd));
	m_conVarIDs.insert(decltype(m_conVarIDs)::value_type(lscmd, m_conVarID));
	m_conVarIdentifiers.insert(decltype(m_conVarIdentifiers)::value_type(m_conVarID, lscmd));
	m_conVarID++;
	return cmd;
}

std::shared_ptr<pragma::console::ConCommand> pragma::console::ConVarMap::RegisterConCommand(const std::string &scmd, const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &fc, ConVarFlags flags, const std::string &help,
  const std::function<void(const std::string &, std::vector<std::string> &)> &autoCompleteCallback)
{
	return RegisterConCommand(scmd, fc, flags, help, [autoCompleteCallback](const std::string &arg, std::vector<std::string> &options, bool) { return autoCompleteCallback(arg, options); });
}

std::shared_ptr<pragma::console::ConCommand> pragma::console::ConVarMap::RegisterConCommand(const ConCommandCreateInfo &createInfo) { return RegisterConCommand(createInfo.name, createInfo.callbackFunction, createInfo.flags, createInfo.helpText, createInfo.autoComplete); }

std::shared_ptr<pragma::console::ConConf> pragma::console::ConVarMap::GetConVar(const std::string &scmd)
{
	auto lscmd = scmd;
	string::to_lower(lscmd);
	auto it = m_conVars.find(lscmd);
	if(it == m_conVars.end())
		return nullptr;
	return it->second;
}

std::unordered_map<std::string, std::vector<pragma::console::CvarCallback>> &pragma::console::ConVarMap::GetConVarCallbacks() { return m_conVarCallbacks; }

std::map<std::string, std::shared_ptr<pragma::console::ConConf>> &pragma::console::ConVarMap::GetConVars() { return m_conVars; }

unsigned int pragma::console::ConVarMap::GetConVarID(const std::string &scmd)
{
	auto lscmd = scmd;
	string::to_lower(lscmd);
	auto it = m_conVarIDs.find(lscmd);
	if(it == m_conVarIDs.end())
		return 0;
	return it->second;
}

bool pragma::console::ConVarMap::GetConVarIdentifier(unsigned int ID, std::string **str)
{
	auto it = m_conVarIdentifiers.find(ID);
	if(it == m_conVarIdentifiers.end())
		return false;
	*str = &it->second;
	return true;
}
