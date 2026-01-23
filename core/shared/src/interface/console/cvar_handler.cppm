// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:console.cvar_handler;

export import :console.convar_handle;

export namespace pragma {
	class NetworkState;
}

export namespace pragma::console {
#pragma warning(push)
#pragma warning(disable : 4251)
	class DLLNETWORK CVarHandler {
		// For internal use only! Not to be used directly!
	  protected:
		static ConVarHandle GetConVarHandle(std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &ptrs, std::string scvar);
	  public:
		virtual std::unordered_map<std::string, std::shared_ptr<PtrConVar>> &GetConVarPtrs() = 0;
		//
	  protected:
		std::map<std::string, std::shared_ptr<ConConf>> m_conVars;
		std::unordered_map<std::string, std::vector<CvarCallback>> m_cvarCallbacks; // Engine-Level Callbacks only
		std::unordered_map<std::string, unsigned int> m_conCommandIDs;
		struct SimilarCmdInfo {
			std::size_t similarity = 0;
			const std::string *cmd = nullptr;
		};

		template<typename T>
		CallbackHandle RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, T, T)> &function);
		void FindSimilarConVars(const std::string &input, const std::map<std::string, std::shared_ptr<ConConf>> &cvars, std::vector<SimilarCmdInfo> &similarCmds) const;
		virtual void Initialize();
	  public:
		CVarHandler();
		virtual ~CVarHandler();
		void ClearCommands();

		template<class T>
		T *GetConVar(std::string scmd)
		{
			ConConf *cv = GetConVar(scmd);
			if(cv == nullptr)
				return nullptr;
			return static_cast<T *>(cv);
		}
		ConConf *GetConVar(std::string scmd);

		int GetConVarInt(std::string scmd);
		std::string GetConVarString(std::string scmd);
		float GetConVarFloat(std::string scmd);
		bool GetConVarBool(std::string scmd);
		ConVarFlags GetConVarFlags(std::string scmd);

		bool GetConVarInt(std::string scmd, int32_t &outVal);
		bool GetConVarString(std::string scmd, std::string &outVal);
		bool GetConVarFloat(std::string scmd, float &outVal);
		bool GetConVarBool(std::string scmd, bool &outVal);
		bool GetConVarFlags(std::string scmd, ConVarFlags &outVal);

		virtual ConVarMap *GetConVarMap();
		std::map<std::string, std::shared_ptr<ConConf>> &GetConVars();
		const std::map<std::string, std::shared_ptr<ConConf>> &GetConVars() const;
		unsigned int GetConVarID(std::string scmd);
		virtual ConVar *SetConVar(std::string scmd, std::string value, bool bApplyIfEqual = false);
		std::vector<std::string> FindSimilarConVars(const std::string &input, std::size_t maxCount = 10) const;
		bool InvokeConVarChangeCallbacks(const std::string &cvarName);

		// These should be avoided, because convars and commands registered this way
		// cannot be synchronized between server and client
		std::shared_ptr<ConVar> RegisterConVar(const std::string &scmd, udm::Type type, const std::string &value, ConVarFlags flags, const std::string &help = "");
		std::shared_ptr<ConCommand> RegisterConCommand(const std::string &scmd, const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &fc, ConVarFlags flags = ConVarFlags::None,
		  const std::string &help = "");
		CallbackHandle RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, int, int)> &function);
		CallbackHandle RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, std::string, std::string)> &function);
		CallbackHandle RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, float, float)> &function);
		CallbackHandle RegisterConVarCallback(const std::string &scvar, const std::function<void(NetworkState *, const ConVar &, bool, bool)> &function);

		virtual void implFindSimilarConVars(const std::string &input, std::vector<SimilarCmdInfo> &similarCmds) const;
	};
#pragma warning(pop)
};
