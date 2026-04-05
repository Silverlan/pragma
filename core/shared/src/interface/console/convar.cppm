// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:console.convar;

export import :console.cvar_callback;
export import :console.enums;
export import pragma.udm;

#define cvar_newglobal_dec(glname)                                                                                                                                                                                                                                                               \
	namespace glname {                                                                                                                                                                                                                                                                           \
		DLLNETWORK ConVarMap *get_convar_map();                                                                                                                                                                                                                                                  \
		DLLNETWORK bool register_convar(const std::string &cvar, udm::Type type, const std::string &value, pragma::console::ConVarFlags flags, const std::string &help);                                                                                                                         \
		DLLNETWORK bool register_convar_callback(const std::string &scvar, int i);                                                                                                                                                                                                               \
		DLLNETWORK bool register_convar_callback(const std::string &scvar, void (*function)(pragma::NetworkState *, const pragma::console::ConVar &, int, int));                                                                                                                                 \
		DLLNETWORK bool register_convar_callback(const std::string &scvar, void (*function)(pragma::NetworkState *, const pragma::console::ConVar &, std::string, std::string));                                                                                                                 \
		DLLNETWORK bool register_convar_callback(const std::string &scvar, void (*function)(pragma::NetworkState *, const pragma::console::ConVar &, float, float));                                                                                                                             \
		DLLNETWORK bool register_convar_callback(const std::string &scvar, void (*function)(pragma::NetworkState *, const pragma::console::ConVar &, bool, bool));                                                                                                                               \
		DLLNETWORK bool register_concommand(const std::string &cvar, void (*function)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), pragma::console::ConVarFlags flags, const std::string &help);                                                   \
		DLLNETWORK bool register_concommand(const std::string &cvar, void (*function)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), pragma::console::ConVarFlags flags, const std::string &help);                                                          \
		DLLNETWORK bool register_concommand(const std::string &cvar, void (*function)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &, float), const std::string &help);                                                                                       \
		DLLNETWORK bool register_concommand(const std::string &cvar, void (*function)(pragma::NetworkState *, pragma::BasePlayerComponent *, std::vector<std::string> &), const std::string &help);                                                                                              \
	};

export namespace pragma {
	class NetworkState;
	class BasePlayerComponent;
}

export namespace pragma::console {
	using ConVarValue = std::unique_ptr<void, void (*)(void *)>;
	class CVarHandler;
	class ConVarMap;

	constexpr bool is_valid_convar_type(udm::Type type) { return udm::is_common_type(type); }
	template<typename T>
	concept is_valid_convar_type_v = is_valid_convar_type(udm::type_to_enum<T>());

	template<bool ENABLE_DEFAULT_RETURN = true>
	constexpr decltype(auto) visit(udm::Type type, auto vs)
	{
		return udm::visit_c<ENABLE_DEFAULT_RETURN>(type, vs);
	}

#pragma warning(push)
#pragma warning(disable : 4251)
	class DLLNETWORK ConConf {
	  public:
		friend CVarHandler;
		friend NetworkState;
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
		void SetID(uint32_t ID);
		void Print(const std::string &name);
		ConVarFlags GetFlags() const;
	};

	DLLNETWORK ConVarValue create_convar_value(udm::Type type, const void *value);
	DLLNETWORK ConVarValue create_convar_value();
	class DLLNETWORK ConVar : public ConConf {
	  public:
		template<typename T>
		using ConstraintFunction = std::function<void(T &)>;
		template<typename T>
		using ValidationFunction = std::function<bool(const T &, std::string &)>;
		friend CVarHandler;
		friend NetworkState;
		template<typename T>
		    requires(console::is_valid_convar_type_v<T>)
		static std::shared_ptr<ConVar> Create(const T &value, ConVarFlags flags, const std::string &help = "", const std::string &usageHelp = "")
		{
			return std::shared_ptr<ConVar> {new ConVar {udm::type_to_enum<T>(), &value, flags, help, usageHelp}};
		}
		template<typename T>
		    requires(console::is_valid_convar_type_v<T>)
		void SetValidationFunction(const ValidationFunction<T> &fun)
		{
			if(udm::type_to_enum<T>() != m_varType)
				throw std::invalid_argument {"Validation function type does not match ConVar type!"};
			m_validationFunction = [fun](udm::ConstDataValue value, std::string &outErr) -> bool { return fun(*static_cast<const T *>(value), outErr); };
		}

