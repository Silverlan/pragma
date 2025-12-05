// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :engine.launch_para_map;

std::stringstream pragma::LaunchParaMap::LAUNCHPARAMETERS_HELP;

pragma::LaunchParaMap::~LaunchParaMap() { m_parameters.clear(); }

const std::unordered_map<std::string, pragma::LaunchParameterFunc> &pragma::LaunchParaMap::GetParameters() const { return m_parameters; }
void pragma::LaunchParaMap::RegisterParameter(const std::string &name, const LaunchParameterFunc &f) { m_parameters.insert(std::make_pair(name, f)); }

void pragma::LaunchParaMap::RegisterParameterHelp(std::string name, const LaunchParameterFunc &function, std::string descCmd, std::string descHelp)
{
	RegisterParameter(name, function);
	std::stringstream pre;
	pre << "\t" << name << " " << descCmd;
	std::string sp = "";
	int l = 22 - static_cast<int>(pre.str().length());
	if(l < 3) {
		pre << "\n\t";
		l = 21;
	}
	for(int j = 0; j < l; j++)
		sp += " ";
	LAUNCHPARAMETERS_HELP << pre.str() << sp << descHelp << "\n";
}

static pragma::LaunchParaMap g_LaunchParameters;

pragma::LaunchParaMap *pragma::GetLaunchParaMap() { return &g_LaunchParameters; }

void RegisterLaunchParameter(std::string name, const pragma::LaunchParameterFunc &function) { g_LaunchParameters.RegisterParameter(name, function); }

void RegisterLaunchParameterHelp(std::string name, const pragma::LaunchParameterFunc &function, std::string descCmd, std::string descHelp)
{
	RegisterLaunchParameter(name, function);
	std::stringstream pre;
	pre << "\t" << name << " " << descCmd;
	std::string sp = "";
	int l = 22 - static_cast<int>(pre.str().length());
	if(l < 3) {
		pre << "\n\t";
		l = 21;
	}
	for(int j = 0; j < l; j++)
		sp += " ";
	g_LaunchParameters.LAUNCHPARAMETERS_HELP << pre.str() << sp << descHelp << "\n";
}
