/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __CONSOLE_COMMAND_OPTIONS_HPP__
#define __CONSOLE_COMMAND_OPTIONS_HPP__

#include "pragma/definitions.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace pragma::console
{
	struct DLLNETWORK CommandOption
	{
		std::vector<std::string> parameters;
	};
	DLLNETWORK void parse_command_options(const std::vector<std::string> args,std::unordered_map<std::string,CommandOption> &outOptions);
	DLLNETWORK std::string get_command_option_parameter_value(std::unordered_map<std::string,CommandOption> &options,const std::string &key,const std::string &defaultValue="");
};

#endif