		template<typename T>
		    requires(console::is_valid_convar_type_v<T>)
		void SetConstraintFunction(const ConstraintFunction<T> &fun)
		{
			if(udm::type_to_enum<T>() != m_varType)
				throw std::invalid_argument {"Constraint function type does not match ConVar type!"};
			m_constraintFunction = [fun](udm::DataValue value) { return fun(*static_cast<T *>(value)); };
		}

		template<typename T>
		    requires(console::is_valid_convar_type_v<T>)
		bool ValidateInput(const T &value, std::string &outErr) const
		{
			if(udm::type_to_enum<T>() != m_varType)
				throw std::invalid_argument {"Input value type does not match ConVar type!"};
			if(!m_validationFunction)
				return true;
			return m_validationFunction(&value, outErr);
		}
		template<typename T>
		    requires(console::is_valid_convar_type_v<T>)
		void ApplyConstraint(T &value) const
		{
			if(udm::type_to_enum<T>() != m_varType)
				throw std::invalid_argument {"Input value type does not match ConVar type!"};
			if(!m_constraintFunction)
				return;
			m_constraintFunction(&value);
		}
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

		template<typename T>
		    requires(console::is_valid_convar_type_v<T>)
		bool SetValue(const T &val, std::string &outErr)
		{
			return console::visit(m_varType, [this, &val, &outErr](auto tag) {
				using TVar = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<T, TVar>()) {
					auto newVal = udm::convert<T, TVar>(val);
					if(!ValidateInput(newVal, outErr))
						return false;
					ApplyConstraint(newVal);
					*static_cast<TVar *>(m_value.get()) = std::move(newVal);
					return true;
				}
				outErr = "Unable to convert value of type " + std::string {udm::enum_type_to_ascii(udm::type_to_enum<T>())} + " to ConVar of type " + std::string {udm::enum_type_to_ascii(udm::type_to_enum<TVar>())};
				return false;
			});
		}
		template<typename T>
		std::optional<T> GetValue(bool applyConstraint = false) const
		{
			if(!m_value)
				return {};
			return console::visit(m_varType, [this, applyConstraint](auto tag) -> std::optional<T> {
				using TVar = typename decltype(tag)::type;
				if constexpr(udm::is_convertible<TVar, T>()) {
					auto val = udm::convert<TVar, T>(*static_cast<TVar *>(m_value.get()));
					if(applyConstraint)
						ApplyConstraint(val);
					return val;
				}
				return {};
			});
		}
	  protected:
		using RawConstraintFunction = std::function<void(udm::DataValue)>;
		using RawValidationFunction = std::function<bool(udm::ConstDataValue, std::string &)>;

		std::vector<int> m_callbacks;

		void SetValidationFunction(const RawValidationFunction &fun);
		const RawValidationFunction &GetValidationFunction() const;

		void SetConstraintFunction(const RawConstraintFunction &fun);
		const RawConstraintFunction &GetConstraintFunction() const;

