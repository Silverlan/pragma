// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __LAUNCHPARAMETERS_H__
#define __LAUNCHPARAMETERS_H__
#include "pragma/definitions.h"
#include <sstream>
#include <string>
#include <functional>
#include <unordered_map>

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
};
#pragma warning(pop)
DLLNETWORK LaunchParaMap *GetLaunchParaMap();
DLLNETWORK void RegisterLaunchParameter(std::string name, const LaunchParameterFunc &function);
DLLNETWORK void RegisterLaunchParameterHelp(std::string name, const LaunchParameterFunc &function, std::string descCmd, std::string descHelp);

#define REGISTER_LAUNCH_PARAMETER__(lpName, function, identifier)                                                                                                                                                                                                                                \
	class LPM##function##identifier {                                                                                                                                                                                                                                                            \
	  public:                                                                                                                                                                                                                                                                                    \
		LPM##function##identifier() { RegisterLaunchParameter(#lpName, &function); }                                                                                                                                                                                                             \
	};                                                                                                                                                                                                                                                                                           \
	static LPM##function##identifier g_LPM##function##identifier;

#define REGISTER_LAUNCH_PARAMETER_(lpName, function, identifier) REGISTER_LAUNCH_PARAMETER__(lpName, function, identifier)

#define REGISTER_LAUNCH_PARAMETER(lpName, function) REGISTER_LAUNCH_PARAMETER_(lpName, function, __COUNTER__)

#define REGISTER_LAUNCH_PARAMETER_HELP__(lpName, function, identifier, descCmd, descHelp)                                                                                                                                                                                                        \
	class LPM##function##identifier {                                                                                                                                                                                                                                                            \
	  public:                                                                                                                                                                                                                                                                                    \
		LPM##function##identifier() { RegisterLaunchParameterHelp(#lpName, &function, descCmd, descHelp); }                                                                                                                                                                                      \
	};                                                                                                                                                                                                                                                                                           \
	static LPM##function##identifier g_LPM##function##identifier;

#define REGISTER_LAUNCH_PARAMETER_HELP_(lpName, function, identifier, descCmd, descHelp) REGISTER_LAUNCH_PARAMETER_HELP__(lpName, function, identifier, descCmd, descHelp)

#define REGISTER_LAUNCH_PARAMETER_HELP(lpName, function, descCmd, descHelp) REGISTER_LAUNCH_PARAMETER_HELP_(lpName, function, __COUNTER__, descCmd, descHelp)

#endif
