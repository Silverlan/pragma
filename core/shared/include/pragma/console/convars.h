// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __CONVARS_H__
#define __CONVARS_H__

#include "pragma/networkdefinitions.h"
#include <pragma/console/fcvar.h>
#include "pragma/networkstate/networkstate.h"
#include "pragma/lua/luafunction.h"
#include "pragma/console/cvar_callback.hpp"
#include <udm.hpp>

enum class DLLNETWORK ConType : uint32_t {
	Var = 0,
	Variable = 0,

	Cmd = 1,
	Command = 1,

	LuaCmd = 2,
	LuaCommand = 2
};

using ConVarValue = std::unique_ptr<void, void (*)(void *)>;
namespace console {
	constexpr bool is_valid_convar_type(udm::Type type) { return udm::is_common_type(type); }
	template<typename T>
	concept is_valid_convar_type_v = is_valid_convar_type(udm::type_to_enum<T>());

	template<bool ENABLE_DEFAULT_RETURN = true>
	constexpr decltype(auto) visit(udm::Type type, auto vs)
	{
		return udm::visit_c<ENABLE_DEFAULT_RETURN>(type, vs);
	}
};

class ServerState;
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLNETWORK ConConf {
  public:
	friend CVarHandler;
	friend NetworkState;
	friend ServerState;
	friend ConVarMap;
  protected:
	ConConf(ConVarFlags flags = ConVarFlags::None);
	std::string m_help;
	std::string m_usageHelp;
	ConType m_type;
	uint32_t m_ID;
	ConVarFlags m_flags;
  public:
	const std::string &GetHelpText() const;
	const std::string &GetUsageHelp() const;
	ConType GetType() const;
	virtual ConConf *Copy();
	uint32_t GetID() const;
	void Print(const std::string &name);
	ConVarFlags GetFlags() const;
};

DLLNETWORK ConVarValue create_convar_value(udm::Type type, const void *value);
DLLNETWORK ConVarValue create_convar_value();
class DLLNETWORK ConVar : public ConConf {
  public:
	friend CVarHandler;
	friend NetworkState;
	template<typename T>
	static std::shared_ptr<ConVar> Create(const T &value, ConVarFlags flags, const std::string &help = "", const std::string &usageHelp = "")
	{
		return std::shared_ptr<ConVar> {new ConVar {udm::type_to_enum<T>(), &value, flags, help, usageHelp}};
	}
  private:
	ConVarValue m_value {nullptr, [](void *) {}};
	ConVarValue m_default {nullptr, [](void *) {}};
	udm::Type m_varType = udm::Type::Invalid;
  protected:
	std::vector<int> m_callbacks;
	void SetValue(const std::string &val);
  public:
	ConVar(udm::Type type, const void *value, ConVarFlags flags, const std::string &help, const std::string &usageHelp);
	std::string GetString() const;
	std::string GetDefault() const;
	udm::Type GetVarType() const { return m_varType; }
	int32_t GetInt() const;
	float GetFloat() const;
	bool GetBool() const;
	void AddCallback(int function);
	ConConf *Copy();

	const ConVarValue &GetRawValue() const { return m_value; }
	const ConVarValue &GetRawDefault() const { return m_default; }
};

class DLLNETWORK ConCommand : public ConConf {
  public:
	ConCommand(const ConCommand &cv);
	ConCommand(const std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &function, ConVarFlags flags = ConVarFlags::None, const std::string &help = "",
	  const std::function<void(const std::string &, std::vector<std::string> &)> &autoCompleteCallback = nullptr);
	ConCommand(const LuaFunction &function, ConVarFlags flags = ConVarFlags::None, const std::string &help = "", const std::function<void(const std::string &, std::vector<std::string> &)> &autoCompleteCallback = nullptr);
  private:
	std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> m_function;
	LuaFunction m_functionLua;
	std::function<void(const std::string &, std::vector<std::string> &)> m_autoCompleteCallback = nullptr;
  public:
	void GetFunction(LuaFunction &function) const;
	void GetFunction(std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &function) const;
	void SetFunction(const std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &function);
	const std::function<void(const std::string &, std::vector<std::string> &)> &GetAutoCompleteCallback() const;
	void SetAutoCompleteCallback(const std::function<void(const std::string &, std::vector<std::string> &)> &callback);
	ConConf *Copy();
};

