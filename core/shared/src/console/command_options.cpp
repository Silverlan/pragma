/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/console/command_options.hpp"
#include <sharedutils/util_string.h>

void pragma::console::parse_command_options(const std::vector<std::string> args, std::unordered_map<std::string, CommandOption> &outOptions)
{
	auto itCurOption = outOptions.end();
	for(auto &arg : args) {
		if(arg.empty())
			continue;
		if(arg.front() == '-') {
			auto larg = arg.substr(1);
			ustring::to_lower(larg);
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
