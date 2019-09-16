#ifndef __CONSOLE_COMMAND_OPTIONS_HPP__
#define __CONSOLE_COMMAND_OPTIONS_HPP__

#include "pragma/definitions.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace pragma::console
{
	struct DLLENGINE CommandOption
	{
		std::vector<std::string> parameters;
	};
	DLLENGINE void parse_command_options(const std::vector<std::string> args,std::unordered_map<std::string,CommandOption> &outOptions);
	DLLENGINE std::string get_command_option_parameter_value(std::unordered_map<std::string,CommandOption> &options,const std::string &key,const std::string &defaultValue="");
};

#endif
