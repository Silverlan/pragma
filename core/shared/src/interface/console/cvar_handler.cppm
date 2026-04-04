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
	struct DLLNETWORK ConCommandResult {
		bool success = false;
		std::optional<std::string> errorMessage;

		[[nodiscard]] operator bool() const noexcept { return success; }
	};

	class DLLNETWORK CVarHandler {
		// For internal use only! Not to be used directly!
	  protected:
		static ConVarHandle GetConVarHandle(string::StringMap<std::shared_ptr<PtrConVar>> &ptrs, std::string_view scvar);
	  public:
		virtual string::StringMap<std::shared_ptr<PtrConVar>> &GetConVarPtrs() = 0;
		//
	  protected:
		string::OrderedStringMap<std::shared_ptr<ConConf>> m_conVars;
		string::StringMap<std::vector<CvarCallback>> m_cvarCallbacks; // Engine-Level Callbacks only
		string::StringMap<unsigned int> m_conCommandIDs;
		struct SimilarCmdInfo {
			std::size_t similarity = 0;
			const std::string *cmd = nullptr;
		};

		template<typename T>
		CallbackHandle RegisterConVarCallback(std::string_view scvar, const std::function<void(NetworkState *, const ConVar &, T, T)> &function);
		void FindSimilarConVars(std::string_view input, const string::OrderedStringMap<std::shared_ptr<ConConf>> &cvars, std::vector<SimilarCmdInfo> &similarCmds) const;
		virtual void Initialize();
	  public:
		struct DLLNETWORK SetConVarResult {
			ConVar *conVar = nullptr;
			bool success = false;
			std::optional<std::string> errorMessage;

			[[nodiscard]] operator bool() const noexcept { return success; }
		};

		CVarHandler();
		virtual ~CVarHandler();
		void ClearCommands();

		template<class T>
		T *GetConVar(std::string_view scmd)
		{
			ConConf *cv = GetConVar(scmd);
			if(cv == nullptr)
				return nullptr;
			return static_cast<T *>(cv);
		}
		ConConf *GetConVar(std::string_view scmd);

		int GetConVarInt(std::string_view scmd);
		std::string GetConVarString(std::string_view scmd);
		float GetConVarFloat(std::string_view scmd);
		bool GetConVarBool(std::string_view scmd);
		ConVarFlags GetConVarFlags(std::string_view scmd);

		bool GetConVarInt(std::string_view scmd, int32_t &outVal);
		bool GetConVarString(std::string_view scmd, std::string &outVal);
		bool GetConVarFloat(std::string_view scmd, float &outVal);
		bool GetConVarBool(std::string_view scmd, bool &outVal);
		bool GetConVarFlags(std::string_view scmd, ConVarFlags &outVal);

		virtual ConVarMap *GetConVarMap();
		string::OrderedStringMap<std::shared_ptr<ConConf>> &GetConVars();
		const string::OrderedStringMap<std::shared_ptr<ConConf>> &GetConVars() const;
		unsigned int GetConVarID(std::string_view scmd);
		virtual SetConVarResult SetConVar(std::string_view scmd, const std::string &value, bool bApplyIfEqual = false);
		std::vector<std::string> FindSimilarConVars(std::string_view input, std::size_t maxCount = 10) const;
		bool InvokeConVarChangeCallbacks(std::string_view cvarName);

		// These should be avoided, because convars and commands registered this way
		// cannot be synchronized between server and client
		std::shared_ptr<ConVar> RegisterConVar(const std::string &scmd, udm::Type type, const std::string &value, ConVarFlags flags, const std::string &help = "");
		std::shared_ptr<ConCommand> RegisterConCommand(const std::string &scmd, const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &fc, ConVarFlags flags = ConVarFlags::None, const std::string &help = "");
		CallbackHandle RegisterConVarCallback(std::string_view scvar, const std::function<void(NetworkState *, const ConVar &, int, int)> &function);
		CallbackHandle RegisterConVarCallback(std::string_view scvar, const std::function<void(NetworkState *, const ConVar &, std::string, std::string)> &function);
		CallbackHandle RegisterConVarCallback(std::string_view scvar, const std::function<void(NetworkState *, const ConVar &, float, float)> &function);
		CallbackHandle RegisterConVarCallback(std::string_view scvar, const std::function<void(NetworkState *, const ConVar &, bool, bool)> &function);

		virtual void implFindSimilarConVars(std::string_view input, std::vector<SimilarCmdInfo> &similarCmds) const;
	};
#pragma warning(pop)
};