struct DLLNETWORK ConVarCreateInfo {
	ConVarCreateInfo() = default;
	ConVarCreateInfo(udm::Type type, const std::string &name, const void *pdefaultValue, ConVarFlags flags = {}, const std::string &helpText = "", const std::string &usageHelp = "") : type {type}, name {name}, flags {flags}, helpText {helpText}, usageHelp {usageHelp}
	{
		console::visit(type, [this, type, pdefaultValue](auto tag) {
			using T = typename decltype(tag)::type;
			defaultValue = create_convar_value(type, pdefaultValue);
		});
	}
	udm::Type type = udm::Type::Invalid;
	std::string name = {};
	ConVarValue defaultValue {nullptr, [](void *) {}};
	ConVarFlags flags = {};
	std::string helpText = {};
	std::string usageHelp = {};
};

struct DLLNETWORK ConCommandCreateInfo {
	ConCommandCreateInfo() = default;
	ConCommandCreateInfo(const std::string &name, const std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &function, ConVarFlags flags = {}, const std::string &helpText = {},
	  const std::function<void(const std::string &, std::vector<std::string> &)> &autoComplete = nullptr)
	    : callbackFunction {function}, name {name}, flags {flags}, helpText {helpText}, autoComplete {autoComplete}
	{
	}
	std::string name = {};
	std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> callbackFunction = nullptr;
	ConVarFlags flags = {};
	std::string helpText = {};
	std::function<void(const std::string &, std::vector<std::string> &)> autoComplete = nullptr;
};

class DLLNETWORK ConVarMap {
  public:
	ConVarMap();
  private:
	std::map<std::string, std::shared_ptr<ConConf>> m_conVars;
	std::unordered_map<std::string, unsigned int> m_conVarIDs;
	std::unordered_map<unsigned int, std::string> m_conVarIdentifiers;
	unsigned int m_conVarID;
	std::unordered_map<std::string, std::vector<CvarCallback>> m_conVarCallbacks;

	std::shared_ptr<ConVar> RegisterConVar(const std::string &scmd, udm::Type type, const void *value, ConVarFlags flags, const std::string &help = "", const std::optional<std::string> &usageHelp = {},
	  std::function<void(const std::string &, std::vector<std::string> &)> autoCompleteFunction = nullptr);
  public:
	std::shared_ptr<ConCommand> PreRegisterConCommand(const std::string &scmd, ConVarFlags flags, const std::string &help = "");
	void PreRegisterConVarCallback(const std::string &scvar);

	template<typename T>
	std::shared_ptr<ConVar> RegisterConVar(const std::string &scmd, const T &value, ConVarFlags flags, const std::string &help = "", const std::optional<std::string> &usageHelp = {}, std::function<void(const std::string &, std::vector<std::string> &)> autoCompleteFunction = nullptr)
	{
		return RegisterConVar(scmd, udm::type_to_enum<T>(), &value, flags, help, usageHelp, autoCompleteFunction);
	}
	std::shared_ptr<ConVar> RegisterConVar(const ConVarCreateInfo &createInfo);
	std::shared_ptr<ConCommand> RegisterConCommand(const std::string &scmd, const std::function<void(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)> &fc, ConVarFlags flags, const std::string &help = "",
	  const std::function<void(const std::string &, std::vector<std::string> &)> &autoCompleteCallback = nullptr);
	std::shared_ptr<ConCommand> RegisterConCommand(const ConCommandCreateInfo &createInfo);

