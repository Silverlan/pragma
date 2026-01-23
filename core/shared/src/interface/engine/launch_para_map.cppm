// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:engine.launch_para_map;

export import std.compat;

export namespace pragma {
#pragma warning(push)
#pragma warning(disable : 4251)
	using LaunchParameterFunc = void (*)(const std::vector<std::string> &);
	class DLLNETWORK LaunchParaMap {
	  public:
	  private:
		std::unordered_map<std::string, LaunchParameterFunc> m_parameters;
	  public:
		static std::stringstream LAUNCHPARAMETERS_HELP;
		~LaunchParaMap();
		const std::unordered_map<std::string, LaunchParameterFunc> &GetParameters() const;
		void RegisterParameter(const std::string &name, const LaunchParameterFunc &f);
		void RegisterParameterHelp(std::string name, const LaunchParameterFunc &function, std::string descCmd, std::string descHelp);
	};
#pragma warning(pop)

	DLLNETWORK LaunchParaMap *GetLaunchParaMap();
	DLLNETWORK void RegisterLaunchParameter(std::string name, const LaunchParameterFunc &function);
	DLLNETWORK void RegisterLaunchParameterHelp(std::string name, const LaunchParameterFunc &function, std::string descCmd, std::string descHelp);
};
