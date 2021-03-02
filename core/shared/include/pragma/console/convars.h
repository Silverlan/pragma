/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CONVARS_H__
#define __CONVARS_H__

#include "pragma/networkdefinitions.h"
#include <pragma/console/fcvar.h>
#include "pragma/networkstate/networkstate.h"
#include "pragma/lua/luafunction.h"

enum class DLLNETWORK ConType : uint32_t
{
	Var = 0,
	Variable = 0,

	Cmd = 1,
	Command = 1,

	LuaCmd = 2,
	LuaCommand = 2
};

class ServerState;
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLNETWORK ConConf
{
public:
	friend CVarHandler;
	friend NetworkState;
	friend ServerState;
	friend ConVarMap;
protected:
	ConConf(ConVarFlags flags=ConVarFlags::None);
	std::string m_help;
	ConType m_type;
	uint32_t m_ID;
	ConVarFlags m_flags;
public:
	const std::string &GetHelpText() const;
	ConType GetType() const;
	virtual ConConf *Copy();
	uint32_t GetID() const;
	void Print(const std::string &name);
	ConVarFlags GetFlags() const;
};

class DLLNETWORK ConVar
	: public ConConf
{
public:
	friend CVarHandler;
	friend NetworkState;
	ConVar();
	ConVar(const std::string &val,const std::string &help="");
	ConVar(const std::string &val,ConVarFlags flags,const std::string &help="");
private:
	std::string m_value;
	std::string m_default;
protected:
	std::vector<int> m_callbacks;
	void SetValue(const std::string &val);
public:
	const std::string &GetString() const;
	const std::string &GetDefault() const;
	int32_t GetInt() const;
	float GetFloat() const;
	bool GetBool() const;
	void AddCallback(int function);
	ConConf *Copy();
};

class DLLNETWORK ConCommand
	: public ConConf
{
public:
	ConCommand(const ConCommand &cv);
	ConCommand(
		const std::function<void(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)> &function,
		ConVarFlags flags=ConVarFlags::None,const std::string &help="",
		const std::function<void(const std::string&,std::vector<std::string>&)> &autoCompleteCallback=nullptr
	);
	ConCommand(
		const LuaFunction &function,ConVarFlags flags=ConVarFlags::None,const std::string &help="",
		const std::function<void(const std::string&,std::vector<std::string>&)> &autoCompleteCallback=nullptr
	);
private:
	std::function<void(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)> m_function;
	LuaFunction m_functionLua;
	std::function<void(const std::string&,std::vector<std::string>&)> m_autoCompleteCallback = nullptr;
public:
	void GetFunction(LuaFunction &function) const;
	void GetFunction(std::function<void(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)> &function) const;
	void SetFunction(const std::function<void(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)> &function);
	const std::function<void(const std::string&,std::vector<std::string>&)> &GetAutoCompleteCallback() const;
	void SetAutoCompleteCallback(const std::function<void(const std::string&,std::vector<std::string>&)> &callback);
	ConConf *Copy();
};

class NetworkState;
class DLLNETWORK CvarCallbackFunction
{
public:
	enum class DLLNETWORK Type : uint32_t
	{
		Invalid = std::numeric_limits<uint32_t>::max(),
		Int = 0,
		String = 1,
		Float = 2,
		Bool = 3
	};
	CvarCallbackFunction();
	CvarCallbackFunction(const std::function<void(NetworkState*,ConVar*,int,int)> &fc);
	CvarCallbackFunction(const std::function<void(NetworkState*,ConVar*,std::string,std::string)> &fc);
	CvarCallbackFunction(const std::function<void(NetworkState*,ConVar*,float,float)> &fc);
	CvarCallbackFunction(const std::function<void(NetworkState*,ConVar*,bool,bool)> &fc);
	CvarCallbackFunction(const CvarCallbackFunction &b);
	void Call(NetworkState *state,ConVar *cvar,const std::string &prev);
private:
	Type m_type;
	std::function<void(NetworkState*,ConVar*,int,int)> m_functionInt;
	std::function<void(NetworkState*,ConVar*,std::string,std::string)> m_functionString;
	std::function<void(NetworkState*,ConVar*,float,float)> m_functionFloat;
	std::function<void(NetworkState*,ConVar*,bool,bool)> m_functionBool;
public:
	bool IsValid();
	void SetFunction(const std::function<void(NetworkState*,ConVar*,int,int)> &fc);
	void SetFunction(const std::function<void(NetworkState*,ConVar*,std::string,std::string)> &fc);
	void SetFunction(const std::function<void(NetworkState*,ConVar*,float,float)> &fc);
	void SetFunction(const std::function<void(NetworkState*,ConVar*,bool,bool)> &fc);
};