	CallbackHandle RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, int, int)> &function);
	CallbackHandle RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, std::string, std::string)> &function);
	CallbackHandle RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, float, float)> &function);
	CallbackHandle RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, bool, bool)> &function);
	std::shared_ptr<ConConf> GetConVar(const std::string &scmd);
	std::map<std::string, std::shared_ptr<ConConf>> &GetConVars();
	unsigned int GetConVarID(const std::string &scmd);
	bool GetConVarIdentifier(unsigned int ID, std::string **str);
	unsigned int GetConVarCount() { return m_conVarID - 1; }
	std::unordered_map<std::string, std::vector<CvarCallback>> &GetConVarCallbacks();
};
#pragma warning(pop)
#define cvar_newglobal_dec(glname)                                                                                                                                                                                                                                                               \
	namespace console_system {                                                                                                                                                                                                                                                                   \
		namespace glname {                                                                                                                                                                                                                                                                       \
			DLLNETWORK ConVarMap *get_convar_map();                                                                                                                                                                                                                                              \
			DLLNETWORK bool register_convar(const std::string &cvar, udm::Type type, const std::string &value, ConVarFlags flags, const std::string &help);                                                                                                                                      \
			DLLNETWORK bool register_convar_callback(const std::string &scvar, int i);                                                                                                                                                                                                           \
			DLLNETWORK bool register_convar_callback(const std::string &scvar, void (*function)(NetworkState *, const ConVar &, int, int));                                                                                                                                                      \
			DLLNETWORK bool register_convar_callback(const std::string &scvar, void (*function)(NetworkState *, const ConVar &, std::string, std::string));                                                                                                                                      \
			DLLNETWORK bool register_convar_callback(const std::string &scvar, void (*function)(NetworkState *, const ConVar &, float, float));                                                                                                                                                  \
			DLLNETWORK bool register_convar_callback(const std::string &scvar, void (*function)(NetworkState *, const ConVar &, bool, bool));                                                                                                                                                    \
			DLLNETWORK bool register_concommand(const std::string &cvar, void (*function)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), ConVarFlags flags, const std::string &help);                                                                        \
			DLLNETWORK bool register_concommand(const std::string &cvar, void (*function)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), ConVarFlags flags, const std::string &help);                                                                               \
			DLLNETWORK bool register_concommand(const std::string &cvar, void (*function)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), const std::string &help);                                                                                           \
			DLLNETWORK bool register_concommand(const std::string &cvar, void (*function)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), const std::string &help);                                                                                                  \
		};                                                                                                                                                                                                                                                                                       \
	};

cvar_newglobal_dec(server);
cvar_newglobal_dec(client);
cvar_newglobal_dec(engine);

#define _REGISTER_CONCOMMAND_SV(cmdName, function, flags, help) auto __reg_cvar_Server_##cmdName = console_system::server::register_concommand(#cmdName, function, flags, help);

#define _REGISTER_CONCOMMAND_CL(cmdName, function, flags, help) auto __reg_cvar_Client_##cmdName = console_system::client::register_concommand(#cmdName, function, flags, help);

#define _REGISTER_CONCOMMAND_EN(cmdName, function, flags, help) auto __reg_cvar_Engine_##cmdName = console_system::engine::register_concommand(#cmdName, function, flags, help);

// Using a combination of __COUNTER__(ctra) and __LINE__(ctrb) to prevent variable name conflicts,
// however they can still occur
#define ___REGISTER_CONVAR_CALLBACK_SV(cvarName, function, ctra, ctrb) auto __reg_cvar_Server_##cmdName##_callback##ctra##ctrb = console_system::server::register_convar_callback(#cvarName, function);

#define __REGISTER_CONVAR_CALLBACK_SV(cvarName, function, ctra, ctrb) ___REGISTER_CONVAR_CALLBACK_SV(cvarName, function, ctra, ctrb)

#define _REGISTER_CONVAR_CALLBACK_SV(cvarName, function) __REGISTER_CONVAR_CALLBACK_SV(cvarName, function, __COUNTER__, __LINE__)

#define ___REGISTER_CONVAR_CALLBACK_CL(cvarName, function, ctra, ctrb) auto __reg_cvar_Client_##cmdName##_callback##ctra##ctrb = console_system::client::register_convar_callback(#cvarName, function);

#define __REGISTER_CONVAR_CALLBACK_CL(cvarName, function, ctra, ctrb) ___REGISTER_CONVAR_CALLBACK_CL(cvarName, function, ctra, ctrb)

#define _REGISTER_CONVAR_CALLBACK_CL(cvarName, function) __REGISTER_CONVAR_CALLBACK_CL(cvarName, function, __COUNTER__, __LINE__)

#define ___REGISTER_CONVAR_CALLBACK_EN(cvarName, function, ctra, ctrb) auto __reg_cvar_Engine_##cmdName##_callback##ctra##ctrb = console_system::engine::register_convar_callback(#cvarName, function);

