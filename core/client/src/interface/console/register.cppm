// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:console.register_commands;

export import pragma.shared;

export namespace pragma::console {
	using DummyReturnValue = bool;
	namespace client {
		template<typename T>
		std::shared_ptr<ConVar> register_variable(const std::string &scmd, const T &value, ConVarFlags flags, const std::string &help = "", const std::optional<std::string> &usageHelp = {})
		{
			client::register_convar(scmd, udm::type_to_enum<T>(), udm::convert<T, udm::String>(value), flags, help);
			return {};
		}

		DummyReturnValue register_command(const std::string &scmd, void (*fc)(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float), ConVarFlags flags, const std::string &help = "");
		DummyReturnValue register_command(const std::string &scmd, void (*fc)(NetworkState *, BasePlayerComponent *, std::vector<std::string> &), ConVarFlags flags, const std::string &help = "");
		DummyReturnValue register_toggle_command(const std::string &scmd, void (*fcIn)(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float), void (*fcOut)(NetworkState *, BasePlayerComponent *, std::vector<std::string> &, float), ConVarFlags flags,
		  const std::string &help = "");
		DummyReturnValue register_toggle_command(const std::string &scmd, void (*fcIn)(NetworkState *, BasePlayerComponent *, std::vector<std::string> &), void (*fcOut)(NetworkState *, BasePlayerComponent *, std::vector<std::string> &), ConVarFlags flags, const std::string &help = "");

		template<typename T>
		    requires(pragma::util::is_of_type<T, int, float, bool, std::string>)
		DummyReturnValue register_variable_listener(const std::string &scvar, void (*function)(NetworkState *, const ConVar &, T, T))
		{
			client::register_convar_callback(scvar, function);
			return {};
		}
	}
};