class DLLNETWORK CvarCallback
{
public:
	CvarCallback();
	CvarCallback(LuaFunction fc);
	CvarCallback(const std::shared_ptr<CvarCallbackFunction> &fc);
	CvarCallback(const CvarCallback &cv);
private:
	bool m_bLua;
	LuaFunction m_functionLua;
	std::shared_ptr<CvarCallbackFunction> m_function;
public:
	bool IsLuaFunction() const;
	LuaFunction *GetLuaFunction();
	CvarCallbackFunction *GetFunction();
};

struct DLLNETWORK ConVarCreateInfo
{
	ConVarCreateInfo()=default;
	ConVarCreateInfo(const std::string &name,const std::string &defaultValue,ConVarFlags flags={},const std::string &helpText="")
		: name{name},defaultValue{defaultValue},flags{flags},helpText{helpText}
	{}
	std::string name = {};
	std::string defaultValue = {};
	ConVarFlags flags = {};
	std::string helpText = {};
};

struct DLLNETWORK ConCommandCreateInfo
{
	ConCommandCreateInfo()=default;
	ConCommandCreateInfo(const std::string &name,const std::function<void(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)> &function,ConVarFlags flags={},const std::string &helpText={},const std::function<void(const std::string&,std::vector<std::string>&)> &autoComplete=nullptr)
		: callbackFunction{function},name{name},flags{flags},helpText{helpText},autoComplete{autoComplete}
	{}
	std::string name = {};
	std::function<void(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)> callbackFunction = nullptr;
	ConVarFlags flags = {};
	std::string helpText = {};
	std::function<void(const std::string&,std::vector<std::string>&)> autoComplete = nullptr;
};

class DLLNETWORK ConVarMap
{
public:
	ConVarMap();
private:
	std::map<std::string,std::shared_ptr<ConConf>> m_conVars;
	std::unordered_map<std::string,unsigned int> m_conVarIDs;
	std::unordered_map<unsigned int,std::string> m_conVarIdentifiers;
	unsigned int m_conVarID;
	std::unordered_map<std::string,std::vector<std::shared_ptr<CvarCallbackFunction>>> m_conVarCallbacks;
public:
	std::shared_ptr<ConCommand> PreRegisterConCommand(const std::string &scmd,ConVarFlags flags,const std::string &help="");
	void PreRegisterConVarCallback(const std::string &scvar);

	std::shared_ptr<ConVar> RegisterConVar(
		const std::string &scmd,const std::string &value,ConVarFlags flags,const std::string &help="",
		std::function<void(const std::string&,std::vector<std::string>&)> autoCompleteFunction=nullptr
	);
	std::shared_ptr<ConVar> RegisterConVar(const ConVarCreateInfo &createInfo);
	std::shared_ptr<ConCommand> RegisterConCommand(
		const std::string &scmd,const std::function<void(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)> &fc,
		ConVarFlags flags,const std::string &help="",const std::function<void(const std::string&,std::vector<std::string>&)> &autoCompleteCallback=nullptr
	);
	std::shared_ptr<ConCommand> RegisterConCommand(const ConCommandCreateInfo &createInfo);

