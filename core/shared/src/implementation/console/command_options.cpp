// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :console.command_options;

void pragma::console::parse_command_options(const std::vector<std::string> args, std::unordered_map<std::string, CommandOption> &outOptions)
{
	auto itCurOption = outOptions.end();
	for(auto &arg : args) {
		if(arg.empty())
			continue;
		if(arg.front() == '-') {
			auto larg = arg.substr(1);
			string::to_lower(larg);
			auto it = outOptions.insert(std::make_pair(larg, CommandOption {})).first;
			itCurOption = it;
		}
		else {
			if(itCurOption == outOptions.end())
				continue;
			itCurOption->second.parameters.push_back(arg);
		}
	}
}

std::string pragma::console::get_command_option_parameter_value(std::unordered_map<std::string, CommandOption> &options, const std::string &key, const std::string &defaultValue)
{
	auto it = options.find(key);
	if(it == options.end() || it->second.parameters.empty())
		return defaultValue;
	return it->second.parameters.front();
}