#define __REGISTER_CONVAR_CALLBACK_EN(cvarName, function, ctra, ctrb) ___REGISTER_CONVAR_CALLBACK_EN(cvarName, function, ctra, ctrb)

#define _REGISTER_CONVAR_CALLBACK_EN(cvarName, function) __REGISTER_CONVAR_CALLBACK_EN(cvarName, function, __COUNTER__, __LINE__)

#ifdef ENGINE_NETWORK // ConCommands are only DECLARED (Unless they're shared) within shared.dll, the function definitions must be done in a different library
#define REGISTER_CONVAR_SV_INTERNAL(cmdName, type, value, flags, help) auto __reg_cvar_Server_##cmdName = console_system::server::register_convar(#cmdName, type, value, flags, help);
#define REGISTER_CONVAR_SV(cmdName, type, value, flags, help)                                                                                                                                                                                                                                    \
	static_assert((static_cast<uint32_t>(flags) & static_cast<uint32_t>(ConVarFlags::Replicated)) == 0, "ConVar with replicated flag needs to be shared!");                                                                                                                                      \
	REGISTER_CONVAR_SV_INTERNAL(cmdName, type, value, flags, help)

#define REGISTER_CONVAR_CALLBACK_SV(cvarName, function) _REGISTER_CONVAR_CALLBACK_SV(cvarName, NULL)

#define REGISTER_CONCOMMAND_SV(cmdName, function, flags, help) auto __reg_cvar_Server_##cmdName = console_system::server::register_concommand(#cmdName, static_cast<void (*)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)>(nullptr), flags, help);

#define REGISTER_CONCOMMAND_TOGGLE_SV(cmdName, functionIn, functionOut, flags, help)                                                                                                                                                                                                             \
	auto __reg_cvar_Server_##cmdName##_in = console_system::server::register_concommand(("+" #cmdName), static_cast<void (*)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)>(nullptr), flags, help);                                                          \
	auto __reg_cvar_Server_##cmdName##_out = console_system::server::register_concommand(("-" #cmdName), static_cast<void (*)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)>(nullptr), flags, help);

#define REGISTER_CONVAR_CL(cmdName, type, value, flags, help) auto __reg_cvar_Client_##cmdName = console_system::client::register_convar(#cmdName, type, value, flags, help);

#define REGISTER_CONVAR_CALLBACK_CL(cvarName, function) _REGISTER_CONVAR_CALLBACK_CL(cvarName, NULL)

#define REGISTER_CONCOMMAND_CL(cmdName, function, flags, help) auto __reg_cvar_Client_##cmdName = console_system::client::register_concommand(#cmdName, static_cast<void (*)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)>(nullptr), flags, help);

#define REGISTER_CONCOMMAND_TOGGLE_CL(cmdName, functionIn, functionOut, flags, help)                                                                                                                                                                                                             \
	auto __reg_cvar_Client_##cmdName##_in = console_system::client::register_concommand(("+" #cmdName), static_cast<void (*)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)>(nullptr), flags, help);                                                          \
	auto __reg_cvar_Client_##cmdName##_out = console_system::client::register_concommand(("-" #cmdName), static_cast<void (*)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)>(nullptr), flags, help);

#define REGISTER_CONVAR_EN(cmdName, type, value, flags, help) auto __reg_cvar_Engine_##cmdName = console_system::engine::register_convar(#cmdName, type, value, flags, help);

#define REGISTER_CONVAR_CALLBACK_EN(cvarName, function) _REGISTER_CONVAR_CALLBACK_EN(cvarName, NULL)

#define REGISTER_CONCOMMAND_EN(cmdName, function, flags, help) auto __reg_cvar_Engine_##cmdName = console_system::engine::register_concommand(#cmdName, static_cast<void (*)(NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float)>(nullptr), flags, help);

#define REGISTER_ENGINE_CONCOMMAND(cmdName, function, flags, help) _REGISTER_CONCOMMAND_EN(cmdName, function, flags, help);

#define REGISTER_SHARED_CONCOMMAND(cmdName, function, flags, help)                                                                                                                                                                                                                               \
	_REGISTER_CONCOMMAND_SV(cmdName, function, flags, help);                                                                                                                                                                                                                                     \
	_REGISTER_CONCOMMAND_CL(cmdName, function, flags, help);