	void RegisterConVarCallback(const std::string &scvar,const std::function<void(NetworkState*,ConVar*,int,int)> &function);
	void RegisterConVarCallback(const std::string &scvar,const std::function<void(NetworkState*,ConVar*,std::string,std::string)> &function);
	void RegisterConVarCallback(const std::string &scvar,const std::function<void(NetworkState*,ConVar*,float,float)> &function);
	void RegisterConVarCallback(const std::string &scvar,const std::function<void(NetworkState*,ConVar*,bool,bool)> &function);
	std::shared_ptr<ConConf> GetConVar(const std::string &scmd);
	std::map<std::string,std::shared_ptr<ConConf>> &GetConVars();
	unsigned int GetConVarID(const std::string &scmd);
	bool GetConVarIdentifier(unsigned int ID,std::string **str);
	unsigned int GetConVarCount() {return m_conVarID -1;}
	std::unordered_map<std::string,std::vector<std::shared_ptr<CvarCallbackFunction>>> &GetConVarCallbacks();
};
#pragma warning(pop)
#define cvar_newglobal_dec(glname) \
	namespace console_system \
	{ \
		namespace glname \
		{ \
			DLLNETWORK ConVarMap *get_convar_map(); \
			DLLNETWORK bool register_convar(const std::string &cvar,const std::string &value,ConVarFlags flags,const std::string &help); \
			DLLNETWORK bool register_convar_callback(const std::string &scvar,int i); \
			DLLNETWORK bool register_convar_callback(const std::string &scvar,void(*function)(NetworkState*,ConVar*,int,int)); \
			DLLNETWORK bool register_convar_callback(const std::string &scvar,void(*function)(NetworkState*,ConVar*,std::string,std::string)); \
			DLLNETWORK bool register_convar_callback(const std::string &scvar,void(*function)(NetworkState*,ConVar*,float,float)); \
			DLLNETWORK bool register_convar_callback(const std::string &scvar,void(*function)(NetworkState*,ConVar*,bool,bool)); \
			DLLNETWORK bool register_concommand(const std::string &cvar,void(*function)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float),ConVarFlags flags,const std::string &help); \
			DLLNETWORK bool register_concommand(const std::string &cvar,void(*function)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&),ConVarFlags flags,const std::string &help); \
			DLLNETWORK bool register_concommand(const std::string &cvar,void(*function)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float),const std::string &help); \
			DLLNETWORK bool register_concommand(const std::string &cvar,void(*function)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&),const std::string &help); \
		}; \
	};

cvar_newglobal_dec(server);
cvar_newglobal_dec(client);
cvar_newglobal_dec(engine);

#define _REGISTER_CONCOMMAND_SV(cmdName,function,flags,help) \
	auto __reg_cvar_Server_##cmdName = console_system::server::register_concommand(#cmdName,function,flags,help);

#define _REGISTER_CONCOMMAND_CL(cmdName,function,flags,help) \
	auto __reg_cvar_Client_##cmdName = console_system::client::register_concommand(#cmdName,function,flags,help);

#define _REGISTER_CONCOMMAND_EN(cmdName,function,flags,help) \
	auto __reg_cvar_Engine_##cmdName = console_system::engine::register_concommand(#cmdName,function,flags,help);

// Using a combination of __COUNTER__(ctra) and __LINE__(ctrb) to prevent variable name conflicts,
// however they can still occur
#define ___REGISTER_CONVAR_CALLBACK_SV(cvarName,function,ctra,ctrb) \
	auto __reg_cvar_Server_##cmdName##_callback##ctra##ctrb = console_system::server::register_convar_callback(#cvarName,function);

#define __REGISTER_CONVAR_CALLBACK_SV(cvarName,function,ctra,ctrb) \
	___REGISTER_CONVAR_CALLBACK_SV(cvarName,function,ctra,ctrb)

#define _REGISTER_CONVAR_CALLBACK_SV(cvarName,function) \
	__REGISTER_CONVAR_CALLBACK_SV(cvarName,function,__COUNTER__,__LINE__)

#define ___REGISTER_CONVAR_CALLBACK_CL(cvarName,function,ctra,ctrb) \
	auto __reg_cvar_Client_##cmdName##_callback##ctra##ctrb = console_system::client::register_convar_callback(#cvarName,function);

#define __REGISTER_CONVAR_CALLBACK_CL(cvarName,function,ctra,ctrb) \
	___REGISTER_CONVAR_CALLBACK_CL(cvarName,function,ctra,ctrb)

#define _REGISTER_CONVAR_CALLBACK_CL(cvarName,function) \
	__REGISTER_CONVAR_CALLBACK_CL(cvarName,function,__COUNTER__,__LINE__)

