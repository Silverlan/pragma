// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/definitions.h"
#include <vector>
#include <string>
#include <unordered_map>

export module pragma.shared:console.command_options;

export namespace pragma::console {
	struct DLLNETWORK CommandOption {
		std::vector<std::string> parameters;
	};
	DLLNETWORK void parse_command_options(const std::vector<std::string> args, std::unordered_map<std::string, CommandOption> &outOptions);
	DLLNETWORK std::string get_command_option_parameter_value(std::unordered_map<std::string, CommandOption> &options, const std::string &key, const std::string &defaultValue = "");
};