#define REGISTER_ENGINE_CONVAR_CALLBACK(cvarName, function) _REGISTER_CONVAR_CALLBACK_EN(cvarName, function);

#define REGISTER_SHARED_CONVAR_CALLBACK(cvarName, function)                                                                                                                                                                                                                                      \
	_REGISTER_CONVAR_CALLBACK_SV(cvarName, function);                                                                                                                                                                                                                                            \
	_REGISTER_CONVAR_CALLBACK_CL(cvarName, function);
#else // For ConCommand definitions only
#define REGISTER_CONVAR_SV_INTERNAL(cmdName, type, value, flags, help)
#define REGISTER_CONVAR_SV(cmdName, type, value, flags, help)
#define REGISTER_CONVAR_CALLBACK_SV(cvarName, function) _REGISTER_CONVAR_CALLBACK_SV(cvarName, function)
#define REGISTER_CONCOMMAND_SV(cmdName, function, flags, help) _REGISTER_CONCOMMAND_SV(cmdName, function, flags, help)
#define REGISTER_CONCOMMAND_TOGGLE_SV(cmdName, functionIn, functionOut, flags, help)                                                                                                                                                                                                             \
	_REGISTER_CONCOMMAND_SV("+", cmdName, functionIn, flags, help);                                                                                                                                                                                                                              \
	_REGISTER_CONCOMMAND_SV("-", cmdName, functionOut, flags, help);

#define REGISTER_CONVAR_CL(cmdName, type, value, flags, help)
#define REGISTER_CONVAR_CALLBACK_CL(cvarName, function) _REGISTER_CONVAR_CALLBACK_CL(cvarName, function)
#define REGISTER_CONCOMMAND_CL(cmdName, function, flags, help) _REGISTER_CONCOMMAND_CL(cmdName, function, flags, help)
#define REGISTER_CONCOMMAND_TOGGLE_CL(cmdName, functionIn, functionOut, flags, help)                                                                                                                                                                                                             \
	auto __reg_cvar_Client_##cmdName##_in = console_system::client::register_concommand(("+" #cmdName), functionIn, flags, help);                                                                                                                                                                \
	auto __reg_cvar_Client_##cmdName##_out = console_system::client::register_concommand(("-" #cmdName), functionOut, flags, help);

#define REGISTER_CONVAR_EN(cmdName, type, value, flags, help)
#define REGISTER_CONVAR_CALLBACK_EN(cvarName, function) _REGISTER_CONVAR_CALLBACK_EN(cvarName, function)
#define REGISTER_CONCOMMAND_EN(cmdName, function, flags, help) _REGISTER_CONCOMMAND_EN(cmdName, function, flags, help)

#define REGISTER_ENGINE_CONCOMMAND(cmdName, function, flags, help) REGISTER_CONCOMMAND_EN(cmdName, function, flags, help);

#define REGISTER_SHARED_CONCOMMAND(cmdName, function, flags, help)                                                                                                                                                                                                                               \
	REGISTER_CONCOMMAND_SV(cmdName, function, flags, help);                                                                                                                                                                                                                                      \
	REGISTER_CONCOMMAND_CL(cmdName, function, flags, help);

#define REGISTER_ENGINE_CONVAR_CALLBACK(cvarName, function) REGISTER_CONVAR_CALLBACK_EN(cvarName, function);

#define REGISTER_SHARED_CONVAR_CALLBACK(cvarName, function)                                                                                                                                                                                                                                      \
	REGISTER_CONVAR_CALLBACK_SV(cvarName, function);                                                                                                                                                                                                                                             \
	REGISTER_CONVAR_CALLBACK_CL(cvarName, function);
#endif

#define REGISTER_ENGINE_CONVAR(cmdName, type, value, flags, help) REGISTER_CONVAR_EN(cmdName, type, value, flags, help);

#define REGISTER_SHARED_CONVAR(cmdName, type, value, flags, help)                                                                                                                                                                                                                                \
	REGISTER_CONVAR_SV_INTERNAL(cmdName, type, value, flags, help);                                                                                                                                                                                                                              \
	REGISTER_CONVAR_CL(cmdName, type, value, flags, help);

#endif