#define ___REGISTER_CONVAR_CALLBACK_EN(cvarName,function,ctra,ctrb) \
	auto __reg_cvar_Engine_##cmdName##_callback##ctra##ctrb = console_system::engine::register_convar_callback(#cvarName,function);

#define __REGISTER_CONVAR_CALLBACK_EN(cvarName,function,ctra,ctrb) \
	___REGISTER_CONVAR_CALLBACK_EN(cvarName,function,ctra,ctrb)

#define _REGISTER_CONVAR_CALLBACK_EN(cvarName,function) \
	__REGISTER_CONVAR_CALLBACK_EN(cvarName,function,__COUNTER__,__LINE__)

#ifdef ENGINE_NETWORK // ConCommands are only DECLARED (Unless they're shared) within shared.dll, the function definitions must be done in a different library
	#define REGISTER_CONVAR_SV_INTERNAL(cmdName,value,flags,help) \
		auto __reg_cvar_Server_##cmdName = console_system::server::register_convar(#cmdName,value,flags,help);
	#define REGISTER_CONVAR_SV(cmdName,value,flags,help) \
		static_assert((static_cast<uint32_t>(flags) &static_cast<uint32_t>(ConVarFlags::Replicated)) == 0,"ConVar with replicated flag needs to be shared!"); \
		REGISTER_CONVAR_SV_INTERNAL(cmdName,value,flags,help)

	#define REGISTER_CONVAR_CALLBACK_SV(cvarName,function) \
		_REGISTER_CONVAR_CALLBACK_SV(cvarName,NULL)

	#define REGISTER_CONCOMMAND_SV(cmdName,function,flags,help) \
		auto __reg_cvar_Server_##cmdName = console_system::server::register_concommand(#cmdName,static_cast<void(*)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)>(nullptr),flags,help);

	#define REGISTER_CONCOMMAND_TOGGLE_SV(cmdName,functionIn,functionOut,flags,help) \
		auto __reg_cvar_Server_##cmdName##_in = console_system::server::register_concommand(( "+" #cmdName ),static_cast<void(*)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)>(nullptr),flags,help); \
		auto __reg_cvar_Server_##cmdName##_out = console_system::server::register_concommand(( "-" #cmdName ),static_cast<void(*)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)>(nullptr),flags,help);

	#define REGISTER_CONVAR_CL(cmdName,value,flags,help) \
		auto __reg_cvar_Client_##cmdName = console_system::client::register_convar(#cmdName,value,flags,help);

	#define REGISTER_CONVAR_CALLBACK_CL(cvarName,function) \
		_REGISTER_CONVAR_CALLBACK_CL(cvarName,NULL)

	#define REGISTER_CONCOMMAND_CL(cmdName,function,flags,help) \
		auto __reg_cvar_Client_##cmdName = console_system::client::register_concommand(#cmdName,static_cast<void(*)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)>(nullptr),flags,help);

	#define REGISTER_CONCOMMAND_TOGGLE_CL(cmdName,functionIn,functionOut,flags,help) \
		auto __reg_cvar_Client_##cmdName##_in = console_system::client::register_concommand(( "+" #cmdName ),static_cast<void(*)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)>(nullptr),flags,help); \
		auto __reg_cvar_Client_##cmdName##_out = console_system::client::register_concommand(( "-" #cmdName ),static_cast<void(*)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)>(nullptr),flags,help);

	#define REGISTER_CONVAR_EN(cmdName,value,flags,help) \
		auto __reg_cvar_Engine_##cmdName = console_system::engine::register_convar(#cmdName,value,flags,help);

	#define REGISTER_CONVAR_CALLBACK_EN(cvarName,function) \
		_REGISTER_CONVAR_CALLBACK_EN(cvarName,NULL)

	#define REGISTER_CONCOMMAND_EN(cmdName,function,flags,help) \
		auto __reg_cvar_Engine_##cmdName = console_system::engine::register_concommand(#cmdName,static_cast<void(*)(NetworkState*,pragma::BasePlayerComponent*,std::vector<std::string>&,float)>(nullptr),flags,help);

	#define REGISTER_ENGINE_CONCOMMAND(cmdName,function,flags,help) \
		_REGISTER_CONCOMMAND_EN(cmdName,function,flags,help);

	#define REGISTER_SHARED_CONCOMMAND(cmdName,function,flags,help) \
		_REGISTER_CONCOMMAND_SV(cmdName,function,flags,help); \
		_REGISTER_CONCOMMAND_CL(cmdName,function,flags,help);

	#define REGISTER_ENGINE_CONVAR_CALLBACK(cvarName,function) \
		_REGISTER_CONVAR_CALLBACK_EN(cvarName,function);

	#define REGISTER_SHARED_CONVAR_CALLBACK(cvarName,function) \
		_REGISTER_CONVAR_CALLBACK_SV(cvarName,function); \
		_REGISTER_CONVAR_CALLBACK_CL(cvarName,function);