		bool SetValue(const std::string &val, std::string &outErr);
	  private:
		ConVarValue m_value {nullptr, [](void *) {}};
		ConVarValue m_default {nullptr, [](void *) {}};
		mutable RawConstraintFunction m_constraintFunction;
		mutable RawValidationFunction m_validationFunction;
		udm::Type m_varType = udm::Type::Invalid;
	};

	class DLLNETWORK ConCommand : public ConConf {
	  public:
		ConCommand(const ConCommand &cv);
		ConCommand(const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &function, ConVarFlags flags = ConVarFlags::None, const std::string &help = "",
		  const std::function<void(const std::string &, std::vector<std::string> &, bool)> &autoCompleteCallback = nullptr);
		ConCommand(const LuaFunction &function, ConVarFlags flags = ConVarFlags::None, const std::string &help = "", const std::function<void(const std::string &, std::vector<std::string> &, bool)> &autoCompleteCallback = nullptr);
	  private:
		std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> m_function;
		LuaFunction m_functionLua;
		std::function<void(const std::string &, std::vector<std::string> &, bool)> m_autoCompleteCallback = nullptr;
	  public:
		void GetFunction(LuaFunction &function) const;
		void GetFunction(std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &function) const;
		void SetFunction(const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &function);
		const std::function<void(const std::string &, std::vector<std::string> &, bool)> &GetAutoCompleteCallback() const;
		void SetAutoCompleteCallback(const std::function<void(const std::string &, std::vector<std::string> &, bool)> &callback);
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
		ConCommandCreateInfo(const std::string &name, const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &function, ConVarFlags flags = {}, const std::string &helpText = {},
		  const std::function<void(const std::string &, std::vector<std::string> &)> &autoComplete = nullptr)
		    : callbackFunction {function}, name {name}, flags {flags}, helpText {helpText}, autoComplete {autoComplete}
		{
		}
		std::string name = {};
		std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> callbackFunction = nullptr;
		ConVarFlags flags = {};
		std::string helpText = {};
		std::function<void(const std::string &, std::vector<std::string> &)> autoComplete = nullptr;
	};

	class DLLNETWORK ConVarMap {
	  public:
		ConVarMap();
	  public:
		std::shared_ptr<ConCommand> PreRegisterConCommand(const std::string &scmd, ConVarFlags flags, const std::string &help = "");
		void PreRegisterConVarCallback(const std::string &scvar);

		template<typename T>
		std::shared_ptr<ConVar> RegisterConVar(const std::string &scmd, const T &value, ConVarFlags flags, const std::string &help = "", const std::optional<std::string> &usageHelp = {},
		  std::function<void(const std::string &, std::vector<std::string> &, bool)> autoCompleteFunction = nullptr)
		{
			return RegisterConVar(scmd, udm::type_to_enum<T>(), &value, flags, help, usageHelp, autoCompleteFunction);
		}
		template<typename T>
		std::shared_ptr<ConVar> RegisterConVar(const std::string &scmd, const T &value, ConVarFlags flags, const std::string &help, const std::optional<std::string> &usageHelp, std::function<void(const std::string &, std::vector<std::string> &)> autoCompleteFunction)
		{
			return RegisterConVar<T>(scmd, value, flags, help, usageHelp, [autoCompleteFunction](const std::string &arg, std::vector<std::string> &options, bool) { autoCompleteFunction(arg, options); });
		}
		std::shared_ptr<ConVar> RegisterConVar(const ConVarCreateInfo &createInfo);
		std::shared_ptr<ConCommand> RegisterConCommand(const std::string &scmd, const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &fc, ConVarFlags flags, const std::string &help,
		  const std::function<void(const std::string &, std::vector<std::string> &)> &autoCompleteCallback);
		std::shared_ptr<ConCommand> RegisterConCommand(const std::string &scmd, const std::function<void(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float)> &fc, ConVarFlags flags, const std::string &help = "",
		  const std::function<void(const std::string &, std::vector<std::string> &, bool)> &autoCompleteCallback = nullptr);
		std::shared_ptr<ConCommand> RegisterConCommand(const ConCommandCreateInfo &createInfo);

		CallbackHandle RegisterConVarCallback(std::string_view scvar, const std::function<void(NetworkState *, const ConVar &, int, int)> &function);
		CallbackHandle RegisterConVarCallback(std::string_view scvar, const std::function<void(NetworkState *, const ConVar &, std::string, std::string)> &function);
		CallbackHandle RegisterConVarCallback(std::string_view scvar, const std::function<void(NetworkState *, const ConVar &, float, float)> &function);
		CallbackHandle RegisterConVarCallback(std::string_view scvar, const std::function<void(NetworkState *, const ConVar &, bool, bool)> &function);
		std::shared_ptr<ConConf> GetConVar(std::string_view scmd);
		string::OrderedStringMap<std::shared_ptr<ConConf>> &GetConVars();
		unsigned int GetConVarID(std::string_view scmd);
		bool GetConVarIdentifier(unsigned int ID, std::string **str);
		unsigned int GetConVarCount() { return m_conVarID - 1; }
		string::StringMap<std::vector<CvarCallback>> &GetConVarCallbacks();
	  private:
		string::OrderedStringMap<std::shared_ptr<ConConf>> m_conVars;
		string::StringMap<unsigned int> m_conVarIDs;
		std::unordered_map<unsigned int, std::string> m_conVarIdentifiers;
		unsigned int m_conVarID;
		string::StringMap<std::vector<CvarCallback>> m_conVarCallbacks;

		std::shared_ptr<ConVar> RegisterConVar(const std::string &scmd, udm::Type type, const void *value, ConVarFlags flags, const std::string &help = "", const std::optional<std::string> &usageHelp = {},
		  std::function<void(const std::string &, std::vector<std::string> &, bool)> autoCompleteFunction = nullptr);
	};
#pragma warning(pop)

	cvar_newglobal_dec(server);
	cvar_newglobal_dec(client);
	cvar_newglobal_dec(engine);
};
