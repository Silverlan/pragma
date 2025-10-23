// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

module pragma.shared;

import :engine.launch_para_map;

std::stringstream LaunchParaMap::LAUNCHPARAMETERS_HELP;

LaunchParaMap::~LaunchParaMap() { m_parameters.clear(); }

const std::unordered_map<std::string, LaunchParameterFunc> &LaunchParaMap::GetParameters() const { return m_parameters; }
void LaunchParaMap::RegisterParameter(const std::string &name, const LaunchParameterFunc &f) { m_parameters.insert(std::make_pair(name, f)); }

void LaunchParaMap::RegisterParameterHelp(std::string name, const LaunchParameterFunc &function, std::string descCmd, std::string descHelp)
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

static LaunchParaMap *g_LaunchParameters = NULL;

LaunchParaMap *GetLaunchParaMap() { return g_LaunchParameters; }

void RegisterLaunchParameter(std::string name, const LaunchParameterFunc &function)
{
	if(g_LaunchParameters == NULL) {
		static LaunchParaMap map;
		g_LaunchParameters = &map;
	}
	g_LaunchParameters->RegisterParameter(name, function);
}

void RegisterLaunchParameterHelp(std::string name, const LaunchParameterFunc &function, std::string descCmd, std::string descHelp)
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
	g_LaunchParameters->LAUNCHPARAMETERS_HELP << pre.str() << sp << descHelp << "\n";
}