#else // For ConCommand definitions only
	#define REGISTER_CONVAR_SV_INTERNAL(cmdName,value,flags,help)
	#define REGISTER_CONVAR_SV(cmdName,value,flags,help)
	#define REGISTER_CONVAR_CALLBACK_SV(cvarName,function) \
		_REGISTER_CONVAR_CALLBACK_SV(cvarName,function)
	#define REGISTER_CONCOMMAND_SV(cmdName,function,flags,help) \
		_REGISTER_CONCOMMAND_SV(cmdName,function,flags,help)
	#define REGISTER_CONCOMMAND_TOGGLE_SV(cmdName,functionIn,functionOut,flags,help) \
		_REGISTER_CONCOMMAND_SV("+",cmdName,functionIn,flags,help); \
		_REGISTER_CONCOMMAND_SV("-",cmdName,functionOut,flags,help);

	#define REGISTER_CONVAR_CL(cmdName,value,flags,help)
	#define REGISTER_CONVAR_CALLBACK_CL(cvarName,function) \
		_REGISTER_CONVAR_CALLBACK_CL(cvarName,function)
	#define REGISTER_CONCOMMAND_CL(cmdName,function,flags,help) \
		_REGISTER_CONCOMMAND_CL(cmdName,function,flags,help)
	#define REGISTER_CONCOMMAND_TOGGLE_CL(cmdName,functionIn,functionOut,flags,help) \
		auto __reg_cvar_Client_##cmdName##_in = console_system::client::register_concommand(( "+" #cmdName ),functionIn,flags,help); \
		auto __reg_cvar_Client_##cmdName##_out = console_system::client::register_concommand(( "-" #cmdName ),functionOut,flags,help);

	#define REGISTER_CONVAR_EN(cmdName,value,flags,help)
	#define REGISTER_CONVAR_CALLBACK_EN(cvarName,function) \
		_REGISTER_CONVAR_CALLBACK_EN(cvarName,function)
	#define REGISTER_CONCOMMAND_EN(cmdName,function,flags,help) \
		_REGISTER_CONCOMMAND_EN(cmdName,function,flags,help)

	#define REGISTER_ENGINE_CONCOMMAND(cmdName,function,flags,help) \
		REGISTER_CONCOMMAND_EN(cmdName,function,flags,help);

	#define REGISTER_SHARED_CONCOMMAND(cmdName,function,flags,help) \
		REGISTER_CONCOMMAND_SV(cmdName,function,flags,help); \
		REGISTER_CONCOMMAND_CL(cmdName,function,flags,help);

	#define REGISTER_ENGINE_CONVAR_CALLBACK(cvarName,function) \
		REGISTER_CONVAR_CALLBACK_EN(cvarName,function);

	#define REGISTER_SHARED_CONVAR_CALLBACK(cvarName,function) \
		REGISTER_CONVAR_CALLBACK_SV(cvarName,function); \
		REGISTER_CONVAR_CALLBACK_CL(cvarName,function);
#endif

#define REGISTER_ENGINE_CONVAR(cmdName,value,flags,help) \
	REGISTER_CONVAR_EN(cmdName,value,flags,help);

#define REGISTER_SHARED_CONVAR(cmdName,value,flags,help) \
	REGISTER_CONVAR_SV_INTERNAL(cmdName,value,flags,help); \
	REGISTER_CONVAR_CL(cmdName,value,flags,help);

#endif
